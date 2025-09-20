#include <QApplication>
#include <QDebug>
#include "singleinstance.h"
#include "widget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("andreag");
    QCoreApplication::setApplicationName("AudioPlayer");
    QString serverName = "SingleInstancePlayerUniqueServer";
    SingleInstance instance(serverName);
    QStringList args;
    for (int i = 1; i < argc; ++i)
    {
        args << QString::fromLocal8Bit(argv[i]);
    }
    // Try sending files to running instance
    bool connectedToPrimary = instance.sendMessageToPrimary(args);
    if (connectedToPrimary )//&& !args.isEmpty())
    {
        qDebug() << "Sent files to running instance. Exiting.";
        return 0;
    }
    if (!instance.startServer())
    {
        qWarning() << "Could not start single-instance server; continuing anyway.";
    }
    Widget w;
    w.show();
    if (!args.isEmpty())
    {
        w.openFiles(args);
    }
    // When second instance sends files, open them and bring widget to front
    QObject::connect(&instance, SIGNAL(receivedMessage(QStringList)),
        &w, SLOT(openFiles(QStringList)));
    return app.exec();
}
