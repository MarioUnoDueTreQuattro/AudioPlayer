#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QListWidgetItem>

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

private slots:
    void handlePlayButton();
    void handleStopButton();
    void handleClearButton();
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
private:
 int m_lastTrackIndex;     // index of last playing track
   qint64 m_lastTrackPosition; // position in milliseconds
   int m_lastVolume;                       // remember last non-mute volume
    bool m_isMuted;
    void addFileToPlaylist(const QString &filePath);
    void loadSettings();                    // <--- NEW
      void saveSettings();
      void updateMuteButtonIcon();
    Ui::Widget *ui;
    QMediaPlayer *m_player;
    QMediaPlaylist *m_playlist;
};

#endif // WIDGET_H
