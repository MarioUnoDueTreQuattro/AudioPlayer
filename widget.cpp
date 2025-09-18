#include "widget.h"
#include "ui_widget.h"
#include <QFileInfo>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QDebug>
#include <QSettings>
#include <QApplication>
#include <QTime>
#include <QMessageBox>
#include <QDir>
#include <QFileInfoList>
#include <QDirIterator>
#include <QRandomGenerator>
#include <QScreen>
#include <QGuiApplication>

Widget::Widget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Widget),
      m_player(new QMediaPlayer(this)),
      m_playlist(new QMediaPlaylist(this)),
      m_lastVolume(50),
      m_isMuted(false)
{
    ui->setupUi(this);
    setWindowTitle ("AudioPlayer");
    setAcceptDrops(true);
    m_player->setPlaylist(m_playlist);
    loadSettings();  // Load volume/mute state before connecting slider
    // Apply loaded volume
    m_player->setVolume(m_lastVolume);
    ui->volumeSlider->setValue(m_lastVolume);
    updateMuteButtonIcon();
    ui->listWidget -> setAlternatingRowColors(true);
    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(handlePlayButton()));
    connect(ui->pauseButton, SIGNAL(clicked()), this, SLOT(handlePauseButton()));
    connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(handleStopButton()));
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(handleClearButton()));
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(handleVolumeChanged(int)));
    connect(ui->muteButton, SIGNAL(clicked()), this, SLOT(handleMuteButton())); // NEW
    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
        this, SLOT(handleItemDoubleClicked()));
    connect(m_player, SIGNAL(stateChanged(QMediaPlayer::State)),
        this, SLOT(handleMediaStateChanged(QMediaPlayer::State)));
    connect(m_playlist, SIGNAL(currentIndexChanged(int)),
        this, SLOT(handlePlaylistCurrentIndexChanged(int)));
    connect(m_player, SIGNAL(positionChanged(qint64)),
        this, SLOT(handlePositionChanged(qint64)));
    connect(m_player, SIGNAL(durationChanged(qint64)),
        this, SLOT(handleDurationChanged(qint64)));
    connect(ui->positionSlider, SIGNAL(sliderMoved(int)),
        this, SLOT(handleSliderMoved(int)));
    connect(m_player, SIGNAL(error(QMediaPlayer::Error)),
        this, SLOT(handleMediaError(QMediaPlayer::Error)));
    connect(m_player, SIGNAL(positionChanged(qint64)),
        this, SLOT(updateLastTrackPosition(qint64)));
    connect(ui->modeButton, SIGNAL(clicked()), this, SLOT(handleModeButton()));
}

Widget::~Widget()
{
    // saveSettings();
    delete ui;
}

void Widget::closeEvent(QCloseEvent *event)
{
    QSettings settings(QApplication::organizationName(),
        QApplication::applicationName());
    // Save window geometry & state
    settings.setValue("windowGeometry", saveGeometry());
    // settings.setValue("windowState", saveState());
    // Save volume & mute
    settings.setValue("volume", m_lastVolume);
    settings.setValue("muted", m_isMuted);
    // Save playlist files
    QStringList playlistFiles;
    for (int i = 0; i < m_playlist->mediaCount(); ++i)
    {
        QUrl url = m_playlist->media(i).canonicalUrl();
        if (url.isLocalFile())
            playlistFiles << url.toLocalFile();
    }
    settings.setValue("playlistFiles", playlistFiles);
    // Save last track & position
    settings.setValue("lastTrackIndex", m_playlist->currentIndex());
    settings.setValue("lastTrackPosition", m_player->position());
    QWidget::closeEvent(event); // call base implementation
}

void Widget::openFiles(const QStringList &filePaths)
{
    this->showNormal();
    this->raise();
    this->activateWindow();
    // for (const QString &path : filePaths)
    // {
    // addFileToPlaylist(path);
    // }
    // int trackToPlay = 0;
    // if (m_playlist->mediaCount() > 0)
    // {
    //        // Restore last track index if valid
    // if (m_lastTrackIndex >= 0 && m_lastTrackIndex < m_playlist->mediaCount())
    // {
    // trackToPlay = m_lastTrackIndex;
    // }
    // m_playlist->setCurrentIndex(trackToPlay);
    // m_player->play();
    //        // Restore last position if available
    // if (m_lastTrackPosition > 0)
    // {
    // m_player->setPosition(m_lastTrackPosition);
    // }
    // }
    int firstNewIndex = m_playlist->mediaCount(); // track where new files start
    for (const QString &path : filePaths)
    {
        addFileToPlaylist(path);
    }
    int trackToPlay = 0;
    if (!filePaths.isEmpty())
    {
        // Play the first newly added file
        trackToPlay = firstNewIndex;
        m_lastTrackPosition = 0; // start from beginning
    }
    else if (m_playlist->mediaCount() > 0)
    {
        // Restore last track index if valid
        if (m_lastTrackIndex >= 0 && m_lastTrackIndex < m_playlist->mediaCount())
        {
            trackToPlay = m_lastTrackIndex;
        }
    }
    if (m_playlist->mediaCount() > 0)
    {
        m_playlist->setCurrentIndex(trackToPlay);
        m_player->play();
        // Restore last position only if not starting a new file
        if (filePaths.isEmpty() && m_lastTrackPosition > 0)
        {
            m_player->setPosition(m_lastTrackPosition);
        }
    }
    saveSettings();
}

void Widget::addFileToPlaylist(const QString &filePath)
{
    QFileInfo fi(filePath);
    if (!fi.exists())
        return;
    QString ext = fi.suffix().toLower();
    if (ext != "wav" && ext != "mp3" && ext != "m4a")
        return;
    // Avoid duplicate entries
    for (int i = 0; i < m_playlist->mediaCount(); ++i)
    {
        if (m_playlist->media(i).canonicalUrl() == QUrl::fromLocalFile(fi.absoluteFilePath()))
            return;
    }
    m_playlist->addMedia(QUrl::fromLocalFile(fi.absoluteFilePath()));
    ui->listWidget->addItem(fi.fileName());
}

// ---------- Drag & Drop Support ----------

void Widget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

// Not recursive

//void Widget::dropEvent(QDropEvent *event)
//{
// QStringList files;

// const QList<QUrl> urls = event->mimeData()->urls();
// for (const QUrl &url : urls) {
// if (url.isLocalFile()) {
// QFileInfo fi(url.toLocalFile());
// if (fi.isDir()) {
//                // If a directory is dropped, recursively add audio files
// QDir dir(fi.absoluteFilePath());
// QFileInfoList fileList = dir.entryInfoList(QStringList() << "*.mp3" << "*.wav" << "*.m4a",
// QDir::Files | QDir::NoDotAndDotDot,
// QDir::Name);
// for (const QFileInfo &f : fileList) {
// files << f.absoluteFilePath();
// }
// } else {
// files << fi.absoluteFilePath();
// }
// }
// }

// if (!files.isEmpty()) {
// openFiles(files);
// }
//}

// Recursive

#include <QDirIterator>

void Widget::dropEvent(QDropEvent *event)
{
    QStringList files;
    const QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls)
    {
        if (url.isLocalFile())
        {
            QFileInfo fi(url.toLocalFile());
            if (fi.isDir())
            {
                QDirIterator it(fi.absoluteFilePath(),
                    QStringList() << "*.mp3" << "*.wav" << "*.m4a",
                    QDir::Files,
                    QDirIterator::Subdirectories);
                while (it.hasNext())
                    files << it.next();
            }
            else
            {
                files << fi.absoluteFilePath();
            }
        }
    }
    if (!files.isEmpty())
    {
        openFiles(files);
        //saveSettings(); // update saved playlist after new files are added
    }
}

// ---------- Slots for buttons & playlist ----------

void Widget::handlePlayButton()
{
    int selectedRow = ui->listWidget->currentRow();
    if (selectedRow >= 0 && selectedRow < m_playlist->mediaCount())
    {
        // Set the selected file as current and start playing it
        m_playlist->setCurrentIndex(selectedRow);
    }
    if (m_player->state() == QMediaPlayer::PlayingState) m_player->stop ();
    m_player->play();
}

void Widget::handlePauseButton()
{
    m_player->pause();
}

void Widget::handleStopButton()
{
    m_player->stop();
}

void Widget::handleItemDoubleClicked()
{
    int idx = ui->listWidget->currentRow();
    if (idx >= 0 && idx < m_playlist->mediaCount())
    {
        m_playlist->setCurrentIndex(idx);
        m_player->play();
    }
}

void Widget::handleMediaStateChanged(QMediaPlayer::State state)
{
    switch (state)
    {
    case QMediaPlayer::PlayingState:
        //ui->playButton->setText("▶ Play");
        ui->playButton->setEnabled(true);
        ui->stopButton->setEnabled(true);
        ui->pauseButton->setEnabled(true);
        break;
    case QMediaPlayer::PausedState:
        // ui->playButton->setText("▶ Resume");
        ui->pauseButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        break;
    case QMediaPlayer::StoppedState:
        //ui->playButton->setText("▶ Play");
        ui->pauseButton->setEnabled(false);
        ui->stopButton->setEnabled(false);
        break;
    }
    // Highlight current item in the playlist
    int currentIndex = m_playlist->currentIndex();
    if (currentIndex >= 0 && currentIndex < ui->listWidget->count())
    {
        ui->listWidget->setCurrentRow(currentIndex);
    }
}

void Widget::handlePlaylistCurrentIndexChanged(int index)
{
    if (index >= 0 && index < ui->listWidget->count())
    {
        ui->listWidget->setCurrentRow(index);
    }
    else
    {
        ui->listWidget->clearSelection();
    }
}

void Widget::handleClearButton()
{
    // Ask for confirmation before clearing the playlist
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Playlist",
            "Are you sure you want to clear the playlist?",
            QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        m_player->stop();
        m_playlist->clear();
        ui->listWidget->clear();
        // Reset last track info
        m_lastTrackIndex = -1;
        m_lastTrackPosition = 0;
        // Save the updated settings
        saveSettings();
    }
    // if No, do nothing
}

void Widget::loadSettings()
{
    QSettings settings(QApplication::organizationName(),
        QApplication::applicationName());
    // Volume
    m_lastVolume = settings.value("volume", 50).toInt();
    m_isMuted = settings.value("muted", false).toBool();
    ui->volumeSlider->setValue(m_lastVolume);
    ui->volumeLabel->setText(QString::number(m_lastVolume) + "%");
    m_player->setVolume(m_isMuted ? 0 : m_lastVolume);
    // Restore playlist
    QStringList playlistFiles = settings.value("playlistFiles").toStringList();
    for (const QString &file : playlistFiles)
    {
        addFileToPlaylist(file);
    }
    // Restore last track index & position
    m_lastTrackIndex = settings.value("lastTrackIndex", 0).toInt();
    m_lastTrackPosition = settings.value("lastTrackPosition", 0).toLongLong();
    if (m_playlist->mediaCount() > 0)
    {
        int idx = qBound(0, m_lastTrackIndex, m_playlist->mediaCount() - 1);
        m_playlist->setCurrentIndex(idx);
        // Select and scroll to the last played track in the UI
        ui->listWidget->setCurrentRow(idx);
        ui->listWidget->scrollToItem(ui->listWidget->item(idx));
        m_player->play();
        if (m_lastTrackPosition > 0)
            m_player->setPosition(m_lastTrackPosition);
    }
    // Restore playback mode
    int modeValue = settings.value("playbackMode", (int)QMediaPlaylist::Sequential).toInt();
    m_playlist->setPlaybackMode(static_cast<QMediaPlaylist::PlaybackMode>(modeValue));
    updateModeButtonIcon();
    // --- Restore window geometry ---
    QByteArray geometry = settings.value("windowGeometry").toByteArray();
    if (!geometry.isEmpty())
        restoreGeometry(geometry);
    // Ensure window is fully visible on any screen
    QRect winRect = frameGeometry();
    bool visibleOnAnyScreen = false;
    for (QScreen * screen : QGuiApplication::screens())
    {
        if (screen->availableGeometry().contains(winRect))
        {
            visibleOnAnyScreen = true;
            break;
        }
    }
    if (!visibleOnAnyScreen)
    {
        // Move to primary screen center
        QRect primaryRect = QGuiApplication::primaryScreen()->availableGeometry();
        int x = primaryRect.x() + (primaryRect.width() - winRect.width()) / 2;
        int y = primaryRect.y() + (primaryRect.height() - winRect.height()) / 2;
        move(x, y);
    }
    // restoreState(settings.value("windowState").toByteArray());
}

void Widget::saveSettings()
{
    QSettings settings(QApplication::organizationName(),
        QApplication::applicationName());
    // Volume
    settings.setValue("volume", m_lastVolume);
    settings.setValue("muted", m_isMuted);
    // Playlist files
    QStringList playlistFiles;
    for (int i = 0; i < m_playlist->mediaCount(); ++i)
    {
        QUrl url = m_playlist->media(i).canonicalUrl();
        if (url.isLocalFile())
            playlistFiles << url.toLocalFile();
    }
    settings.setValue("playlistFiles", playlistFiles);
    // Last track & position
    settings.setValue("lastTrackIndex", m_playlist->currentIndex());
    settings.setValue("lastTrackPosition", m_player->position());
    // Save playback mode
    settings.setValue("playbackMode", static_cast<int>(m_playlist->playbackMode()));
    // Save window position and size
    settings.setValue("windowGeometry", saveGeometry());
}

// ------------------- Slots -------------------

void Widget::handleVolumeChanged(int value)
{
    m_lastVolume = value;
    ui->volumeLabel->setText(QString::number(value) + "%");  // NEW
    if (!m_isMuted)
    {
        m_player->setVolume(value);
    }
    saveSettings();
}

void Widget::handleMuteButton()
{
    if (m_isMuted)
    {
        // Unmute
        m_isMuted = false;
        m_player->setVolume(m_lastVolume);
        ui->volumeLabel->setText(QString::number(m_lastVolume) + "%");
    }
    else
    {
        // Mute
        m_isMuted = true;
        m_player->setVolume(0);
        ui->volumeLabel->setText("0%");
    }
    updateMuteButtonIcon();
    saveSettings();
}

void Widget::updateMuteButtonIcon()
{
    if (m_isMuted)
    {
        ui->muteButton->setIcon(QIcon(":/img/img/icons8-sound-48.png"));
    }
    else
    {
        ui->muteButton->setIcon(QIcon(":/img/img/icons8-mute-48.png"));
    }
}

void Widget::handleDurationChanged(qint64 duration)
{
    ui->positionSlider->setMaximum(static_cast<int>(duration));
    QTime total(0, 0);
    total = total.addMSecs(static_cast<int>(duration));
    ui->timeLabel->setText("00:00 / " + total.toString("mm:ss"));
}

void Widget::handlePositionChanged(qint64 position)
{
    if (!ui->positionSlider->isSliderDown())
    {
        ui->positionSlider->setValue(static_cast<int>(position));
    }
    QTime current(0, 0);
    current = current.addMSecs(static_cast<int>(position));
    QTime total(0, 0);
    total = total.addMSecs(ui->positionSlider->maximum());
    ui->timeLabel->setText(current.toString("mm:ss") + " / " + total.toString("mm:ss"));
}

void Widget::handleSliderMoved(int position)
{
    m_player->setPosition(position);
}

void Widget::handleMediaError(QMediaPlayer::Error error)
{
    QString errorText;
    switch (error)
    {
    case QMediaPlayer::ResourceError:
        errorText = "Cannot access media file.";
        break;
    case QMediaPlayer::FormatError:
        errorText = "Unsupported media format or missing codec.";
        break;
    case QMediaPlayer::NetworkError:
        errorText = "Network error during playback.";
        break;
    case QMediaPlayer::AccessDeniedError:
        errorText = "Access denied to media file.";
        break;
    default:
        errorText = "Unknown playback error.";
        break;
    }
    QMessageBox::warning(this, tr("Playback Error"), errorText);
}

void Widget::updateLastTrackPosition(qint64 position)
{
    m_lastTrackPosition = position;
}

void Widget::on_nextButton_clicked()
{
    if (m_playlist->mediaCount() == 0)
        return;
    QMediaPlaylist::PlaybackMode currentMode = m_playlist->playbackMode();
    if (currentMode == QMediaPlaylist::Random)
    {
        int count = m_playlist->mediaCount();
        if (count > 1)
        {
            int currentIndex = m_playlist->currentIndex();
            if (currentIndex >= 0)
                m_shuffleHistory.push(currentIndex);  // remember current before moving
            int newIndex;
            do
            {
                newIndex = QRandomGenerator::global()->bounded(count);
            }
            while (newIndex == currentIndex);   // avoid picking same
            m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
            m_playlist->setCurrentIndex(newIndex);
            m_playlist->setPlaybackMode(currentMode);
        }
    }
    else if (currentMode == QMediaPlaylist::CurrentItemInLoop)
    {
        int currentIndex = m_playlist->currentIndex();
        int nextIndex = (currentIndex + 1) % m_playlist->mediaCount();
        m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        m_playlist->setCurrentIndex(nextIndex);
        m_playlist->setPlaybackMode(currentMode);
    }
    else
    {
        m_playlist->next();
    }
    m_player->play();
    saveSettings();
}

void Widget::on_prevButton_clicked()
{
    if (m_playlist->mediaCount() == 0)
        return;
    QMediaPlaylist::PlaybackMode currentMode = m_playlist->playbackMode();
    if (currentMode == QMediaPlaylist::Random && !m_shuffleHistory.isEmpty())
    {
        int lastIndex = m_shuffleHistory.pop(); // go back to last track
        m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        m_playlist->setCurrentIndex(lastIndex);
        m_playlist->setPlaybackMode(currentMode);
    }
    else if (currentMode == QMediaPlaylist::CurrentItemInLoop)
    {
        int currentIndex = m_playlist->currentIndex();
        int prevIndex = (currentIndex - 1 + m_playlist->mediaCount()) % m_playlist->mediaCount();
        m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        m_playlist->setCurrentIndex(prevIndex);
        m_playlist->setPlaybackMode(currentMode);
    }
    else
    {
        m_playlist->previous();
    }
    m_player->play();
    saveSettings();
}

void Widget::on_configureButton_clicked()
{
}

void Widget::handleModeButton()
{
    // Cycle through modes: Sequential -> Loop -> Random -> back to Sequential
    QMediaPlaylist::PlaybackMode mode = m_playlist->playbackMode();
    switch (mode)
    {
    case QMediaPlaylist::Sequential:
        m_playlist->setPlaybackMode(QMediaPlaylist::Loop);  // Loop All
        break;
    case QMediaPlaylist::Loop:
        m_playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);  // Loop One
        break;
    case QMediaPlaylist::CurrentItemInLoop:
        m_playlist->setPlaybackMode(QMediaPlaylist::Random);  // Shuffle
        break;
    case QMediaPlaylist::Random:
        m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        break;
    default:
        m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        break;
    }
    updateModeButtonIcon();
    saveSettings();  // Save current mode
}

void Widget::updateModeButtonIcon()
{
    switch (m_playlist->playbackMode())
    {
    case QMediaPlaylist::Sequential:
        ui->modeButton->setIcon(QIcon(":/img/img/icons8-right-48.png"));
        ui->modeButton->setToolTip("Sequential mode");
        break;
    case QMediaPlaylist::Loop:
        ui->modeButton->setIcon(QIcon(":/img/img/icons8-loop-48.png"));
        ui->modeButton->setToolTip("Loop all tracks");
        break;
    case QMediaPlaylist::CurrentItemInLoop:
        ui->modeButton->setIcon(QIcon(":/img/img/icons8-repeat-one-48.png"));
        ui->modeButton->setToolTip("Repeat current track");
        break;
    case QMediaPlaylist::Random:
        ui->modeButton->setIcon(QIcon(":/img/img/icons8-random-48.png"));
        ui->modeButton->setToolTip("Shuffle mode");
        break;
    default:
        break;
    }
}
