#include "singleinstance.h"
#include <QTextStream>
#include <QDebug>
#include <QFile>

SingleInstance::SingleInstance(const QString &serverName, QObject *parent)
    : QObject(parent),
      m_serverName(serverName),
      m_localServer(nullptr)
{
}

SingleInstance::~SingleInstance()
{
    if (m_localServer)
    {
        m_localServer->close();
        // remove server name so new runs can re-create cleanly
        QLocalServer::removeServer(m_serverName);
        delete m_localServer;
    }
}

bool SingleInstance::startServer()
{
    // If a stale socket file exists (on Unix) or stale server is present, remove it first.
    // QLocalServer::removeServer is safe even if no server present.
    QLocalServer::removeServer(m_serverName);
    m_localServer = new QLocalServer(this);
    if (!m_localServer->listen(m_serverName))
    {
        qWarning() << "SingleInstance: could not listen on server:" << m_localServer->errorString();
        delete m_localServer;
        m_localServer = nullptr;
        return false;
    }
    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
    return true;
}

bool SingleInstance::sendMessageToPrimary(const QStringList &messages, int timeoutMs)
{
    QLocalSocket socket;
    socket.connectToServer(m_serverName, QIODevice::WriteOnly);
    if (!socket.waitForConnected(timeoutMs))
    {
        // maybe stale server; try removing and fail so caller can decide
        qWarning() << "SingleInstance: unable to connect to server:" << socket.errorString();
        return false;
    }
    // send newline-separated utf-8; end with extra newline
    QByteArray payload;
    for (const QString &m : messages)
    {
        payload += m.toUtf8();
        payload += '\n';
    }
    qint64 written = socket.write(payload);
    if (written != payload.size())
    {
        qWarning() << "SingleInstance: wrote" << written << "of" << payload.size();
    }
    socket.flush();
    socket.waitForBytesWritten(timeoutMs);
    socket.disconnectFromServer();
    return true;
}

void SingleInstance::handleNewConnection()
{
    QLocalSocket *clientConnection = m_localServer->nextPendingConnection();
    if (!clientConnection)
    {
        return;
    }
    connect(clientConnection, SIGNAL(readyRead()), clientConnection, SLOT(deleteLater())); // placeholder
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(handleNewConnection())); // not used; we'll read directly below
    // Read all available data (blocking read is acceptable here because data is small)
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(0)); // avoid unused-slot warnings (not ideal)
    // We'll actually read immediately (not in a lambda).
    QByteArray allData;
    while (clientConnection->waitForReadyRead(50))
    {
        allData += clientConnection->readAll();
    }
    // Ensure we read at least once
    allData += clientConnection->readAll();
    clientConnection->disconnectFromServer();
    clientConnection->deleteLater();
    if (allData.isEmpty())
    {
        return;
    }
    // Parse newline-separated UTF-8 strings
    QList<QByteArray> lines = allData.split('\n');
    QStringList messages;
    for (const QByteArray &b : lines)
    {
        if (b.trimmed().isEmpty()) continue;
        messages << QString::fromUtf8(b);
    }
    if (!messages.isEmpty())
    {
        emit receivedMessage(messages);
    }
}
