#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QResizeEvent>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QListWidgetItem>
#include <QStack>
#include "settings_manager.h"
#include "system_volume_controller.h"
#include "audio_fader.h"
//#include <3rdparty/qhotkey/QHotkey/QHotkey>
#include "info_widget.h"
#include "playlist_table.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;
    void setInfoWidgetTitle();
    // QAction *getActionPlay() const { return m_playAction; }
public slots:
    void openFiles(const QStringList &filePaths);
protected:
    void showEvent(QShowEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void changeEvent(QEvent *event) override;
private slots:
    void filterList(const QString &text);
    void playlistTableWindowFocusReceived();
    void playlistTableWindowClosed();
    void infoWindowFocusReceived();
    void infoWindowClosed();
    void openGoogleSearch(const QString &text);
    void openFolderAndSelectFile(const QString &filePath);
    void openFolderAndSelectFileEx(const QString &filePath);
    void openFolderAndSelectFileInExplorer(const QString &filePath);
    void switchVolume();
    void handleVolumeUp();
    void handleVolumeDown();
    void handlePositionForward();
    void handlePositionBackward();
    void onSystemVolumeChanged(float newVolume);
    void onSystemMuteChanged(bool muted);
    void onDefaultDeviceChanged();
    void onDeviceChanged(const QString &deviceId, const QString &friendlyName);
    void settingsDialogAccepted();
    void handleSilenceFinished(QAudio::State state);
    void handleModeButton();  // NEW slot
    void updateModeButtonIcon(); // Helper to update icon
    void handlePlayButton();
    void handleStopButton();
    void clearExceptSelected();
    void clearPlaylist(bool silent = false);
    void handleClearPlaylist();
    void handleItemDoubleClicked();
    void handleMediaStateChanged(QMediaPlayer::State state);
    void handleMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void handlePlaylistCurrentIndexChanged(int index);
    void handleVolumeChanged(int value);
    void handleMuteButton();
    void handlePositionChanged(qint64 position);  // NEW
    void handleDurationChanged(qint64 duration);  // NEW
    void handleSliderMoved(int position);        // NEW
    void handleMediaError(QMediaPlayer::Error error);
    void updateLastTrackPosition(qint64 position);
    void handlePauseButton();
    void on_prevButton_clicked();
    void on_nextButton_clicked();
    void on_configureButton_clicked();
    void handleSavePlaylist();
    void handleLoadPlaylist();
    void showPlaylistContextMenu(const QPoint &pos);
    void showModeButtonContextMenu(const QPoint &pos);
    void showVolumeSliderContextMenu(const QPoint &pos);
    void handleRemoveSelected();
    void on_pushButtonResetFilter_clicked();
    void playlistUpdated(QMediaPlaylist *playlist);
    void playlistSorted(QMediaPlaylist *playlist);
    void handlePlaylistCurrentIndexChangedByTable(int index);
    void showPlaylistTable();
    void playlistIsSorting(bool bIsSorting);
    void reloadPlaylist();
    void showAbout();
private:
    // QAction *m_playAction;
    // bool m_bPlaylistFinished;
    bool m_bIsInShuffleMode;
    bool m_bUserRequestedPlayback;
    // QHotkey *m_hotkey;
    int findTrackIndex(const QString &filePath);
    void forgetPlayed();
    void handlePlay();
    void copyCurrentName();
    void copyCurrentFullPath();
    void setKeyboardShortcuts();
    void setSignalsConnections();
    SettingsManager *settingsMgr;
    AudioFader *musicFader;
    SystemVolumeController *m_systemVolumeController;
    bool m_bTablePlaylist;
    bool m_bVolumeFade;
    bool m_bVolumeFadeDisabled;
    int m_iVolumeFadeTime;
    bool m_bAutoplay;
    bool m_bShowInfo;
    bool m_bInfoWindowHasBeenMinimized;
    bool m_bInfoWindowHasBeenClosed;
    bool m_bPlaylistTableWindowHasBeenClosed;
    bool m_bPlaylistTableWindowHasBeenShown;
    bool m_bSystemVolumeSlider;
    QSet<QString> m_playlistPaths;
    QList<QString> m_playedList;
    QColor m_playedTextColor;
    QString m_sTheme;
    QString m_sPalette;
    void setTheme();
    QString currentTrackName();
    void playSilence(int ms = 500);
    void scrollToCurrentTrack();
    QString m_lastPlaylistPath;
    QString m_lastDialogPlaylistPath;
    QStack<int> m_shuffleHistory;  // store played indices in shuffle mode
    int m_lastTrackIndex;     // index of last playing track
    qint64 m_lastTrackPosition; // position in milliseconds
    int m_lastVolume;                       // remember last non-mute volume
    bool m_isMuted;
    void addFileToPlaylist(const QString &filePath);
    void loadPlaylistFile(const QString &path, bool restoreLastTrack = true, bool savePlaylist = false);
    void savePlaylistFile(const QString &path, bool bSaveDialogPlaylistPath = true, bool bExtendedM3U = true);
    void savePlaylist();
    void loadSettings();                    // <--- NEW
    void saveSettings();
    void updateMuteButtonIcon();
    Ui::Widget *ui;
    QMediaPlayer *m_player;
    QMediaPlaylist *m_playlist;
    InfoWidget *m_infoWidget;
    // PlaylistView *m_playlistView;
    PlaylistTable *m_playlistView;
    void startUp();
    QMenu *toolButtonMenu;
    QAction *settingsAction;
    QAction *aboutAction;
    void setupToolButton();
    // QWidget interface
};

#endif // WIDGET_H
