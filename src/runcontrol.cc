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

#include "runcontrol.hh"
#include "constants.hh"
#include "device.hh"
#include "http/server.hh"
#include "remote/remoteqml.hh"

#include <debugger/debuggerruncontrol.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/runcontrol.h>
#include <QtTaskTree/QBarrier>

#include <QNetworkProxy>
#include <QStringConverter>
#include <QTcpSocket>
#include <QTimer>

using namespace ProjectExplorer;
using namespace QtTaskTree;
using namespace Utils;

namespace Freebox {
namespace {

class RemoteSession final : public QObject
{
    Q_OBJECT

public:
    void configure(RunControl *control, const QHostAddress &address, bool waitForDebugger);
    void start();

signals:
    void readyForTools();
    void finished(bool success);

private:
    void startLogging(int qmlPort, int stdoutPort, int stderrPort);
    void readLog(QTcpSocket &socket, QStringDecoder &decoder, OutputFormat format);
    void fail(const QString &error);

    RunControl *m_control = nullptr;
    QHostAddress m_address;
    Http::Server m_server;
    Remote::QmlRemote m_remote;
    QTcpSocket m_stdout;
    QTcpSocket m_stderr;
    QStringDecoder m_stdoutDecoder{QStringDecoder::Utf8};
    QStringDecoder m_stderrDecoder{QStringDecoder::Utf8};
    QTimer m_logDeadline;
    QTimer m_drainDeadline;
    bool m_waitForDebugger = false;
    bool m_stdoutConnected = false;
    bool m_stderrConnected = false;
    bool m_stdoutClosed = false;
    bool m_finished = false;
};

void RemoteSession::configure(RunControl *control, const QHostAddress &address, bool waitForDebugger)
{
    m_control = control;
    m_address = address;
    m_waitForDebugger = waitForDebugger;
}

void RemoteSession::start()
{
    if (!m_server.start(m_control->project()->projectDirectory().toFSPathString(), m_address)) {
        fail(tr("Cannot serve the Freebox project: %1").arg(m_server.errorString()));
        return;
    }

    m_stdout.setProxy(QNetworkProxy::NoProxy);
    m_stderr.setProxy(QNetworkProxy::NoProxy);
    connect(&m_remote, &Remote::QmlRemote::failed, this, &RemoteSession::fail);
    connect(&m_remote, &Remote::QmlRemote::started, this, &RemoteSession::startLogging);
    connect(&m_stdout, &QTcpSocket::readyRead, this, [this] {
        readLog(m_stdout, m_stdoutDecoder, StdOutFormat);
    });
    connect(&m_stderr, &QTcpSocket::readyRead, this, [this] {
        readLog(m_stderr, m_stderrDecoder, StdErrFormat);
    });
    auto checkConnected = [this] {
        if (!m_stdoutConnected || !m_stderrConnected || m_finished)
            return;
        m_logDeadline.stop();
        if (m_waitForDebugger)
            emit readyForTools();
        else
            m_control->reportStarted();
    };
    connect(&m_stdout, &QTcpSocket::connected, this, [this, checkConnected] {
        m_stdoutConnected = true;
        checkConnected();
    });
    connect(&m_stderr, &QTcpSocket::connected, this, [this, checkConnected] {
        m_stderrConnected = true;
        checkConnected();
    });

    const auto finish = [this] {
        if (!m_finished) {
            m_finished = true;
            emit finished(true);
        }
    };
    m_drainDeadline.setSingleShot(true);
    connect(&m_drainDeadline, &QTimer::timeout, this, finish);
    connect(&m_stdout, &QTcpSocket::disconnected, this, [this, finish] {
        readLog(m_stdout, m_stdoutDecoder, StdOutFormat);
        m_stdoutClosed = true;
        if (m_stderr.state() == QAbstractSocket::UnconnectedState && m_stderrConnected)
            finish();
        else
            m_drainDeadline.start(500);
    });
    connect(&m_stderr, &QTcpSocket::disconnected, this, [this, finish] {
        readLog(m_stderr, m_stderrDecoder, StdErrFormat);
        if (m_stdoutClosed)
            finish();
    });
    const auto logError = [this](QAbstractSocket::SocketError error) {
        if (error == QAbstractSocket::RemoteHostClosedError)
            return;
        const auto socket = static_cast<const QTcpSocket *>(sender());
        fail(tr("Freebox log connection failed: %1").arg(socket->errorString()));
    };
    connect(&m_stdout, &QTcpSocket::errorOccurred, this, logError);
    connect(&m_stderr, &QTcpSocket::errorOccurred, this, logError);
    m_logDeadline.setSingleShot(true);
    connect(&m_logDeadline, &QTimer::timeout, this, [this] {
        fail(tr("Timed out while connecting to the Freebox logs."));
    });

    m_remote.start(m_address, m_control->buildKey(), m_server.serverPort(), m_waitForDebugger);
}

void RemoteSession::startLogging(int qmlPort, int stdoutPort, int stderrPort)
{
    const auto validPort = [](int port) { return port > 0 && port <= 65535; };
    if (!validPort(stdoutPort) || !validPort(stderrPort)
        || (m_waitForDebugger && !validPort(qmlPort))) {
        fail(tr("The Freebox returned invalid debug or log ports."));
        return;
    }

    if (m_waitForDebugger) {
        QUrl url;
        url.setScheme(QStringLiteral("tcp"));
        url.setHost(m_address.toString());
        url.setPort(qmlPort);
        m_control->setQmlChannel(url);
    }
    m_logDeadline.start(10000);
    m_stdout.connectToHost(m_address, quint16(stdoutPort));
    m_stderr.connectToHost(m_address, quint16(stderrPort));
}

void RemoteSession::readLog(QTcpSocket &socket, QStringDecoder &decoder, OutputFormat format)
{
    const QString text = decoder(socket.readAll());
    if (!text.isEmpty())
        m_control->postMessage(text, format, false);
}

void RemoteSession::fail(const QString &error)
{
    if (m_finished)
        return;
    m_finished = true;
    m_control->postMessage(error, ErrorMessageFormat);
    emit finished(false);
}

class RemoteSessionAdapter
{
public:
    void operator()(RemoteSession *session, QTaskInterface *iface) const
    {
        QObject::connect(session, &RemoteSession::finished, session, [iface](bool success) {
            iface->reportDone(success ? DoneResult::Success : DoneResult::Error);
        });
        QTimer::singleShot(0, session, &RemoteSession::start);
    }
};

using RemoteSessionTask = QCustomTask<RemoteSession, RemoteSessionAdapter>;

Group remoteRecipe(RunControl *control)
{
    const auto device = std::dynamic_pointer_cast<const FreeboxDevice>(control->device());
    if (!device || device->address().isNull() || !control->project() || control->buildKey().isEmpty())
        return control->errorTask(RemoteSession::tr("No Freebox Player or QML entry point selected."));

    using namespace ProjectExplorer::Constants;
    const bool waitForDebugger = control->runMode() != NORMAL_RUN_MODE;
    const QHostAddress address = device->address();
    const RemoteSessionTask task([control, address, waitForDebugger](RemoteSession &session) {
        session.configure(control, address, waitForDebugger);
    });

    if (!waitForDebugger)
        return {task};

    if (control->runMode() == QML_PROFILER_RUN_MODE) {
        return {
            When (task, &RemoteSession::readyForTools, WorkflowPolicy::StopOnSuccessOrError) >> Do {
                QTaskTreeTask([control](QTaskTree &tree) { tree.setRecipe(control->createRecipe(QML_PROFILER_RUNNER)); })
            }
        };
    }

    auto parameters = Debugger::DebuggerRunParameters::fromRunControl(control);
    parameters.setStartMode(Debugger::AttachToQmlServer);
    parameters.setCloseMode(Debugger::DetachAtClose);
    parameters.setQmlDebugging(true);
    parameters.setUseTerminal(false);
    return {
        When (task, &RemoteSession::readyForTools, WorkflowPolicy::StopOnSuccessOrError) >> Do {
            Debugger::debuggerRecipe(control, parameters, [control](Debugger::DebuggerRunParameters &rp) {
                rp.setQmlServer(control->qmlChannel());
            })
        }
    };
}

class RemoteRunWorkerFactory final : public RunWorkerFactory
{
public:
    RemoteRunWorkerFactory()
    {
        setId("Freebox.RemoteRunWorker");
        addSupportedRunConfig(Constants::REMOTE_RUN_ID);
        addSupportedDeviceType(Constants::FREEBOX_DEVICE_TYPE);
        addSupportedRunMode(ProjectExplorer::Constants::NORMAL_RUN_MODE);
        addSupportedRunMode(ProjectExplorer::Constants::DEBUG_RUN_MODE);
        addSupportedRunMode(ProjectExplorer::Constants::QML_PROFILER_RUN_MODE);
        setRecipeProducer(remoteRecipe);
    }
};

} // namespace

void setupRunWorkers()
{
    static RemoteRunWorkerFactory remote;
    static ProcessRunnerFactory local({Constants::LOCAL_RUN_ID});
}

} // namespace Freebox

#include "runcontrol.moc"
