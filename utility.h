#include <QDateTime>
// #include <QApplication>

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

QString formatFileSize(qint64 bytes)
{
    const double KB = 1024.0;
    const double MB = KB * 1024.0;
    if (bytes < MB)
    {
        return QString::number(bytes / KB, 'f', 2) + " KB";
    }
    else
    {
        return QString::number(bytes / MB, 'f', 2) + " MB";
    }
}

QString formatTime(int totalSeconds)
{
    // 1. Calcola i minuti
    int minutes = totalSeconds / 60;
    // 2. Calcola i secondi rimanenti
    int seconds = totalSeconds % 60;
    // 3. Converte i minuti in stringa
    // Il formato è semplice, non è necessario il padding per i minuti
    QString minutesStr = QString::number(minutes);
    // 4. Converte i secondi in stringa, assicurando due cifre (padding)
    // - campo di 2 cifre (width = 2)
    // - carattere '0' per il riempimento (fillChar = '0')
    // - allineamento a destra (right justification)
    QString secondsStr = QString::number(seconds).rightJustified(2, '0');
    // 5. Combina il risultato nel formato finale
    return minutesStr + ":" + secondsStr;
}
