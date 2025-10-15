#ifndef PLAYLISTDURATIONDELEGATE_H
#define PLAYLISTDURATIONDELEGATE_H

#include <QStyledItemDelegate>

class PlaylistDurationDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit PlaylistDurationDelegate(QObject *parent = 0);

    QString displayText(const QVariant &value, const QLocale &locale) const override;
};

#endif // PLAYLISTDURATIONDELEGATE_H
