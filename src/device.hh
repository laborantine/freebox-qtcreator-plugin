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

#include <projectexplorer/devicesupport/idevice.h>
#include <projectexplorer/devicesupport/idevicefactory.h>

#include <QHostAddress>

namespace Freebox {

class FreeboxDevice final : public ProjectExplorer::IDevice
{
public:
    FreeboxDevice();
    void setAddress(const QHostAddress &address);
    const QHostAddress &address() const { return m_address; }

    ProjectExplorer::IDeviceWidget *createWidget() override;
    QUrl toolControlChannel(const ControlChannelHint &) const override;
    DeviceInfo deviceInformation() const override;

private:
    void fromMap(const Utils::Store &map) override;
    void toMap(Utils::Store &map) const override;

    QHostAddress m_address;
};

class DeviceFactory final : public ProjectExplorer::IDeviceFactory
{
public:
    DeviceFactory();
};

} // namespace Freebox
