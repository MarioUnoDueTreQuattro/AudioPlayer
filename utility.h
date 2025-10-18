#include <QDateTime>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#ifndef LOG_MACROS_H
#define LOG_MACROS_H

// Basic log with file, line, and function
#define LOG() \
    qDebug().noquote() \
    << "File:" << __FILE__ \
    << "Function:" << __FUNCTION__ \
    << "Line:" << __LINE__

// Log with custom message
#define LOG_MSG(msg) \
    qDebug().noquote() \
    << "File:" << __FILE__ \
    << "Function:" << __FUNCTION__ \
    << "Line:" << __LINE__ \
    << "Msg:" \
    << msg

// IF Condition Log with custom message
#define LOG_MSG_IF(cond, msg) \
             if (cond) qDebug().noquote() \
             << "File:" << __FILE__ \
             << "Function:" << __FUNCTION__ \
             << "Line:" << __LINE__ \
             << "Msg:" \
             << msg

// IF Condition==false Log with custom message
#define LOG_MSG_IF_FALSE(cond, msg) \
             if (cond==false) qDebug().noquote() \
             << "File:" << __FILE__ \
             << "Function:" << __FUNCTION__ \
             << "Line:" << __LINE__ \
             << "Msg:" \
             << msg

// Log a variable name and value
#define LOG_VAR(var) \
    qDebug().noquote() \
    << "File:" << __FILE__ \
    << "Function:" << __FUNCTION__ \
    << "Line:" << __LINE__ \
    << "Var:" << #var << "=" << var

// Log to file
#define LOG_FILE(msg) \
    writeToLogFile( msg, __FILE__, __FUNCTION__, __LINE__)

// Internal helper for "Log to file"
inline void writeToLogFile(const QString& message, const char* fileName, const char* function, int line)
{
    QString sFile = qApp->applicationName();
    sFile.append(".log");
    QFile file(sFile);
    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&file);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " | File: " << fileName << " | Function: " << function << " | Line: " << line << " | " << message << "\n";
        file.close();
    }
}

#endif // LOG_MACROS_H

#ifndef MYUTILITY_H
#define MYUTILITY_H

    QString formatFileSize(qint64 bytes);
    QString formatTime(int totalSeconds);

#endif // MYUTILITY_H
