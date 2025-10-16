#ifndef PLAYLISTTABLE_H
#define PLAYLISTTABLE_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QMediaPlaylist>
#include <QMediaPlayer>
#include "audiotag.h"
#include "playlistsortmodel.h"
#include "settingsmanager.h"

namespace Ui
{
class PlaylistTable;
}

class PlaylistTable : public QWidget
{
    Q_OBJECT

public:
    explicit PlaylistTable(QMediaPlayer *player, QWidget *parent = 0);
    ~PlaylistTable();
    void addTrack(const QString &filePath);
    void addTrackExt(const QString &filePath);
    void clear();
    QMediaPlaylist *mediaPlaylist() const { return m_playlist; }
    void playlistLoadFinished();
    void setSectionsResizeMode();
signals:
    void trackActivated(int index);
    void playlistUpdated(QMediaPlaylist *playlist);
    void windowClosed();
    void focusReceived();
public slots:
    void onDoubleClicked(const QModelIndex &index);
    void onCurrentTrackChanged(int index);
    void onHeaderSortChanged(int logicalIndex, Qt::SortOrder order);
protected:
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
private slots:
    void on_pushButton_clicked();
    void onTagLoadingFinished();
    void onTagLoaded(const QString &filePath, const AudioTagInfo &info);
    void on_pushButton_2_clicked();
private:
        SettingsManager *settingsMgr;
 Ui::PlaylistTable *ui;
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
};

#endif // PLAYLISTTABLE_H
