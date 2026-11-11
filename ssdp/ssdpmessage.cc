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

#include "ssdpmessage.hh"

#include <QStringList>

namespace Freebox::Ssdp {

Message::Message(const QByteArray &datagram)
{
    QStringList lines = QString::fromUtf8(datagram).split(u'\n', Qt::SkipEmptyParts);
    if (lines.isEmpty())
        return;

    const QString status = lines.takeFirst().simplified();
    if (status == QStringLiteral("NOTIFY * HTTP/1.1"))
        m_type = NOTIFY;
    else if (status.startsWith(QStringLiteral("HTTP/1.1 200 ")))
        m_type = REPLY;
    else if (status == QStringLiteral("M-SEARCH * HTTP/1.1"))
        m_type = MSEARCH;
    else
        return;

    for (const QString &line : lines) {
        const qsizetype colon = line.indexOf(u':');
        if (colon > 0)
            m_headers.insert(line.first(colon).trimmed().toUpper(), line.mid(colon + 1).trimmed());
    }
}

QString Message::fieldValue(const QString &name) const
{
    return m_headers.value(name.toUpper());
}

} // namespace Freebox::Ssdp
