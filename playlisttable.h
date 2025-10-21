#ifndef PLAYLISTTABLE_H
#define PLAYLISTTABLE_H

#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QMediaPlaylist>
#include <QMediaPlayer>
#include "audiotag.h"
#include "playlistsortmodel.h"
#include "settingsmanager.h"
#include "tagloaderworker.h"

namespace Ui
{
class PlaylistTable;
}

class PlaylistTable : public QWidget
{
    Q_OBJECT

public:
    explicit PlaylistTable(QMediaPlayer *player, QWidget *parent = 0);
    ~PlaylistTable() override;
    void addTrack(const QString &filePath);
    void clear();
    QMediaPlaylist *mediaPlaylist() const { return m_playlist; }
    void playlistLoadFinished();
    void addFilesFinished();
    void setSectionsResizeMode();
signals:
    void trackActivated(int index);
    void playlistUpdated(QMediaPlaylist *playlist);
    void isSorting(bool);
    void windowClosed();
    void focusReceived();
public slots:
    void setCurrentItemIcon(bool);
    void onDoubleClicked(const QModelIndex &index);
    void onCurrentTrackChanged(int index);
    void onHeaderSortChanged(int logicalIndex, Qt::SortOrder order);
    void setPlaylist(QMediaPlaylist *playlist);
protected:
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
private slots:
    void on_pushButton_clicked();
    void readTags();
    void readPlaylistTags();
    void onTagLoadingFinished();
    void onTagLoaded(const QString &filePath, const AudioTagInfo &info);
    void on_pushButton_2_clicked();
    void onHeaderContextMenu(const QPoint &pos);
    void restoreColumnVisibility();
    void onColumnResized(int column, int oldSize, int newSize);
    void restoreColumnWidths();
    void findInTable(const QString &searchText);
    void findNext();
    void findPrevious();
    void onClicked(const QModelIndex &index);
    void showPlaylistContextMenu(const QPoint &pos);
    void clearSearchHighlight();
private:
    SettingsManager *settingsMgr;
    Ui::PlaylistTable *ui;
    QStandardItem *m_CurrentItem;
    void syncPlaylistOrder();
    QString extractFileName(const QString &filePath);
    QMediaPlayer *m_player;
    QMediaPlaylist *m_playlist;
    QStandardItemModel *m_model;
    PlaylistSortModel *m_sortModel;
    QTableView *m_view;
    void loadsettings();
    QHash<QString, int> m_FilePathToRow;
    int mapSourceRowToProxy(QAbstractItemModel *sourceModel, QSortFilterProxyModel *proxyModel, int sourceRow);
    int mapProxyRowToSource(QSortFilterProxyModel *proxyModel, int proxyRow);
    TagLoaderWorker *m_tagWorker;
    QFutureWatcher<void> *m_FutureWatcher;
    // QList<QModelIndex> m_findMatches;    // indici sul proxy (visibili)
    // int m_findCurrentIndex = -1;
    QString m_lastSearchText;
    QList<QPersistentModelIndex> m_findMatches;
    void updateSearchCount(int currentMatchIndex);
};

#endif // PLAYLISTTABLE_H
