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

#include <QHostAddress>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>

namespace Freebox::Remote {

class QmlRemote final : public QObject
{
    Q_OBJECT

public:
    explicit QmlRemote(QObject *parent = nullptr);
    void start(const QHostAddress &address, const QString &entryPoint, quint16 port, bool wait);

signals:
    void started(int qmlPort, int stdoutPort, int stderrPort);
    void failed(const QString &error);

private:
    QTcpSocket m_routeProbe;
    QTimer m_routeDeadline;
    bool m_routePending = false;
};

} // namespace Freebox::Remote
