#include "elided_header_view.h"
#include <QMouseEvent>
#include <QFontMetrics>
#include <QAbstractItemModel>

ElidedHeaderView::ElidedHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setMouseTracking(true);
}

void ElidedHeaderView::mouseMoveEvent(QMouseEvent *event)
{
    int logicalIndex = logicalIndexAt(event->pos());
    if (logicalIndex < 0)
        return;

    QVariant headerData = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole);
    QString text = headerData.toString();
    if (text.isEmpty())
        return;

    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(text) +10;
    int sectionWidth = sectionSize(logicalIndex) - 4; // piccola tolleranza per padding

    if (textWidth > sectionWidth) {
        QToolTip::showText(event->globalPos(), text, this);
    } else {
        QToolTip::hideText();
    }

    QHeaderView::mouseMoveEvent(event);
}

void ElidedHeaderView::leaveEvent(QEvent *event)
{
    QToolTip::hideText();
    QHeaderView::leaveEvent(event);
}
