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

#pragma once

#include "fileformat/manifest.hh"

#include <projectexplorer/buildsystem.h>
#include <projectexplorer/project.h>

#include <QDir>
#include <QPointer>

namespace Freebox::Fileformat { class QmlProjectItem; }

namespace Freebox {

class Project final : public ProjectExplorer::Project
{
    Q_OBJECT

public:
    explicit Project(const Utils::FilePath &filename);

    static ProjectExplorer::Tasks kitIssues(const ProjectExplorer::Kit *kit);
    QDir projectDir() const;
    const Fileformat::Manifest &manifest() const { return m_manifest; }
    Utils::FilePaths sourceFiles() const;
    Utils::FilePaths importPaths() const;
    bool isValid() const;
    bool matchesFile(const Utils::FilePath &path) const;
    void refresh();

    QString packageFileName;

signals:
    void sourceFilesChanged();

private:
    RestoreResult fromMap(const Utils::Store &map, QString *errorMessage) override;
    void updateKits();
    void rebuildTree();

    Fileformat::Manifest m_manifest;
    QPointer<Fileformat::QmlProjectItem> m_projectItem;
    bool m_updateKitsPending = false;
};

class BuildSystem final : public ProjectExplorer::BuildSystem
{
public:
    explicit BuildSystem(ProjectExplorer::BuildConfiguration *configuration);
    static QString name() { return QStringLiteral("Freebox"); }

private:
    void triggerParsing() override;
    void updateQmlCodeModelInfo(ProjectExplorer::QmlCodeModelInfo &info) override;
    bool supportsAction(ProjectExplorer::Node *, ProjectExplorer::ProjectAction,
                        const ProjectExplorer::Node *node) const override;
    bool addFiles(ProjectExplorer::Node *, const Utils::FilePaths &paths,
                  Utils::FilePaths *notAdded) override;
    bool deleteFiles(ProjectExplorer::Node *, const Utils::FilePaths &) override;
    bool canRenameFile(ProjectExplorer::Node *, const Utils::FilePath &,
                       const Utils::FilePath &newFile) override;
    bool renameFiles(ProjectExplorer::Node *, const Utils::FilePairs &, Utils::FilePaths *) override;
};

void setupBuildConfiguration();

} // namespace Freebox
