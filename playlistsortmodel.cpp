#include "playlistsortmodel.h"
#include <QVariant>

PlaylistSortModel::PlaylistSortModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool PlaylistSortModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    bool leftIsNumber, rightIsNumber;
    double leftVal = leftData.toDouble(&leftIsNumber);
    double rightVal = rightData.toDouble(&rightIsNumber);

    if (leftIsNumber && rightIsNumber)
        return leftVal < rightVal; // Numeric comparison
    else
        return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
}
