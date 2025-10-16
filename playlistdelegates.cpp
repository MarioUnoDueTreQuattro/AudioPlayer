#include "playlistdelegates.h"
#include <QString>
#include "utility.h"

PlaylistDurationDelegate::PlaylistDurationDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QString PlaylistDurationDelegate::displayText(const QVariant &value, const QLocale &) const
{
    bool ok = false;
    int totalSeconds = value.toInt(&ok);
    if (!ok)
        return value.toString();
//    int minutes = totalSeconds / 60;
//    int seconds = totalSeconds % 60;
//    return QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0'))
//        .arg(seconds, 2, 10, QLatin1Char('0'));
    return formatTime(totalSeconds);
}

PlaylistFileSizeDelegate::PlaylistFileSizeDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QString PlaylistFileSizeDelegate::displayText(const QVariant &value, const QLocale &) const
{
    bool ok = false;
    int totalBytes = value.toInt(&ok);
    if (!ok)
        return value.toString();
    return formatFileSize(totalBytes);
}
