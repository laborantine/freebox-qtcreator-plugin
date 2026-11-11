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

#include "configuration.hh"
#include "constants.hh"
#include "device.hh"
#include "ssdp/ssdpclient.hh"
#include "ssdp/ssdpmessage.hh"

#include <projectexplorer/devicesupport/devicekitaspects.h>
#include <projectexplorer/devicesupport/devicemanager.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <qtsupport/qtkitaspect.h>

#include <QLoggingCategory>

using namespace ProjectExplorer;
using namespace Utils;

namespace Freebox {

Q_LOGGING_CATEGORY(freeboxDiscovery, "qtc.freebox.discovery", QtWarningMsg)

Discovery::Discovery(QObject *parent)
    : QObject(parent)
    , m_client(new Ssdp::Client(this))
{
    connect(m_client, &Ssdp::Client::messageReceived, this, &Discovery::processMessage);
    connect(&m_searchTimer, &QTimer::timeout, m_client, &Ssdp::Client::search);
}

void Discovery::start()
{
    connect(DeviceManager::instance(), &DeviceManager::devicesLoaded, this, &Discovery::start,
            Qt::UniqueConnection);
    connect(KitManager::instance(), &KitManager::kitsLoaded, this, &Discovery::ensureKits,
            Qt::UniqueConnection);
    if (m_started || !DeviceManager::isLoaded())
        return;

    m_started = true;
    m_client->start(QLatin1String(Constants::FREEBOX_DEVICE_NT));
    m_client->search();
    m_searchTimer.start(15000);
}

void Discovery::processMessage(const QHostAddress &address, const Ssdp::Message &message)
{
    const QString usn = message.fieldValue(QStringLiteral("USN")).trimmed();
    if (usn.isEmpty() || address.isNull())
        return;
    const QLatin1String service(Constants::FREEBOX_DEVICE_NT);
    if (message.type() == Ssdp::Message::REPLY && message.fieldValue(QStringLiteral("ST")) == service) {
        deviceFound(address, usn);
    } else if (message.type() == Ssdp::Message::NOTIFY
               && message.fieldValue(QStringLiteral("NT")) == service) {
        const QString notification = message.fieldValue(QStringLiteral("NTS"));
        if (notification == QStringLiteral("ssdp:alive"))
            deviceFound(address, usn);
        else if (notification == QStringLiteral("ssdp:byebye") && m_players.value(usn) == address)
            deviceLeft(usn);
    }
}

void Discovery::deviceFound(const QHostAddress &address, const QString &usn)
{
    const Id id = Id::fromString(usn);
    auto device = std::dynamic_pointer_cast<FreeboxDevice>(DeviceManager::find(id));
    if (!device || device->address() != address) {
        device = std::make_shared<FreeboxDevice>();
        device->setupId(IDevice::AutoDetected, id);
        device->setAddress(address);
        device->setDisplayName(QStringLiteral("Freebox (%1)").arg(address.toString()));
        DeviceManager::addDevice(device);
    }

    m_players.insert(usn, address);
    if (device->deviceState() != IDevice::DeviceReadyToUse) {
        DeviceManager::setDeviceState(id, IDevice::DeviceReadyToUse);
        qCInfo(freeboxDiscovery) << "Freebox Player discovered" << usn << address.toString();
    }
    ensureKit(usn);
}

void Discovery::deviceLeft(const QString &usn)
{
    m_players.remove(usn);
    DeviceManager::setDeviceState(Id::fromString(usn), IDevice::DeviceDisconnected);
}

void Discovery::ensureKit(const QString &usn)
{
    if (!KitManager::isLoaded())
        return;
    const auto device = DeviceManager::find(Id::fromString(usn));
    if (!device)
        return;

    const QString kitName = QStringLiteral("Freebox.Kit.") + usn;
    const Id kitId = Id::fromString(kitName);
    if (Kit *kit = KitManager::kit(kitId)) {
        if (kit->unexpandedDisplayName() != device->displayName())
            kit->setUnexpandedDisplayName(device->displayName());
        return;
    }

    KitManager::registerKit([device](Kit *kit) {
        kit->setUnexpandedDisplayName(device->displayName());
        kit->setDetectionSource(DetectionSource(DetectionSource::FromSystem, device->id().toString()));
        BuildDeviceTypeKitAspect::setDeviceTypeId(kit, ProjectExplorer::Constants::DESKTOP_DEVICE_TYPE);
        BuildDeviceKitAspect::setDevice(kit, DeviceManager::defaultDesktopDevice());
        RunDeviceTypeKitAspect::setDeviceTypeId(kit, Constants::FREEBOX_DEVICE_TYPE);
        RunDeviceKitAspect::setDevice(kit, device);
        QtSupport::QtKitAspect::setQtVersionId(kit, -1);
    }, kitId);
}

void Discovery::ensureKits()
{
    for (auto it = m_players.cbegin(); it != m_players.cend(); ++it)
        ensureKit(it.key());
}

} // namespace Freebox
