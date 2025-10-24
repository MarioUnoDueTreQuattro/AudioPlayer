#include "playlist_delegates.h"
#include <QString>
#include <QPainter>
#include <QPainterPath>
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
    // LOG_VAR(textWidth);
    // LOG_VAR(availableWidth);
    bool isElided = (textWidth > (availableWidth - tolerance));
    // LOG_MSG(availableWidth+tolerance);
    // LOG_VAR(isElided);
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

//void PlaylistDelegate::paint(QPainter *painter,
// const QStyleOptionViewItem &option,
// const QModelIndex &index) const
//{
// QStyledItemDelegate::paint(painter, option, index);

// const QAbstractItemModel *model = index.model();
// if (!model)
// return;

// const int row = index.row();
// const int columnCount = model->columnCount();

// bool rowHighlighted = false;
// for (int col = 0; col < columnCount; ++col)
// {
// QVariant flag = model->data(model->index(row, col), Qt::UserRole + 10);
// if (flag.isValid() && flag.toBool())
// {
// rowHighlighted = true;
// break;
// }
// }

// if (rowHighlighted)
// {
// painter->save();

//        // Semi-transparent yellow overlay over the full cell
// QColor overlayColor(255, 255, 128, 80);
// painter->fillRect(option.rect, overlayColor);

//        // Optional border for emphasis
// QPen pen(QColor(255, 200, 0, 180));
// pen.setWidth(1);
// painter->setPen(pen);
// painter->drawRect(option.rect.adjusted(0, 0, -1, -1));

// painter->restore();
// }
//}

void PlaylistDelegate::paint(QPainter *painter,
    const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    // Normal painting first
    QStyledItemDelegate::paint(painter, option, index);
    // Check custom highlight flag (Qt::UserRole + 10)
    QVariant highlightFlag = index.data(Qt::UserRole + 10);
    if (highlightFlag.isValid() && highlightFlag.toBool())
    {
        painter->save();
        // Transparent yellow overlay
        QColor overlayColor(255, 255, 128, 32);
        painter->fillRect(option.rect, overlayColor);
        // Optional border for clarity
        QPen pen(QColor(255, 200, 0, 255));
        // QPen pen(QColor(255, 200, 0, 180));
        pen.setWidth(2);
        painter->setPen(pen);
        painter->drawRect(option.rect.adjusted(0, 0, -1, -1));
        painter->restore();
    }
}

/*
 * void PlaylistDelegate::paint(QPainter *painter,
    const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    // Normal painting first
    QStyledItemDelegate::paint(painter, option, index);

    // Check custom highlight flag (Qt::UserRole + 10)
    QVariant highlightFlag = index.data(Qt::UserRole + 10);
    if (highlightFlag.isValid() && highlightFlag.toBool())
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        // Rounded rectangle with some padding
        QRect roundedRect = option.rect.adjusted(2, 1, -2, -1);
        int radius = 8;

        // Create a gradient for smooth visual effect
        QLinearGradient gradient(roundedRect.topLeft(), roundedRect.bottomLeft());
        gradient.setColorAt(0.0, QColor(255, 250, 205, 60));  // Light yellow top
        gradient.setColorAt(1.0, QColor(255, 235, 150, 45));  // Slightly darker bottom

        // Fill with gradient using rounded rectangle
        QPainterPath path;
        path.addRoundedRect(roundedRect, radius, radius);
        painter->fillPath(path, gradient);

        // Subtle border around the rounded cell
        QPen border(QColor(255, 215, 0, 120));
        border.setWidth(2);
        painter->setPen(border);
        painter->drawRoundedRect(roundedRect, radius, radius);

        // Optional: add a subtle glow effect on top edge
        QRect topGlowRect(roundedRect.left(), roundedRect.top(),
                          roundedRect.width(), 2);
        QLinearGradient topGlow(topGlowRect.topLeft(), topGlowRect.bottomLeft());
        topGlow.setColorAt(0.0, QColor(255, 255, 255, 40));
        topGlow.setColorAt(1.0, QColor(255, 255, 255, 0));

        QPainterPath glowPath;
        glowPath.addRoundedRect(roundedRect.adjusted(1, 1, -1, roundedRect.height() - 3),
                                radius - 1, radius - 1);
        painter->setClipPath(glowPath);
        painter->fillRect(topGlowRect, topGlow);
        painter->setClipping(false);

        painter->restore();
    }
}
*/
/*void PlaylistDelegate::paint(QPainter *painter,
    const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
      // Normal painting AFTER highlight (so text is on top)
    QStyledItemDelegate::paint(painter, option, index);
  // Check custom highlight flag (Qt::UserRole + 10)
    QVariant highlightFlag = index.data(Qt::UserRole + 10);
    if (highlightFlag.isValid() && highlightFlag.toBool())
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        // Rounded rectangle with some padding
        QRect roundedRect = option.rect.adjusted(0, 0, 0, 0);
        int radius = 6;

        // Create a gradient for smooth visual effect
//        QLinearGradient gradient(roundedRect.topLeft(), roundedRect.bottomLeft());
//        gradient.setColorAt(0.0, QColor(255, 245, 157, 32));  // More opaque light yellow top
//        gradient.setColorAt(1.0, QColor(255, 220, 100, 16));  // More opaque darker bottom

//        // Fill with gradient using rounded rectangle
//        QPainterPath path;
//        path.addRoundedRect(roundedRect, radius, radius);
//        painter->fillPath(path, gradient);

        // Subtle border around the rounded cell
        QPen border(QColor(255, 200, 0, 255));
        border.setWidth(2);
        painter->setPen(border);
        painter->drawRoundedRect(roundedRect, radius, radius);

        // Optional: add a subtle glow effect on top edge
//        QRect topGlowRect(roundedRect.left(), roundedRect.top(),
//                          roundedRect.width(), 2);
//        QLinearGradient topGlow(topGlowRect.topLeft(), topGlowRect.bottomLeft());
//        topGlow.setColorAt(0.0, QColor(255, 255, 255, 128));
//        topGlow.setColorAt(1.0, QColor(255, 255, 255, 0));

//        QPainterPath glowPath;
//        glowPath.addRoundedRect(roundedRect.adjusted(1, 1, -1, roundedRect.height() - 3),
//                                radius - 1, radius - 1);
//        painter->setClipPath(glowPath);
//        painter->fillRect(topGlowRect, topGlow);
        painter->setClipping(false);

        painter->restore();
    }

}

*/
