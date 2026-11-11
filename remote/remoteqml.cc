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

#include "remoteqml.hh"
#include "../util/jsonrpc.hh"

#include <QJsonObject>
#include <QNetworkProxy>
#include <QUrl>

namespace Freebox::Remote {

QmlRemote::QmlRemote(QObject *parent)
    : QObject(parent)
{
    m_routeProbe.setProxy(QNetworkProxy::NoProxy);
    m_routeDeadline.setSingleShot(true);
    connect(&m_routeDeadline, &QTimer::timeout, this, [this] {
        m_routePending = false;
        m_routeProbe.abort();
        emit failed(tr("Timed out while connecting to the Freebox Player."));
    });
    connect(&m_routeProbe, &QTcpSocket::errorOccurred, this, [this] {
        if (!m_routePending)
            return;
        m_routePending = false;
        m_routeDeadline.stop();
        emit failed(tr("Cannot connect to the Freebox Player: %1").arg(m_routeProbe.errorString()));
    });
}

void QmlRemote::start(const QHostAddress &address, const QString &entryPoint, quint16 port, bool wait)
{
    m_routePending = true;
    connect(&m_routeProbe, &QTcpSocket::connected, this, [this, address, entryPoint, port, wait] {
        const QHostAddress localAddress = m_routeProbe.localAddress();
        m_routePending = false;
        m_routeDeadline.stop();
        m_routeProbe.disconnectFromHost();

        QUrl manifest;
        manifest.setScheme(QStringLiteral("http"));
        manifest.setHost(localAddress.toString());
        manifest.setPort(port);
        manifest.setPath(QStringLiteral("/manifest.json"));

        QUrl endpoint;
        endpoint.setScheme(QStringLiteral("http"));
        endpoint.setHost(address.toString());
        endpoint.setPath(QStringLiteral("/pub/devel"));

        const QJsonObject parameters{
            {QStringLiteral("manifest_url"), manifest.toString(QUrl::FullyEncoded)},
            {QStringLiteral("entry_point"), entryPoint},
            {QStringLiteral("wait"), wait}
        };

        auto request = new Util::JsonRpcRequest(endpoint, QStringLiteral("debug_qml_app"), parameters, this);
        connect(request, &Util::JsonRpcRequest::error, this, [this, request](const QJsonObject &error) {
            request->deleteLater();
            emit failed(error.value(QStringLiteral("message")).toString(tr("Freebox rejected the request.")));
        });
        connect(request, &Util::JsonRpcRequest::done, this, [this, request](const QJsonValue &reply) {
            request->deleteLater();
            const QJsonObject result = reply.toObject();
            emit started(result.value(QStringLiteral("qml_port")).toInt(),
                         result.value(QStringLiteral("stdout_port")).toInt(),
                         result.value(QStringLiteral("stderr_port")).toInt());
        });
    }, Qt::SingleShotConnection);
    m_routeDeadline.start(4000);
    m_routeProbe.connectToHost(address, 80);
}

} // namespace Freebox::Remote
