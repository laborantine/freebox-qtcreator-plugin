/*
  Freebox QtCreator plugin for QML application development.

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not see
  http://www.gnu.org/licenses/lgpl-2.1.html.

  Copyright (c) 2014, Freebox SAS, See AUTHORS for details.
*/
// Modified on 2026-11-11 for Qt Creator 20.0.2 and Qt 6.

#include "project.hh"
#include "constants.hh"
#include "fileformat/qmlprojectfileformat.hh"
#include "fileformat/qmlprojectitem.hh"

#include <coreplugin/icontext.h>
#include <coreplugin/messagemanager.h>
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/buildinfo.h>
#include <projectexplorer/devicesupport/devicekitaspects.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>
#include <qtsupport/baseqtversion.h>
#include <qtsupport/qtkitaspect.h>
#include <utils/fsengine/fileiconprovider.h>

#include <QTimer>

using namespace ProjectExplorer;
using namespace Utils;

namespace Freebox {

Project::Project(const FilePath &filename)
    : ProjectExplorer::Project(QLatin1String(Constants::FBXPROJECT_MIMETYPE), filename)
{
    setType(Constants::PROJECT_ID);
    setProjectLanguages(Core::Context(ProjectExplorer::Constants::QMLJS_LANGUAGE_ID));
    setDisplayName(filename.completeBaseName());
    setSupportsBuilding(false);
    setIsEditModePreferred(true);
    setBuildSystemCreator<Freebox::BuildSystem>();

    const FilePath manifestPath = projectDirectory().pathAppended("manifest.json");
    setExtraProjectFiles({manifestPath}, {}, [this](Core::IDocument *) { refresh(); });
    connect(this, &ProjectExplorer::Project::projectFileIsDirty, this, &Project::refresh);
    connect(KitManager::instance(), &KitManager::kitsChanged, this, [this] {
        if (m_updateKitsPending)
            return;
        m_updateKitsPending = true;
        QTimer::singleShot(0, this, [this] {
            m_updateKitsPending = false;
            updateKits();
        });
    });

    refresh();
}

Tasks Project::kitIssues(const Kit *kit)
{
    const Id type = RunDeviceTypeKitAspect::deviceTypeId(kit);
    if (type == Constants::FREEBOX_DEVICE_TYPE)
        return {};
    if (type == ProjectExplorer::Constants::DESKTOP_DEVICE_TYPE) {
        const auto version = QtSupport::QtKitAspect::qtVersion(kit);
        if (version && !version->qmlRuntimeFilePath().isEmpty())
            return {};
        return {createTask(Task::Error, tr("This desktop kit has no QML runtime."))};
    }
    return {createTask(Task::Error, tr("Select a Freebox or a desktop QML kit."))};
}

QDir Project::projectDir() const
{
    return QDir(projectDirectory().toFSPathString());
}

FilePaths Project::sourceFiles() const
{
    return m_projectItem ? FilePaths::fromStrings(m_projectItem->files()) : FilePaths{};
}

FilePaths Project::importPaths() const
{
    if (!m_projectItem)
        return {};
    FilePaths paths;
    for (const QString &path : m_projectItem->importPaths())
        paths.append(projectDirectory().resolvePath(path));
    return paths;
}

bool Project::isValid() const
{
    return m_projectItem && m_manifest.isValid();
}

bool Project::matchesFile(const FilePath &path) const
{
    return m_projectItem && m_projectItem->matchesFile(path.toFSPathString());
}

void Project::refresh()
{
    QString error;
    delete m_projectItem.data();
    m_projectItem = Fileformat::QmlProjectFileFormat::parseProjectFile(projectFilePath(), &error);
    m_manifest = Fileformat::Manifest(projectDir().absoluteFilePath(QStringLiteral("manifest.json")));
    if (m_projectItem) {
        m_projectItem->setParent(this);
        m_projectItem->setSourceDirectory(projectDir().path());
        connect(m_projectItem, &Fileformat::QmlProjectItem::qmlFilesChanged, this, [this] {
            rebuildTree();
            emit sourceFilesChanged();
        });
    } else {
        Core::MessageManager::writeSilently(tr("Cannot read Freebox project %1: %2")
                                                .arg(projectFilePath().toUserOutput(), error));
    }

    rebuildTree();
    emit sourceFilesChanged();
}

void Project::rebuildTree()
{
    auto root = std::make_unique<ProjectNode>(projectDirectory());
    root->setDisplayName(displayName());
    root->setIcon(QIcon(FileIconProvider::overlayIcon(
        QStyle::SP_DirIcon, QIcon(QStringLiteral(":/freebox/images/qmlfolder.png")), QSize(16, 16))));

    for (const FilePath &file : sourceFiles()) {
        if (file != projectFilePath())
            root->addNestedNode(std::make_unique<FileNode>(file, FileType::Source));
    }
    root->addNestedNode(std::make_unique<FileNode>(projectFilePath(), FileType::Project));
    setRootProjectNode(std::move(root));
}

Project::RestoreResult Project::fromMap(const Store &map, QString *errorMessage)
{
    const auto result = ProjectExplorer::Project::fromMap(map, errorMessage);
    if (result == RestoreResult::Ok)
        updateKits();
    return result;
}

void Project::updateKits()
{
    Target *firstFreebox = nullptr;
    Target *firstDesktop = nullptr;
    const bool selectTarget = !activeTarget();
    for (Kit *kit : KitManager::kits()) {
        if (!kit->isValid() || !kitIssues(kit).isEmpty())
            continue;
        Target *candidate = target(kit);
        if (!candidate)
            candidate = addTargetForKit(kit);
        if (!candidate)
            continue;
        if (candidate->buildConfigurations().isEmpty())
            candidate->updateDefaultBuildConfigurations();

        if (RunDeviceTypeKitAspect::deviceTypeId(kit) == Constants::FREEBOX_DEVICE_TYPE) {
            if (!firstFreebox)
                firstFreebox = candidate;
        } else if (!firstDesktop || kit == KitManager::defaultKit()) {
            firstDesktop = candidate;
        }
    }

    if (selectTarget) {
        if (Target *candidate = firstFreebox ? firstFreebox : firstDesktop)
            setActiveTarget(candidate, SetActive::NoCascade);
    }
}

BuildSystem::BuildSystem(BuildConfiguration *configuration)
    : ProjectExplorer::BuildSystem(configuration)
{
    const auto freeboxProject = static_cast<Project *>(project());
    connect(freeboxProject, &Project::sourceFilesChanged, this, &BuildSystem::requestParse);
    requestParse();
}

void BuildSystem::triggerParsing()
{
    const auto guard = guardParsingRun();
    const auto freeboxProject = static_cast<Project *>(project());
    QList<BuildTargetInfo> targets;
    if (freeboxProject->isValid()) {
        const auto &manifest = freeboxProject->manifest();
        const QString defaultEntry = manifest.defaultEntryPoint();
        for (const QString &entry : manifest.entryPoints()) {
            BuildTargetInfo target;
            target.buildKey = entry;
            target.displayName = entry;
            target.projectFilePath = projectFilePath();
            target.targetFilePath = projectDirectory().resolvePath(manifest.entryPointFile(entry));
            target.workingDirectory = projectDirectory();
            target.isQtcRunnable = true;
            if (entry == defaultEntry)
                targets.prepend(target);
            else
                targets.append(target);
        }
        guard.markAsSuccess();
    }
    setApplicationTargets(targets);
    updateQmlCodeModel();
    emitBuildSystemUpdated();
}

void BuildSystem::updateQmlCodeModelInfo(QmlCodeModelInfo &info)
{
    const auto freeboxProject = static_cast<Project *>(project());
    info.sourceFiles = freeboxProject->sourceFiles();
    info.qmlImportPaths = freeboxProject->importPaths();
}

bool BuildSystem::supportsAction(Node *, ProjectAction action, const Node *node) const
{
    return action == AddNewFile || action == EraseFile
           || (action == Rename && node && node->asFileNode()
               && node->asFileNode()->fileType() != FileType::Project);
}

bool BuildSystem::addFiles(Node *, const FilePaths &paths, FilePaths *notAdded)
{
    const auto freeboxProject = static_cast<Project *>(project());
    bool allAdded = true;
    for (const FilePath &path : paths) {
        if (!freeboxProject->matchesFile(path)) {
            allAdded = false;
            if (notAdded)
                notAdded->append(path);
        }
    }
    requestParse();
    return allAdded;
}

bool BuildSystem::deleteFiles(Node *, const FilePaths &)
{
    return true;
}

bool BuildSystem::canRenameFile(Node *, const FilePath &, const FilePath &newFile)
{
    return static_cast<Project *>(project())->matchesFile(newFile);
}

bool BuildSystem::renameFiles(Node *, const FilePairs &, FilePaths *)
{
    return true;
}

class BuildConfigurationFactory final : public ProjectExplorer::BuildConfigurationFactory
{
public:
    BuildConfigurationFactory()
    {
        registerBuildConfiguration<BuildConfiguration>("Freebox.BuildConfiguration");
        setSupportedProjectType(Constants::PROJECT_ID);
        setSupportedProjectMimeTypeName(QLatin1String(Constants::FBXPROJECT_MIMETYPE));
        setBuildGenerator([](const Kit *, const FilePath &projectFile, bool) -> QList<BuildInfo> {
            BuildInfo info;
            info.buildSystemName = BuildSystem::name();
            info.buildDirectory = projectFile.parentDir();
            info.displayName = info.typeName = Project::tr("Default");
            info.showBuildConfigs = info.showBuildDirConfigWidget = false;
            return {info};
        });
    }
};

void setupBuildConfiguration()
{
    static BuildConfigurationFactory factory;
}

} // namespace Freebox
