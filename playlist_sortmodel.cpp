#include "playlist_sortmodel.h"
#include <QVariant>

PlaylistSortModel::PlaylistSortModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void PlaylistSortModel::setFilterText(const QString &text)
{
    if (text.length() < MIN_SEARCH_CHARS)
    {
        m_filterText = "";
        invalidateFilter();
    }
    else
    {
        m_filterText = text;
        invalidateFilter();
    }
}

void PlaylistSortModel::setFilterColumns(const QSet<int> &columns)
{
    m_filterColumns = columns;
    invalidateFilter();
}

bool PlaylistSortModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_filterText.isEmpty())
        return true;
    for (int col : m_filterColumns)
    {
        QModelIndex index = sourceModel()->index(sourceRow, col, sourceParent);
        QString cellText = sourceModel()->data(index).toString();
        if (cellText.contains(m_filterText, Qt::CaseInsensitive))
            return true;
    }
    return false;
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

QVariant PlaylistSortModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical && role == Qt::DisplayRole)
    {
        return QString::number(section + 1); // numeri ordinati
    }
    return QSortFilterProxyModel::headerData(section, orientation, role);
}
