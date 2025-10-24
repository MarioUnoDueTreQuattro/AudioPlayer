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

#endif // PLAYLISTDELEGATES_H
