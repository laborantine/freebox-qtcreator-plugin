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

#include "configuration.hh"
#include "device.hh"

#include <extensionsystem/iplugin.h>

#include <memory>

class QAction;
namespace ProjectExplorer { class Project; }

namespace Freebox {

class FreeboxPlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Freebox.json")

private:
    void initialize() override;
    void extensionsInitialized() override;
    void updatePackageActions(ProjectExplorer::Project *project);
    void makePackage(bool saveAs);

    std::unique_ptr<DeviceFactory> m_deviceFactory;
    Discovery m_discovery;
    QAction *m_package = nullptr;
    QAction *m_packageAs = nullptr;
};

} // namespace Freebox
