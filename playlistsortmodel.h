#ifndef PLAYLISTSORTMODEL_H
#define PLAYLISTSORTMODEL_H

#include <QSortFilterProxyModel>

class PlaylistSortModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit PlaylistSortModel(QObject *parent = 0);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

#endif // PLAYLISTSORTMODEL_H
