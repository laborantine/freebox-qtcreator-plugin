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

#include "device.hh"
#include "constants.hh"

#include <QCoreApplication>

using namespace ProjectExplorer;
using namespace Utils;

namespace Freebox {

FreeboxDevice::FreeboxDevice()
{
    setType(Constants::FREEBOX_DEVICE_TYPE);
    setMachineType(Hardware);
    setOsType(OsTypeLinux);
    setDisplayType(QCoreApplication::translate("FreeboxDevice", "Freebox Player"));
}

void FreeboxDevice::setAddress(const QHostAddress &address)
{
    m_address = address;
    setDefaultDisplayName(QStringLiteral("Freebox (%1)").arg(address.toString()));
}

IDeviceWidget *FreeboxDevice::createWidget()
{
    return nullptr;
}

QUrl FreeboxDevice::toolControlChannel(const ControlChannelHint &) const
{
    QUrl channel;
    channel.setScheme(QStringLiteral("tcp"));
    channel.setHost(m_address.toString());
    return channel;
}

IDevice::DeviceInfo FreeboxDevice::deviceInformation() const
{
    return {{QStringLiteral("IP"), m_address.toString()}};
}

void FreeboxDevice::fromMap(const Store &map)
{
    IDevice::fromMap(map);
    setAddress(QHostAddress(map.value("Freebox.Address").toString()));
    setDeviceState(DeviceDisconnected);
}

void FreeboxDevice::toMap(Store &map) const
{
    IDevice::toMap(map);
    map.insert("Freebox.Address", m_address.toString());
}

DeviceFactory::DeviceFactory()
    : IDeviceFactory(Constants::FREEBOX_DEVICE_TYPE)
{
    setDisplayName(QCoreApplication::translate("FreeboxDevice", "Freebox Player"));
    setCombinedIcon(FilePath::fromString(":/freebox/images/tvdevicesmall.png"),
                    FilePath::fromString(":/freebox/images/tvdevice.png"));
    setConstructionFunction([] { return std::make_shared<FreeboxDevice>(); });
}

} // namespace Freebox
