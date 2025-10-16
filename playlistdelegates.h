#ifndef PLAYLISTDELEGATES_H
#define PLAYLISTDELEGATES_H

#include <QStyledItemDelegate>

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

#endif // PLAYLISTDELEGATES_H
