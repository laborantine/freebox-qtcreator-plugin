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

#include "freeboxplugin.hh"
#include "constants.hh"
#include "freestorepackager.hh"
#include "project.hh"
#include "runconfigurationfactory.hh"
#include "runcontrol.hh"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <projectexplorer/jsonwizard/jsonwizardfactory.h>
#include <projectexplorer/projectmanager.h>
#include <utils/fsengine/fileiconprovider.h>

#include <QAction>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>

namespace Freebox {

void FreeboxPlugin::initialize()
{
    using namespace ProjectExplorer;
    using namespace Utils;
    m_deviceFactory = std::make_unique<DeviceFactory>();
    JsonWizardFactory::addWizardPath(FilePath::fromString(":/freebox/templates/wizard"));
    ProjectManager::registerProjectType<Freebox::Project>(
        QLatin1String(Constants::FBXPROJECT_MIMETYPE), Freebox::Project::kitIssues);
    FileIconProvider::registerIconOverlayForSuffix(
        QStringLiteral(":/freebox/images/qmlproject.png"), QStringLiteral("fbxproject"));
    setupBuildConfiguration();
    setupRunConfigurations();
    setupRunWorkers();

    m_package = new QAction(tr("Make FreeStore &Package"), this);
    m_packageAs = new QAction(tr("Make FreeStore Package &As…"), this);
    const Core::Context context(Core::Constants::C_GLOBAL);
    const auto packageCommand = Core::ActionManager::registerAction(
        m_package, Constants::MAKE_PACKAGE_ACTION_ID, context);
    const auto packageAsCommand = Core::ActionManager::registerAction(
        m_packageAs, Constants::MAKE_PACKAGE_AS_ACTION_ID, context);
    packageCommand->setDefaultKeySequence(QKeySequence(QStringLiteral("Ctrl+Alt+P")));
    packageAsCommand->setDefaultKeySequence(QKeySequence(QStringLiteral("Ctrl+Alt+Shift+P")));
    connect(m_package, &QAction::triggered, this, [this] { makePackage(false); });
    connect(m_packageAs, &QAction::triggered, this, [this] { makePackage(true); });

    const auto menu = Core::ActionManager::createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("&Freebox SDK"));
    menu->addAction(packageCommand);
    menu->addAction(packageAsCommand);
    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);
    connect(ProjectManager::instance(), &ProjectManager::startupProjectChanged,
            this, &FreeboxPlugin::updatePackageActions);
    updatePackageActions(ProjectManager::startupProject());
}

void FreeboxPlugin::extensionsInitialized()
{
    m_discovery.start();
}

void FreeboxPlugin::updatePackageActions(ProjectExplorer::Project *project)
{
    const bool enabled = qobject_cast<Project *>(project) != nullptr;
    m_package->setEnabled(enabled);
    m_packageAs->setEnabled(enabled);
}

void FreeboxPlugin::makePackage(bool saveAs)
{
    const auto project = qobject_cast<Project *>(ProjectExplorer::ProjectManager::startupProject());
    if (!project)
        return;
    QString output = project->packageFileName;
    if (output.isEmpty())
        output = project->projectDir().canonicalPath() + QStringLiteral(".fbxqml");

    if (saveAs) {
        output = QFileDialog::getSaveFileName(Core::ICore::dialogParent(), tr("Make FreeStore Package"),
                                              output, tr("Freebox QML package (*.fbxqml)"));
        if (output.isEmpty())
            return;
    }
    project->packageFileName = output;
    Internal::FreeStorePackager packager(project, output);
    if (packager()) {
        QMessageBox::information(Core::ICore::dialogParent(), tr("FreeStore package created"),
                                 tr("Package for %1 saved to %2.").arg(project->displayName(), output));
    }
}

} // namespace Freebox
