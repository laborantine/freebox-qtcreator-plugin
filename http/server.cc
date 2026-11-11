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

#include "server.hh"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkProxy>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>

namespace Freebox::Http {
namespace {

constexpr int MaxHeaderSize = 64 * 1024;
constexpr int BlockSize = 64 * 1024;

void sendFileData(QTcpSocket *client, QFile *file)
{
    while (client->bytesToWrite() < BlockSize) {
        const QByteArray block = file->read(BlockSize);
        if (block.isEmpty()) {
            client->disconnectFromHost();
            return;
        }
        if (client->write(block) < 0) {
            client->abort();
            return;
        }
    }
}

} // namespace

Server::Server()
{
    setProxy(QNetworkProxy::NoProxy);
    connect(this, &QTcpServer::newConnection, this, &Server::acceptClients);
}

bool Server::start(const QString &directory, const QHostAddress &player)
{
    m_directory = QDir(directory).canonicalPath();
    if (m_directory.isEmpty()) {
        m_error = tr("The project directory is unavailable.");
        return false;
    }
    m_player = player;
    return listen(QHostAddress::AnyIPv4);
}

QString Server::errorString() const
{
    return m_error.isEmpty() ? QTcpServer::errorString() : m_error;
}

void Server::acceptClients()
{
    while (auto client = nextPendingConnection()) {
        connect(client, &QTcpSocket::disconnected, client, &QObject::deleteLater);
        if (!client->peerAddress().isEqual(m_player, QHostAddress::ConvertV4MappedToIPv4)) {
            client->abort();
            continue;
        }
        auto deadline = new QTimer(client);
        deadline->setSingleShot(true);
        connect(deadline, &QTimer::timeout, client, &QTcpSocket::abort);
        deadline->start(10000);
        connect(client, &QTcpSocket::readyRead, client, [this, client, deadline] {
            if (client->bytesAvailable() > MaxHeaderSize) {
                deadline->stop();
                sendError(client, "431 Request Header Fields Too Large");
                return;
            }
            const QByteArray header = client->peek(MaxHeaderSize);
            const qsizetype end = header.indexOf("\r\n\r\n");
            if (end < 0)
                return;
            deadline->stop();
            QObject::disconnect(client, &QTcpSocket::readyRead, client, nullptr);
            client->read(end + 4);
            readRequest(client, header.first(header.indexOf("\r\n")));
        });
    }
}

void Server::readRequest(QTcpSocket *client, const QByteArray &request)
{
    const QList<QByteArray> parts = request.split(' ');
    if (parts.size() != 3 || !parts.at(2).startsWith("HTTP/1.")) {
        sendError(client, "400 Bad Request");
        return;
    }
    const bool headOnly = parts.first() == "HEAD";
    if (!headOnly && parts.first() != "GET") {
        sendError(client, "405 Method Not Allowed");
        return;
    }

    const QUrl url = QUrl::fromEncoded(parts.at(1), QUrl::StrictMode);
    const QString path = url.path(QUrl::FullyDecoded);
    const QString relative = path.mid(1);
    if (!url.isValid() || !url.isRelative() || !url.authority().isEmpty()
        || !path.startsWith(u'/') || relative.isEmpty() || relative.startsWith(u'/')
        || relative.contains(u'\\') || relative.contains(u':') || relative.contains(QChar(u'\0'))
        || relative.split(u'/').contains(QStringLiteral(".."))) {
        sendError(client, "400 Bad Request");
        return;
    }

    const QFileInfo fileInfo(QDir(m_directory).absoluteFilePath(relative));
    const QString canonical = fileInfo.canonicalFilePath();
    const QString prefix = m_directory.endsWith(u'/') ? m_directory : m_directory + u'/';
    if (!fileInfo.isFile() || !canonical.startsWith(prefix, Qt::CaseInsensitive)) {
        sendError(client, "404 Not Found");
        return;
    }

    auto file = new QFile(canonical, client);
    if (!file->open(QIODevice::ReadOnly)) {
        file->deleteLater();
        sendError(client, "404 Not Found");
        return;
    }

    client->write("HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: "
                  + QByteArray::number(file->size())
                  + "\r\nCache-Control: no-cache\r\nPragma: no-cache\r\nAccept-Ranges: none\r\n\r\n");
    if (headOnly) {
        client->disconnectFromHost();
        return;
    }
    connect(client, &QTcpSocket::bytesWritten, file, [client, file] { sendFileData(client, file); });
    sendFileData(client, file);
}

void Server::sendError(QTcpSocket *client, const QByteArray &status)
{
    client->write("HTTP/1.1 " + status + "\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
    client->disconnectFromHost();
}

} // namespace Freebox::Http
