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

#include "runconfigurationfactory.hh"
#include "constants.hh"
#include "project.hh"

#include <projectexplorer/devicesupport/devicekitaspects.h>
#include <projectexplorer/devicesupport/idevice.h>
#include <projectexplorer/environmentaspect.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/runconfiguration.h>
#include <projectexplorer/runconfigurationaspects.h>
#include <qtsupport/baseqtversion.h>
#include <qtsupport/qtkitaspect.h>
#include <utils/processinterface.h>
#include <utils/qtcprocess.h>

using namespace ProjectExplorer;
using namespace Utils;

namespace Freebox {
namespace {

class RemoteRunConfiguration final : public RunConfiguration
{
public:
    RemoteRunConfiguration(BuildConfiguration *bc, Id id) : RunConfiguration(bc, id) {}

private:
    QString disabledReason(Id mode) const override
    {
        const QString baseReason = RunConfiguration::disabledReason(mode);
        if (!baseReason.isEmpty())
            return baseReason;
        const auto device = RunDeviceKitAspect::device(kit());
        if (!device || device->deviceState() != IDevice::DeviceReadyToUse)
            return Project::tr("Enable developer mode on the Freebox Player and connect it to this network.");
        if (!buildTargetInfo().targetFilePath.isFile())
            return Project::tr("The QML entry point specified in manifest.json is missing.");
        return {};
    }

    bool isEnabled(Id mode) const override
    {
        return RunConfiguration::isEnabled(mode) && disabledReason(mode).isEmpty();
    }
};

class LocalRunConfiguration final : public RunConfiguration
{
public:
    LocalRunConfiguration(BuildConfiguration *bc, Id id)
        : RunConfiguration(bc, id)
    {
        m_arguments.setSettingsKey(Constants::FBX_VIEWER_ARGUMENTS_KEY);
        m_environment.addPreferredBaseEnvironment(Project::tr("System Environment"), [this] {
            return kit()->runEnvironment();
        });
        setCommandLineGetter([this] {
            CommandLine command(runtime());
            command.addArgs(m_arguments(), CommandLine::Raw);
            const auto freeboxProject = static_cast<Project *>(project());
            for (const FilePath &path : freeboxProject->importPaths()) {
                command.addArg("-I");
                command.addArg(path.toFSPathString());
            }
            command.addArg(buildTargetInfo().targetFilePath.toFSPathString());
            return command;
        });
        setRunnableModifier([this](ProcessRunData &data) {
            data.workingDirectory = project()->projectDirectory();
        });
    }

private:
    FilePath runtime() const
    {
        const auto version = QtSupport::QtKitAspect::qtVersion(kit());
        return version ? version->qmlRuntimeFilePath() : FilePath{};
    }

    QString disabledReason(Id mode) const override
    {
        const QString baseReason = RunConfiguration::disabledReason(mode);
        if (!baseReason.isEmpty())
            return baseReason;
        if (!runtime().isExecutableFile())
            return Project::tr("The selected Qt kit has no installed QML runtime.");
        if (!buildTargetInfo().targetFilePath.isFile())
            return Project::tr("The QML entry point specified in manifest.json is missing.");
        return {};
    }

    bool isEnabled(Id mode) const override
    {
        return RunConfiguration::isEnabled(mode) && disabledReason(mode).isEmpty();
    }

    ArgumentsAspect m_arguments{this};
    EnvironmentAspect m_environment{this};
};

class RemoteRunConfigurationFactory final : public RunConfigurationFactory
{
public:
    RemoteRunConfigurationFactory()
    {
        registerRunConfiguration<RemoteRunConfiguration>(Constants::REMOTE_RUN_ID);
        addSupportedProjectType(Constants::PROJECT_ID);
        addSupportedTargetDeviceType(Constants::FREEBOX_DEVICE_TYPE);
    }
};

class LocalRunConfigurationFactory final : public RunConfigurationFactory
{
public:
    LocalRunConfigurationFactory()
    {
        registerRunConfiguration<LocalRunConfiguration>(Constants::LOCAL_RUN_ID);
        addSupportedProjectType(Constants::PROJECT_ID);
        addSupportedTargetDeviceType(ProjectExplorer::Constants::DESKTOP_DEVICE_TYPE);
        setExecutionTypeId(ProjectExplorer::Constants::STDPROCESS_EXECUTION_TYPE_ID);
    }
};

} // namespace

void setupRunConfigurations()
{
    static RemoteRunConfigurationFactory remote;
    static LocalRunConfigurationFactory local;
}

} // namespace Freebox
