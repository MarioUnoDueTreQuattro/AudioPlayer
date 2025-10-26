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
    // QStyleOptionViewItem opt(option);
    // initStyleOption(&opt, index);
    // opt.textElideMode = Qt::ElideNone;
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

// Define the static resource path here. Adjust this if your QRC path is different.
const QString PlaylistRatingDelegate::STAR_RESOURCE_PATH = ":/img/img/icons8-star-48.png";
const int STAR_SPACING = 0;
static const int DEFAULT_STAR_SIZE = 24;
const int MAX_RATING = 5;

#include <QDebug>
#include <QStyleOptionViewItem>
#include <QStyle>
#include <QApplication>
#include <QLocale> // Necessario per una conversione pulita

PlaylistRatingDelegate::PlaylistRatingDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
      m_iconSize(DEFAULT_STAR_SIZE, DEFAULT_STAR_SIZE)
{
    // --- ICON LOADING (Internal) ---
    m_fullStarIcon.addFile(STAR_RESOURCE_PATH);
    QSize sizeHint = m_fullStarIcon.actualSize(m_iconSize, QIcon::Normal, QIcon::On);
    if (sizeHint.isValid() && sizeHint.width() > 0)
    {
        m_iconSize = sizeHint;
    }
}

//void PlaylistRatingDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
// const QModelIndex &index) const
//{
//    // 1. Retrieve the data using the standard DisplayRole
// QVariant data = index.data(Qt::DisplayRole);

// if (!data.isValid()) {
//        // Fallback to default painting if no data is present
// QStyledItemDelegate::paint(painter, option, index);
// return;
// }

//    // 2. Convert the cell content (QString) to an integer
// bool ok = false;
//    // We use QLocale().toInt() to handle potential localization differences if needed,
//    // but QString::toInt() is usually sufficient for simple integers.
// int rating = data.toString().toInt(&ok);

// if (!ok) {
//        // If conversion fails (e.g., cell contains "N/A" or "Rating"),
//        // draw the original text using the default delegate behavior.
// QStyledItemDelegate::paint(painter, option, index);
// return;
// }

//    // 3. Clamp the rating and proceed with drawing
// rating = qBound(0, rating, MAX_RATING);

// const int iconWidth = m_iconSize.width();
// const int iconHeight = m_iconSize.height();

// const int totalWidth = MAX_RATING * iconWidth + (MAX_RATING - 1) * STAR_SPACING;

// int currentX = option.rect.left() + (option.rect.width() - totalWidth) / 2;
// int currentY = option.rect.center().y() - iconHeight / 2;

// painter->save();

//    // Handle cell background and selection state
// painter->fillRect(option.rect, option.state & QStyle::State_Selected ? option.palette.highlight() : option.palette.base());

// for (int i = 0; i < MAX_RATING; ++i) {
// QRect iconRect(currentX, currentY, iconWidth, iconHeight);

// if (i < rating) {
// m_fullStarIcon.paint(painter, iconRect, Qt::AlignCenter, QIcon::Normal, QIcon::On);
// }

// currentX += iconWidth + STAR_SPACING;
// }

// painter->restore();
//}

void PlaylistRatingDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    QVariant data = index.data(Qt::DisplayRole);
    // 1. Lettura e conversione del Rating
    bool ok = false;
    int rating = data.toString().toInt(&ok);
    // Se la conversione fallisce, usiamo il comportamento predefinito.
    if (!ok | rating < 0)
    {
        QStyleOptionViewItem opt_bg = option;
        QStyle *style = opt_bg.widget ? opt_bg.widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt_bg, painter, opt_bg.widget);
        // QStyledItemDelegate::paint(painter, opt_base, index);
        return;
    }
    QStyleOptionViewItem opt_bg = option;
    QStyle *style = opt_bg.widget ? opt_bg.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt_bg, painter, opt_bg.widget);
    // 2. Preparazione per il disegno
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    // Gestione dello stato di selezione/sfondo della cella
    //painter->fillRect(option.rect, option.state & QStyle::State_Selected ? option.palette.highlight() : option.palette.base());
    const int iconWidth = m_iconSize.width();
    const int iconHeight = m_iconSize.height();
    // Calcoliamo la metrica del font per centrare il testo
    QFont font = option.font;
    font.setBold(true); // Rendi il numero più leggibile
    painter->setFont(font);
    QFontMetrics fm = painter->fontMetrics();
    QString ratingText = QString::number(rating);
    int textWidth = fm.width(ratingText);
    int textHeight = fm.height();
    // 3. Calcolo delle posizioni (Centrato nella cella)
    // Larghezza totale necessaria (Icona + Piccolo spazio + Testo)
    const int totalWidth = iconWidth + STAR_SPACING + textWidth;
    // Punto di partenza X per centrare il blocco combinato (Icona + Testo)
    // int startX = option.rect.left() + (option.rect.width() - totalWidth) / 2;
    int startX = option.rect.center().x() - iconWidth / 2;
    // Posizione Y per l'icona (centrata verticalmente)
    int iconY = option.rect.center().y() - iconHeight / 2;
    // Posizione Y per il testo (centrata verticalmente)
    //int textY = option.rect.center().y() + textHeight / 2 - fm.descent(); // Allinea il testo con la base dell'icona (approssimazione)
    int textY = option.rect.center().y() + textHeight / 2 - fm.descent();
    // 4. Disegno dell'Icona (Stella)
    QRect iconRect(startX, iconY, iconWidth, iconHeight);
    if (!m_fullStarIcon.isNull())
    {
        m_fullStarIcon.paint(painter, iconRect, Qt::AlignCenter, QIcon::Normal, QIcon::On);
    }
    // 5. Disegno del Testo (Rating)
    // Posizione X del testo
    //int textX = startX + iconWidth + STAR_SPACING;
    int textX = startX - textWidth / 2 + iconWidth / 2;
    // Imposta il colore del testo (es. Bianco o Colore di Primo Piano)
    // painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : option.palette.text().color());
    painter->setPen(Qt::black);
    painter->drawText(textX, textY + 1, ratingText);
    painter->restore();
}

QSize PlaylistRatingDelegate::sizeHint(const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    // Size logic remains the same
    const int totalWidth = MAX_RATING * m_iconSize.width() + (MAX_RATING - 1) * STAR_SPACING + 10;
    const int totalHeight = m_iconSize.height() + 4;
    int defaultHeight = QStyledItemDelegate::sizeHint(option, index).height();
    return QSize(totalWidth, qMax(totalHeight, defaultHeight));
}
