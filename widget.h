#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QResizeEvent>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QListWidgetItem>
#include <QStack>
#include "systemvolumecontroller.h"
#include <3rdparty/qhotkey/QHotkey/QHotkey>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
public slots:
    void openFiles(const QStringList &filePaths);

protected:
    // Enable drag & drop
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void switchVolume();
    void handleVolumeUp();
    void handleVolumeDown();
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
private:
   QHotkey *m_hotkey;
   void handlePlay();
    void setKeyboardShortcuts();
    void setSignalsConnections();
    SystemVolumeController *m_systemVolumeController;
    bool m_bAutoplay;
    bool m_bSystemVolumeSlider;
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
    void loadPlaylistFile(const QString &path, bool restoreLastTrack = true);
    void savePlaylistFile(const QString &path);
    void loadSettings();                    // <--- NEW
    void saveSettings();
    void updateMuteButtonIcon();
    Ui::Widget *ui;
    QMediaPlayer *m_player;
    QMediaPlaylist *m_playlist;
};

#endif // WIDGET_H
