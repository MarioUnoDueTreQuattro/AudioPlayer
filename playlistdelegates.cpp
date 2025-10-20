#include "playlistdelegates.h"
#include <QString>
#include "utility.h"
#include <QAbstractItemView>

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
    // int minutes = totalSeconds / 60;
    // int seconds = totalSeconds % 60;
    // return QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0'))
    // .arg(seconds, 2, 10, QLatin1Char('0'));
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

PlaylistDelegate::PlaylistDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

bool PlaylistDelegate::helpEvent(QHelpEvent *event,
    QAbstractItemView *view,
    const QStyleOptionViewItem &option,
    const QModelIndex &index)
{
    if (!event || !view)
        return false;
    const QString text = index.data(Qt::DisplayRole).toString();
    QRect visualRect = view->visualRect(index);
    QFontMetrics fm(option.font);
    int textWidth = fm.horizontalAdvance(text);
    int availableWidth = option.rect.width() - 4; // small padding
    const int tolerance = 4;
//    LOG_VAR(textWidth);
//    LOG_VAR(availableWidth);
    bool isElided = (textWidth > (availableWidth - tolerance));
//    LOG_MSG(availableWidth+tolerance);
//    LOG_VAR(isElided);
if (isElided && visualRect.contains(event->pos()))
    {
        if (visualRect.contains(event->pos()))
        {
            QToolTip::showText(event->globalPos(), text, view->viewport());
            return true;
        }
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}
