#ifndef PLAYLISTTABLE_H
#define PLAYLISTTABLE_H

#include "audio_tag.h"
#include "elided_header_view.h"
#include "playlist_sortmodel.h"
#include "settings_manager.h"
#include "tag_loader_worker.h"
#include <QFuture>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QStandardItemModel>
#include <QTableView>
#include <QWidget>
#include <QtConcurrent/QtConcurrent>

namespace ColumnIndex
{
enum Value
{
    Filename = 0,
    Ext,
    Path,
    Duration,
    Artist,
    Title,
    Album,
    Track,
    Year,
    Genre,
    Comment,
    Bitrate,
    Samplerate,
    Bits,
    Channels,
    Format,
    CoverSize,
    FileSize,
    LastModified,
    Rating,
    PlayCount,
    ColumnCount
};

// Definiamo un alias
// using Value = EnumValue;

// Array of column names (must match order of enum)
static const QStringList Headers =
{
    "File name",
    "Extension",
    "Path",
    "Duration",
    "Artist",
    "Title",
    "Album",
    "Track",
    "Year",
    "Genre",
    "Comment",
    "Bitrate",
    "Samplerate",
    "Bits",
    "Channels",
    "Format",
    "Cover size",
    "File size",
    "Last modified",
    "Rating",
    "Play count"
};

static const QList<int> DefaultWidths =
{
    250, // Filename
    30, // Ext
    120, // Path
    50, // Duration
    120, // Artist
    120, // Title
    120, // Album
    30, // Track
    40, // Year
    100, // Genre
    100, // Comment
    30, // Bitrate
    40, // Samplerate
    30, // Bits
    30, // Channels
    100, // Format
    60, // Cover size
    60, // File size
    80, // Last modified
    40, // Rating
    40 // Play count
};

// Helper functions
inline QString headerName(int index)
{
    if (index >= 0 && index < Headers.size())
        return Headers.at(index);
    return QString("Unknown(%1)").arg(index);
}

inline int defaultWidth(int index)
{
    if (index >= 0 && index < DefaultWidths.size())
        return DefaultWidths.at(index);
    return 100; // fallback width
}
} // namespace ColumnIndex

//class Widget;

namespace Ui
{
class PlaylistTable;
}

class PlaylistTable : public QWidget
{
    Q_OBJECT

public:
    explicit PlaylistTable(QMediaPlayer *player, QMediaPlaylist *playlist, QWidget *parent = 0);
    ~PlaylistTable() override;
    void addTrack(const QString &filePath);
    void clear();
    void playlistLoadFinished();
    void addFilesFinished();
    void setSectionsResizeMode();
    // void setKeyboardTargetWidget(Widget *target);
    void setupToolButton();
    void alignColumns(int iLastRow);
signals:
    void trackActivated(int index);
    void playlistUpdated(QMediaPlaylist *playlist);
    void playlistSorted(QMediaPlaylist *playlist);
    void isSorting(bool);
    void windowClosed();
    void focusReceived();
public slots:
    void setCurrentItemIcon(bool);
    void onDoubleClicked(const QModelIndex &index);
    void onCurrentTrackChanged(int index);
    void onHeaderSortChanged(int logicalIndex, Qt::SortOrder order);
protected:
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    // void keyPressEvent(QKeyEvent *event) override;
private slots:
    void handleNewSearchInput();
    void handleNewFilterInput();
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
    void delayedFindInTable();
    void on_pushButtonFav_clicked();
    void on_pushButtonHistory_clicked();
    void on_pushButtonPlaylist_clicked();
    void onTrackFinishedOrStopped(QMediaPlayer::State);
    void resetAllPlayCounts();
    void exportFavorites();
    void importFavorites();
    void testMigration();
private:
    ElidedHeaderView *m_HorizontalHeader;
    SettingsManager *settingsMgr;
    Ui::PlaylistTable *ui;
    QStandardItem *m_CurrentItem;
    QStandardItem *m_CurrentPlaylistItem;
    void incrementPlayCount(int sourceRow);
    void setSignalsConnections();
    void syncStagedPlaylistOrder(int sortColumn, Qt::SortOrder order);
    void syncPlaylistOrder(int sortCol, Qt::SortOrder sortOrder);
    void syncPlaylistOrder_(int sortColumn, Qt::SortOrder order);
    int m_iSortCol;
    Qt::SortOrder m_SortOrder;
    bool m_bHasBeenSorted;
    bool m_bSessionPlaylistIsShown;
    QString extractFileName(const QString &filePath);
    QMediaPlayer *m_player;
    QMediaPlaylist *m_playlist;
    QMediaPlaylist *m_stagedPlaylist;
    QStandardItemModel *m_model;
    PlaylistSortModel *m_sortModel;
    QTableView *m_view;
    void loadsettings();
    QHash<QString, int> m_FilePathToRow;
    int mapSourceRowToProxy(QAbstractItemModel *sourceModel, QSortFilterProxyModel *proxyModel, int sourceRow);
    int mapProxyRowToSource(QSortFilterProxyModel *proxyModel, int proxyRow);
    int mapRowToProxy(int sourceRow);
    int mapRowToSource(int proxyRow);
    TagLoaderWorker *m_tagWorker;
    QFutureWatcher<void> *m_FutureWatcher;
    // QList<QModelIndex> m_findMatches;    // indici sul proxy (visibili)
    // int m_findCurrentIndex = -1;
    QString m_lastSearchText;
    // QString m_lastPlaylistFile;
    int m_iLastPlaylistFile;
    QList<QPersistentModelIndex> m_findMatches;
    void updateSearchCount(int currentMatchIndex);
    const int MAX_SEARCH_HISTORY_SIZE = 20;
    const int MIN_SEARCH_CHARS = 3;
    void loadSearchHistory();
    void saveSearchHistory(const QStringList &history);
    void loadFilterHistory();
    void saveFilterHistory(const QStringList &history);
    // Widget *m_target;
    void setRating(const QModelIndex &index, int newRating);
    void saveSessionPlaylist();
    QMenu *toolButtonMenu;
    QAction *resetAllPlayCountsAction;
    QAction *deleteInexistentFilesAction;
    QAction *exportFavoritesAction;
    QAction *importFavoritesAction;
    QAction *testMigrationAction;
};

#endif // PLAYLISTTABLE_H
