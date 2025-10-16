#include "utility.h"

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
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    QString minutesStr = QString::number(minutes);
    // - Converte i secondi in stringa, assicurando due cifre (padding)
    // - campo di 2 cifre (width = 2)
    // - carattere '0' per il riempimento (fillChar = '0')
    // - allineamento a destra (right justification)
    QString secondsStr = QString::number(seconds).rightJustified(2, '0');
    return minutesStr + ":" + secondsStr;
}
