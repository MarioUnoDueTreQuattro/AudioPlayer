#ifndef PLAYLISTDELEGATES_H
#define PLAYLISTDELEGATES_H

#include <QStyledItemDelegate>
#include <QToolTip>
#include <QHelpEvent>

class PlaylistDurationDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit PlaylistDurationDelegate(QObject *parent = 0);

    QString displayText(const QVariant &value, const QLocale &locale) const override;
};

class PlaylistFileSizeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit PlaylistFileSizeDelegate(QObject *parent = 0);

    QString displayText(const QVariant &value, const QLocale &locale) const override;
};

class PlaylistDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit PlaylistDelegate(QObject *parent = 0);
    bool helpEvent(QHelpEvent *event,
        QAbstractItemView *view,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) override;
    void paint(QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
};

class PlaylistRatingDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    // Constructor now only takes the QObject parent
    PlaylistRatingDelegate(QObject *parent = nullptr);

    // Override the paint method to draw icons
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

private:
    QIcon m_fullStarIcon;
    QSize m_iconSize;

    // Define the resource path internally
    static const QString STAR_RESOURCE_PATH;

    //static const int MAX_RATING = 5;
    //static const int STAR_SPACING = 4;
};
#endif // PLAYLISTDELEGATES_H
