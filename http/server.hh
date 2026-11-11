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
#include <QTcpServer>

class QTcpSocket;

namespace Freebox::Http {

class Server final : public QTcpServer
{
public:
    Server();
    bool start(const QString &directory, const QHostAddress &player);
    QString errorString() const;

private:
    void acceptClients();
    void readRequest(QTcpSocket *client, const QByteArray &request);
    void sendError(QTcpSocket *client, const QByteArray &status);

    QString m_directory;
    QString m_error;
    QHostAddress m_player;
};

} // namespace Freebox::Http
