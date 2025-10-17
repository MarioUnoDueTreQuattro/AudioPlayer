#ifndef PLAYLISTSORTMODEL_H
#define PLAYLISTSORTMODEL_H

#include <QSortFilterProxyModel>
#include <QSet>

class PlaylistSortModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit PlaylistSortModel(QObject *parent = 0);
    void setFilterText(const QString &text);
    void setFilterColumns(const QSet<int> &columns);
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
private:
    QString m_filterText;
    QSet<int> m_filterColumns;
};

#endif // PLAYLISTSORTMODEL_H
