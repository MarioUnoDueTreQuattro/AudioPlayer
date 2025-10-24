#include "single_instance.h"
#include "widget.h"
#include <QApplication>
#include <QDateTime>
#include <QDebug>

void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Stringa per il tipo di log
    QByteArray localMsg = msg.toLocal8Bit();
    QString logType;
    switch (type)
    {
        case QtDebugMsg:
            logType = "DEBUG";
            break;
        case QtInfoMsg:
            logType = "INFO ";
            break;
        case QtWarningMsg:
            logType = "WARN ";
            break;
        case QtCriticalMsg:
            logType = "CRIT ";
            break;
        case QtFatalMsg:
            logType = "FATAL";
            // Nei messaggi FATAL, l'applicazione si interrompe subito,
            // ma è comunque bene stamparli.
            break;
        default:
            return;
    }
    // Estrae l'ora corrente
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    // Prepara la riga di output formattata
    // Formato: [DATA/ORA] [TIPO] (File:riga, Funzione) Messaggio
    QString output = QString("[%1] [%2] (%3:%4, %5) %6")
        .arg(currentDateTime)
        .arg(logType)
        .arg(context.file)
        .arg(context.line)
        .arg(context.function)
        .arg(msg);
    // Stampa su console (stderr è standard per i log)
    fprintf(stdout, "%s\n", output.toLocal8Bit().constData());
}

int main(int argc, char *argv[])
{
    // qInstallMessageHandler(customMessageOutput);
    // qputenv("QT_DEBUG_PLUGINS", "1");
    // qputenv("QT_MEDIA_DEBUG", "1");
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
    if (connectedToPrimary) //&& !args.isEmpty())
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
