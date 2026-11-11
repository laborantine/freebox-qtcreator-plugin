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

#include <QHash>
#include <QHostAddress>
#include <QObject>
#include <QTimer>

namespace Freebox {
namespace Ssdp { class Client; class Message; }

class Discovery final : public QObject
{
public:
    explicit Discovery(QObject *parent = nullptr);
    void start();

private:
    void processMessage(const QHostAddress &address, const Ssdp::Message &message);
    void deviceFound(const QHostAddress &address, const QString &usn);
    void deviceLeft(const QString &usn);
    void ensureKit(const QString &usn);
    void ensureKits();

    Ssdp::Client *m_client;
    QTimer m_searchTimer;
    QHash<QString, QHostAddress> m_players;
    bool m_started = false;
};

} // namespace Freebox
