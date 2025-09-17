#ifndef SINGLEINSTANCE_H
#define SINGLEINSTANCE_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>

/*
 SingleInstance implements:
 - If another instance already runs, connect to it and send a message (list of file paths),
   then the second instance should exit.
 - If this is the first instance, create a QLocalServer to accept messages from future instances.

 Messages are simple newline-separated UTF-8 strings.
*/

class SingleInstance : public QObject
{
    Q_OBJECT
public:
    explicit SingleInstance(const QString &serverName, QObject *parent = nullptr);
    ~SingleInstance();

    // returns true if we are the primary (first) instance and server started successfully
    bool startServer();

    // When called by a secondary instance, send message to the running primary instance.
    // Returns true if message sent successfully.
    bool sendMessageToPrimary(const QStringList &messages, int timeoutMs = 2000);

signals:
    // Emitted on the primary instance when a new client sends messages (file paths)
    void receivedMessage(const QStringList &messages);

private slots:
    void handleNewConnection();

private:
    QString m_serverName;
    QLocalServer *m_localServer;
};

#endif // SINGLEINSTANCE_H
