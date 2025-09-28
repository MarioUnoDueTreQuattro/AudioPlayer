#include "widget.h"
#include "settings.h"
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
#include <QFileDialog>
#include <QMenu>
#include <QStandardPaths>
#include <QThread>
#include <QBuffer>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QStyleFactory>
#include <QPalette>
#include <QDirIterator>
#include <QToolTip>
#include <QDesktopServices>
#include <fileref.h>
#include <tag.h>

//#include <taglib/fileref.h>
//#include <taglib/tag.h>
//#include <taglib/toolkit//tpropertymap.h>

Widget::Widget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Widget),
      m_player(new QMediaPlayer(this)),
      m_playlist(new QMediaPlaylist(this)),
      m_lastVolume(50),
      m_isMuted(false),
      m_bInfoWindowHasBeenMinimized(false),
      m_bInfoWindowHasBeenClosed(false)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    m_infoWidget = new InfoWidget();
    // m_hotkey = new QHotkey(QKeySequence("Ctrl+Shift+A"), true, this);
    // if (!m_hotkey->isRegistered())
    // {
    // qDebug() << "Hotkey not registered!";
    // }
    // else
    // qDebug() << "Hotkey registered!";
    // QObject::connect(m_hotkey, &QHotkey::activated,
    // [&]()
    // {
    // QMessageBox::information(nullptr, "Hotkey Pressed",
    // "You pressed Hotkey!");
    // });
    setWindowTitle ("AudioPlayer");
    setAcceptDrops(true);
    m_bSystemVolumeSlider = false;
    m_systemVolumeController = new SystemVolumeController(this);
    m_player->setPlaylist(m_playlist);
    loadSettings();  // Load volume/mute state before connecting slider
    // Apply loaded volume
    m_player->setVolume(m_lastVolume);
    ui->volumeSlider->setValue(m_lastVolume);
    ui->listWidget -> setAlternatingRowColors(true);
    setSignalsConnections();
    setKeyboardShortcuts();
}

void Widget::setKeyboardShortcuts()
{
    ui->playButton->setShortcut(QKeySequence(Qt::Key_Space));
    ui->stopButton->setShortcut(QKeySequence(Qt::Key_S));
    ui->pauseButton->setShortcut(QKeySequence(Qt::Key_P));
    ui->muteButton->setShortcut(QKeySequence(Qt::Key_M));
    ui->prevButton->setShortcut(QKeySequence(Qt::Key_Left));
    ui->nextButton->setShortcut(QKeySequence(Qt::Key_Right));
    // Volume up with '+'
    QAction *volumeUpAction = new QAction(tr("Volume Up"), this);
    volumeUpAction->setShortcut(QKeySequence(Qt::Key_Plus));
    connect(volumeUpAction, SIGNAL(triggered()), this, SLOT(handleVolumeUp()));
    addAction(volumeUpAction);
    // Volume down with '-'
    QAction *volumeDownAction = new QAction(tr("Volume Down"), this);
    volumeDownAction->setShortcut(QKeySequence(Qt::Key_Minus));
    connect(volumeDownAction, SIGNAL(triggered()), this, SLOT(handleVolumeDown()));
    addAction(volumeDownAction);
    QAction *volumeSwitchAction = new QAction(tr("Switch volume control"), this);
    volumeSwitchAction->setShortcut(QKeySequence(Qt::Key_V));
    connect(volumeSwitchAction, SIGNAL(triggered()), this, SLOT(switchVolume()));
    addAction(volumeSwitchAction);
}

void Widget::handleVolumeUp()
{
    int iDiff = 5;
    if (m_bSystemVolumeSlider == false)
    {
        int vol = qBound(0, m_lastVolume + iDiff, 100);
        handleVolumeChanged (vol);
        ui->volumeSlider->setValue (vol);
        //ui->volumeLabel->setText(QString::number(vol) + "%");
    }
    else
    {
        int iCurVol = ui->volumeSlider->value ();
        iCurVol = qBound(0, iCurVol + iDiff, 100);
        float fVol = float (float(iCurVol) / 100.0f);
        m_systemVolumeController->setVolume (fVol);
        ui->volumeSlider->setValue (iCurVol);
        ui->volumeLabel->setText(QString::number(iCurVol) + "%");
    }
}

void Widget::handleVolumeDown()
{
    int iDiff = -5;
    if (m_bSystemVolumeSlider == false)
    {
        int vol = qBound(0, m_lastVolume + iDiff, 100);
        handleVolumeChanged (vol);
        ui->volumeSlider->setValue (vol);
        //ui->volumeLabel->setText(QString::number(vol) + "%");
    }
    else
    {
        int iCurVol = ui->volumeSlider->value ();
        iCurVol = qBound(0, iCurVol + iDiff, 100);
        float fVol = float (float(iCurVol) / 100.0f);
        m_systemVolumeController->setVolume (fVol);
        ui->volumeSlider->setValue (iCurVol);
        ui->volumeLabel->setText(QString::number(iCurVol) + "%");
    }
}

void Widget::setSignalsConnections()
{
    connect(m_infoWidget, SIGNAL(windowClosed()), this, SLOT(infoWindowClosed()));
    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(handlePlayButton()));
    connect(ui->pauseButton, SIGNAL(clicked()), this, SLOT(handlePauseButton()));
    connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(handleStopButton()));
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(handleVolumeChanged(int)));
    connect(ui->muteButton, SIGNAL(clicked()), this, SLOT(handleMuteButton())); // NEW
    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(handleItemDoubleClicked()));
    connect(m_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(handleMediaStateChanged(QMediaPlayer::State)));
    connect(m_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(handlePlaylistCurrentIndexChanged(int)));
    connect(m_player, SIGNAL(positionChanged(qint64)), this, SLOT(handlePositionChanged(qint64)));
    connect(m_player, SIGNAL(durationChanged(qint64)), this, SLOT(handleDurationChanged(qint64)));
    connect(ui->positionSlider, SIGNAL(sliderMoved(int)), this, SLOT(handleSliderMoved(int)));
    connect(m_player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(handleMediaError(QMediaPlayer::Error)));
    connect(m_player, SIGNAL(positionChanged(qint64)), this, SLOT(updateLastTrackPosition(qint64)));
    connect(ui->modeButton, SIGNAL(clicked()), this, SLOT(handleModeButton()));
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, &Widget::showPlaylistContextMenu);
    connect(ui->modeButton, &QListWidget::customContextMenuRequested, this, &Widget::showModeButtonContextMenu);
    connect(ui->volumeSlider, &QListWidget::customContextMenuRequested, this, &Widget::showVolumeSliderContextMenu);
    connect(m_systemVolumeController, SIGNAL(volumeChanged(float)),
        this, SLOT(onSystemVolumeChanged(float)));
    connect(m_systemVolumeController, SIGNAL(muteStateChanged(bool)),
        this, SLOT(onSystemMuteChanged(bool)));
    // connect(m_systemVolumeController, SIGNAL(defaultDeviceChanged()),
    // this, SLOT(onDefaultDeviceChanged()));
    connect(m_systemVolumeController, SIGNAL(defaultDeviceChanged(QString, QString)),
        this, SLOT(onDeviceChanged(QString, QString)));
}

Widget::~Widget()
{
    // saveSettings();
    delete ui;
    if (m_infoWidget != nullptr) delete m_infoWidget;
}

void Widget::changeEvent(QEvent *event)
{
    //qDebug() << "Window changeEvent";
    QWidget::changeEvent(event);
    if (event->type() == QEvent::ActivationChange)
    {
        if (isActiveWindow())
        {
            //qDebug() << "Window gained focus";
            if (m_infoWidget != nullptr && m_bShowInfo)
            {
                if (m_bInfoWindowHasBeenMinimized)
                {
                    m_bInfoWindowHasBeenMinimized = false;
                    m_bInfoWindowHasBeenClosed=false;
                    m_infoWidget->show ();
                }
                //m_infoWidget->showNormal();
                //m_infoWidget->show ();
                m_infoWidget->raise();
                //m_infoWidget->activateWindow();
            }
        }
    }
    else if (event->type() == QEvent::WindowStateChange)
    {
        if (isMinimized())
        {
            if (m_infoWidget != nullptr && m_bShowInfo)
            {
                // qDebug() << "MainWindow was minimized";
                if (m_infoWidget->isVisible ())
                {
                    m_infoWidget->hide ();
                    m_bInfoWindowHasBeenMinimized = true;
                }
            }
        }
        else
        {
            // qDebug() << "MainWindow state changed (restored/maximized)";
        }
    }
}

void Widget::infoWindowClosed()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_bInfoWindowHasBeenClosed=true;
}

void Widget::closeEvent(QCloseEvent *event)
{
    //qDebug() << "closeEvent";
    //this->hide ();
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    // Save window geometry & state
    settings.setValue("windowGeometry", saveGeometry());
    // settings.setValue("windowState", saveState());
    // Save volume & mute
    settings.setValue("volume", m_lastVolume);
    //settings.setValue("muted", m_isMuted);
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
    settings.sync ();
    // disconnect(m_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(handleMediaStateChanged(QMediaPlayer::State)));
    // disconnect(m_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(handlePlaylistCurrentIndexChanged(int)));
    // disconnect(m_player, SIGNAL(positionChanged(qint64)), this, SLOT(handlePositionChanged(qint64)));
    // disconnect(m_player, SIGNAL(durationChanged(qint64)), this, SLOT(handleDurationChanged(qint64)));
    //    // disconnect(ui->positionSlider, SIGNAL(sliderMoved(int)),    this, SLOT(handleSliderMoved(int)));
    // disconnect(m_player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(handleMediaError(QMediaPlayer::Error)));
    // disconnect(m_player, SIGNAL(positionChanged(qint64)), this, SLOT(updateLastTrackPosition(qint64)));
    if (m_player)
    {
        // disconnect(m_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(handleMediaStateChanged(QMediaPlayer::State)));
        // disconnect(m_player, SIGNAL(positionChanged(qint64)), this, SLOT(handlePositionChanged(qint64)));
        // disconnect(m_player, SIGNAL(durationChanged(qint64)), this, SLOT(handleDurationChanged(qint64)));
        // disconnect(m_player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(handleMediaError(QMediaPlayer::Error)));
        // disconnect(m_player, SIGNAL(positionChanged(qint64)), this, SLOT(updateLastTrackPosition(qint64)));
        // Disconnect signals
        disconnect(m_player, nullptr, this, nullptr);
        disconnect(m_playlist, nullptr, this, nullptr);
        //m_playlist->clear();
        m_player->stop();
        playSilence(100);
        // QAudioFormat af;
        // QAudioOutput m_audioOutput = new QAudioOutput(af ,this);
        // m_player->setAudioOutput(m_audioOutput);
        // Clear playlist and delete objects
        if (m_playlist)
        {
            //m_playlist->clear();
            delete m_playlist;
            m_playlist = nullptr;
        }
        delete m_player;
        m_player = nullptr;
    }
    // Optional: give a short time to release audio device
    // QCoreApplication::processEvents();
    // QThread::msleep(500);
    QWidget::closeEvent(event); // call base implementation
}

void Widget::playSilence(int ms)
{
    // Configure PCM format for silence
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);
    // Prepare a buffer of silence
    QByteArray silence(ms * format.sampleRate() * format.channelCount() * (format.sampleSize() / 8), 0);
    QBuffer *buffer = new QBuffer;
    buffer->setData(silence);
    buffer->open(QIODevice::ReadOnly);
    // Create an audio output instance
    QAudioOutput *audioOut = new QAudioOutput(format, this);
    // Store buffer pointer in audioOut properties for later cleanup
    audioOut->setProperty("silenceBuffer", QVariant::fromValue<void*>(buffer));
    // Connect stateChanged signal for this audioOut instance
    connect(audioOut, SIGNAL(stateChanged(QAudio::State)),
        this, SLOT(handleSilenceFinished(QAudio::State)));
    // Start playback
    audioOut->start(buffer);
}

void Widget::handleSilenceFinished(QAudio::State state)
{
    QAudioOutput *audioOut = qobject_cast<QAudioOutput *>(sender());
    if (!audioOut)
        return;
    if (state == QAudio::IdleState || state == QAudio::StoppedState)
    {
        // Retrieve and delete the buffer associated with this audioOut
        QBuffer *buffer = static_cast<QBuffer *>(audioOut->property("silenceBuffer").value<void*>());
        if (buffer)
        {
            buffer->close();
            buffer->deleteLater();
        }
        audioOut->stop();
        audioOut->deleteLater();
    }
}

void Widget::resizeEvent(QResizeEvent *event)
{
    // QSize newSize = event->size();
    // QSize oldSize = event->oldSize();
    // qDebug() << "Widget resized from" << oldSize << "to" << newSize;
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    // Save window geometry & state
    settings.setValue("windowGeometry", saveGeometry());
    // settings.setValue("windowState", saveState());
    // Call base implementation (optional if QWidget)
    QWidget::resizeEvent(event);
}

void Widget::focusInEvent(QFocusEvent *event)
{
    qDebug() << "Il widget '" << objectName() << "' HA OTTENUTO IL FOCUS!";
    if (m_infoWidget != nullptr)
    {
        //m_infoWidget->showNormal();
        m_infoWidget->raise();
        //m_infoWidget->activateWindow();
    }
    QWidget::focusInEvent(event);
}

void Widget::openGoogleSearch(const QString &text)
{
    // Percent-encode the query text
    const QByteArray encodedQuery = QUrl::toPercentEncoding(text);
    // Build google search URL (uses 'q' parameter)
    const QUrl searchUrl(QStringLiteral("https://www.google.com/search?q=%1").arg(QString::fromUtf8(encodedQuery)));
    // Open default browser
    bool success = QDesktopServices::openUrl(searchUrl);
    if (!success)
    {
        qWarning() << "Failed to open URL:" << searchUrl.toString();
    }
}

void Widget::switchVolume()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!m_bSystemVolumeSlider)
    {
        m_bSystemVolumeSlider = true;
        int iVol = int (m_systemVolumeController->volume () * 100.01f);
        qDebug() << "Current master volume:" << iVol;
        ui->volumeSlider->setValue (iVol);
        ui->volumeLabel->setText(QString::number(iVol) + "%");
    }
    else
    {
        m_bSystemVolumeSlider = false;
        ui->volumeSlider->setValue (m_lastVolume);
    }
    updateMuteButtonIcon ();
}

QString Widget::currentTrackName()
{
    QString sCurrent = "";
    if (!m_playlist)
        return sCurrent;
    int index = m_playlist->currentIndex();
    if (index < 0)
    {
        qDebug() << "No current item in playlist";
        return "No current item in playlist";
    }
    QMediaContent mediaContent = m_playlist->media(index);
    QUrl mediaUrl = mediaContent.canonicalUrl();
    if (mediaUrl.isLocalFile())
    {
        QFileInfo fileInfo(mediaUrl.toLocalFile());
        sCurrent = fileInfo.fileName();
        qDebug() << "Now playing:" << sCurrent; // just file name
    }
    else
    {
        sCurrent = mediaUrl.toString();
        qDebug() << "Now playing URL:" << sCurrent;
    }
    return sCurrent;
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
        if (m_bAutoplay)
        {
            handlePlay ();
        }
        //this->setWindowTitle ("AudioPlayer - " + ui->listWidget->currentItem ()->text ());
        // Restore last position only if not starting a new file
        if (filePaths.isEmpty() && m_lastTrackPosition > 0)
        {
            m_player->setPosition(m_lastTrackPosition);
        }
    }
    if (m_playlist->mediaCount() <= 1 && m_playlist->playbackMode() == QMediaPlaylist::Random)
    {
        m_shuffleHistory.clear(); // no history needed
        m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        updateModeButtonIcon();
    }
    saveSettings();
}

void Widget::addFileToPlaylist(const QString &filePath)
{
    QFileInfo fi(filePath);
    if (!fi.exists())
        return;
    QString ext = fi.suffix().toLower();
    if (ext != "wav" && ext != "mp3" && ext != "m4a" && ext != "aac" && ext != "opus" && ext != "flac")
        return;
    // Avoid duplicate entries
    for (int i = 0; i < m_playlist->mediaCount(); ++i)
    {
        if (m_playlist->media(i).canonicalUrl() == QUrl::fromLocalFile(fi.absoluteFilePath()))
            return;
    }
    m_playlist->addMedia(QUrl::fromLocalFile(fi.absoluteFilePath()));
    ui->listWidget->addItem(fi.fileName());
    saveSettings();
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
                    QStringList() << "*.mp3" << "*.wav" << "*.m4a" << "*.aac" << "*.opus" << "*.flac",
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
    handlePlay ();
}

void Widget::handlePauseButton()
{
    m_player->pause();
}

void Widget::handleStopButton()
{
    m_player->stop();
    int iListWidgetCount = ui->listWidget->count();
    for (int iIdx = 0; iIdx < iListWidgetCount; iIdx++)
    {
        ui->listWidget->item (iIdx)->setIcon (QIcon());
    }
}

void Widget::handlePlay()
{
    m_player->play();
    QString sPlaying = currentTrackName ();
    m_playedList.append (sPlaying);
    this->setWindowTitle ("AudioPlayer - " + sPlaying);
    ui->listWidget->currentItem ()->setTextColor (m_playedTextColor);
    ui->listWidget->currentItem ()->setIcon (QIcon(":/img/img/icons8-play-48.png"));
    QUrl mediaUrl;
    // If you are using a playlist:
    if (m_player->playlist())
        mediaUrl = m_player->playlist()->currentMedia().canonicalUrl();
    else
        mediaUrl = m_player->media().canonicalUrl();
    QString localFile = mediaUrl.toLocalFile();
    qDebug() << "Current media file:" << localFile;
    if (!localFile.isEmpty())
    {
        TagLib::FileRef f(TagLib::FileName(localFile.toUtf8().constData()));
        if (!f.isNull() && f.tag())
        {
            TagLib::Tag *tag = f.tag();
            auto safeString = [](const TagLib::String &str) -> QString
            {
                return QString::fromUtf8(str.toCString(true));
            };
            QString title = safeString(tag->title());
            QString artist = safeString(tag->artist());
            QString album = safeString(tag->album());
            QString genre = safeString(tag->genre());
            int year = tag->year();
            int trackNum = tag->track();
//            qDebug() << "Title:" << (title.isEmpty() ? "[Unknown]" : title);
//            qDebug() << "Artist:" << (artist.isEmpty() ? "[Unknown]" : artist);
//            qDebug() << "Album:" << (album.isEmpty() ? "[Unknown]" : album);
//            qDebug() << "Year:" << (year == 0 ? "[Unknown]" : QString::number(year));
//            qDebug() << "Track:" << (trackNum == 0 ? "[Unknown]" : QString::number(trackNum));
//            qDebug() << "Genre:" << (genre.isEmpty() ? "[Unknown]" : genre);
            // Optional: Format string for UI
            // QString info = QString("Artist: %1\nTitle: %2\nAlbum: %3\nYear: %4").arg(artist.isEmpty() ? "[Unknown Artist]" : artist,
            // title.isEmpty() ? "[Unknown Title]" : title, album.isEmpty() ? "[Unknown Album]" : album, year == 0 ? "[Unknown Year]" : QString::number(year));
            // QString info = QString("<b>Artist:</b> %1<br><b>Title:</b> %2<br>Album: %3\nYear: %4").arg(artist.isEmpty() ? "[Unknown Artist]" : artist,
            // title.isEmpty() ? "[Unknown Title]" : title, album.isEmpty() ? "[Unknown Album]" : album, year == 0 ? "[Unknown Year]" : QString::number(year));
            QString info;
            info.append ("Artist: ");
            info.append (artist.isEmpty() ? "[Unknown artist]" : artist);
            info.append ("\n");
            info.append ("Title: ");
            info.append ( title.isEmpty() ? "[Unknown title]" : title);
            info.append ("\n");
            info.append ("Album: ");
            info.append ( album.isEmpty() ? "[Unknown album]" : album);
            info.append ("\n");
            info.append ("Track: ");
            info.append (trackNum == 0 ? "[Unknown track number]" : QString::number(trackNum));
            info.append ("\n");
            info.append ("Year: ");
            info.append ( year == 0 ? "[Unknown year]" : QString::number(year));
            if (tag->genre ().isEmpty () == false)
            {
                info.append ("\n");
                info.append ("Genre: ");
                info.append (safeString(tag->genre ()));
            }
            if (tag->comment ().isEmpty () == false)
            {
                info.append ("\n");
                info.append ("Comment: ");
                info.append (safeString(tag->comment ()));
            }
            if (m_infoWidget != nullptr) m_infoWidget->setInfo (info);
            if (m_bShowInfo == false)
            {
                QPoint globalPos = ui->listWidget->mapToGlobal(QPoint(ui->listWidget->width() / 2, ui->listWidget->height() / 2));
                QToolTip::showText(globalPos, info, ui->listWidget);
            }
            else
            {
                if (m_infoWidget == nullptr) m_infoWidget = new InfoWidget();
                m_infoWidget->raise ();
                // m_infoWidget->setInfo (info);
                m_infoWidget->show ();
            }
        }
        else
        {
            qDebug() << "Failed to read metadata!";
        }
    }
    else
    {
        qDebug() << "File path is empty!";
    }
}

void Widget::handleItemDoubleClicked()
{
    int idx = ui->listWidget->currentRow();
    if (idx >= 0 && idx < m_playlist->mediaCount())
    {
        m_playlist->setCurrentIndex(idx);
        if (m_player->state() == QMediaPlayer::PlayingState) m_player->stop ();
        handlePlay();
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
    int iListWidgetCount = ui->listWidget->count();
    for (int iIdx = 0; iIdx < iListWidgetCount; iIdx++)
    {
        ui->listWidget->item (iIdx)->setIcon (QIcon());
    }
    if (index >= 0 && index < iListWidgetCount)
    {
        ui->listWidget->setCurrentRow(index);
        if (m_player->state() == QMediaPlayer::PlayingState || m_bAutoplay)
        {
            handlePlay ();
        }
    }
    else
    {
        // ui->listWidget->clearSelection();
    }
}

void Widget::handleClearPlaylist()
{
    clearPlaylist(false); // normal behavior with confirmation
}

void Widget::clearPlaylist(bool silent /* = false */)
{
    if (!silent)
    {
        // Ask for confirmation before clearing the playlist
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Clear Playlist",
                "Are you sure you want to clear the playlist?",
                QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes)
        {
            return; // user cancelled
        }
    }
    m_player->stop();
    m_playlist->clear();
    ui->listWidget->clear();
    // Reset last track info
    m_lastTrackIndex = -1;
    m_lastTrackPosition = 0;
    saveSettings(); // Save the updated settings
}

void Widget::loadSettings()
{
    QSettings settings(QApplication::organizationName(),
        QApplication::applicationName());
    m_sTheme = settings.value("Theme").toString();
    m_sPalette = settings.value("ThemePalette", "Light").toString();
    setTheme ();
    m_bAutoplay = settings.value("AutoPlay", true).toBool ();
    m_bShowInfo = settings.value("ShowInfo", true).toBool ();
    QString colorName = settings.value("PlayedTextColor", "#000080").toString();
    m_playedTextColor = QColor(colorName);
    // --- Volume ---
    m_lastVolume = settings.value("volume", 50).toInt();
    m_isMuted = false; // force unmuted
    ui->volumeSlider->setValue(m_lastVolume);
    ui->volumeLabel->setText(QString::number(m_lastVolume) + "%");
    //m_player->setVolume(m_isMuted ? 0 : m_lastVolume);
    // --- Last track & position ---
    m_lastTrackIndex = settings.value("lastTrackIndex", 0).toInt();
    m_lastTrackPosition = settings.value("lastTrackPosition", 0).toLongLong();
    // --- Restore playlist ---
    QString lastPlaylistPath = settings.value("lastPlaylistPath").toString();
    QStringList playlistFiles = settings.value("playlistFiles").toStringList();
    if (!playlistFiles.isEmpty())
    {
        // Restore from saved playlist files (fallback)
        m_playlist->clear();
        ui->listWidget->clear();
        for (const QString &file : playlistFiles)
            addFileToPlaylist(file);
        if (m_playlist->mediaCount() > 0)
        {
            int idx = qBound(0, m_lastTrackIndex, m_playlist->mediaCount() - 1);
            m_playlist->setCurrentIndex(idx);
            ui->listWidget->setCurrentRow(idx);
            ui->listWidget->scrollToItem(ui->listWidget->item(idx), QAbstractItemView::PositionAtCenter);
            if (m_lastTrackPosition > 0)
                m_player->setPosition(m_lastTrackPosition);
        }
    }
    // --- Playback mode ---
    int modeValue = settings.value("playbackMode", (int)QMediaPlaylist::Sequential).toInt();
    m_playlist->setPlaybackMode(static_cast<QMediaPlaylist::PlaybackMode>(modeValue));
    updateModeButtonIcon();
    // --- Window geometry ---
    QByteArray geometry = settings.value("windowGeometry").toByteArray();
    if (!geometry.isEmpty())
        restoreGeometry(geometry);
    // Ensure window visible
    QRect winRect = frameGeometry();
    bool visibleOnAnyScreen = false;
    for (QScreen *screen : QGuiApplication::screens())
    {
        if (screen->availableGeometry().contains(winRect))
        {
            visibleOnAnyScreen = true;
            break;
        }
    }
    if (!visibleOnAnyScreen)
    {
        QRect primaryRect = QGuiApplication::primaryScreen()->availableGeometry();
        move(primaryRect.center() - winRect.center());
    }
    // Start playback if playlist not empty
    if (m_playlist->mediaCount() > 0)
        if (m_bAutoplay)
        {
            handlePlay ();
        }
    if (m_playlist->mediaCount() <= 1 && m_playlist->playbackMode() == QMediaPlaylist::Random)
    {
        m_shuffleHistory.clear(); // no history needed
        m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        updateModeButtonIcon();
    }
}

void Widget::saveSettings()
{
    QSettings settings(QApplication::organizationName(),
        QApplication::applicationName());
    // --- Volume ---
    //settings.setValue("volume", m_lastVolume);
    //settings.setValue("muted", m_isMuted); // optional
    // --- Playlist files fallback ---
    QStringList playlistFiles;
    for (int i = 0; i < m_playlist->mediaCount(); ++i)
    {
        QUrl url = m_playlist->media(i).canonicalUrl();
        if (url.isLocalFile())
            playlistFiles << url.toLocalFile();
    }
    settings.setValue("playlistFiles", playlistFiles);
    // --- Last playlist path (optional, for user-loaded playlists) ---
    settings.setValue("lastPlaylistPath", m_lastPlaylistPath);
    // --- Last track & position ---
    settings.setValue("lastTrackIndex", m_playlist->currentIndex());
    settings.setValue("lastTrackPosition", m_player->position());
    // --- Playback mode ---
    // settings.setValue("playbackMode", static_cast<int>(m_playlist->playbackMode()));
    // --- Window geometry ---
    //settings.setValue("windowGeometry", saveGeometry());
    settings.sync();
}

// ------------------- Slots -------------------

void Widget::handleVolumeChanged(int value)
{
    if (m_bSystemVolumeSlider == false)
    {
        m_lastVolume = value;
        ui->volumeLabel->setText(QString::number(value) + "%");  // NEW
        if (!m_isMuted)
        {
            m_player->setVolume(value);
        }
        QSettings settings;
        settings.setValue("volume", m_lastVolume);
    }
    else
    {
        float fVol = float (float(value) / 100.0f);
        int iVol = int (fVol * 100.01f);
        // qDebug() << "Current master volume f:" << fVol;
        // qDebug() << "Current master volume i:" << iVol;
        m_systemVolumeController->setVolume (fVol);
        ui->volumeSlider->setValue (iVol);
        ui->volumeLabel->setText(QString::number(iVol) + "%");
    }
}

void Widget::handleMuteButton()
{
    if (m_bSystemVolumeSlider == false)
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
    }
    else
    {
        bool bMuted = m_systemVolumeController->isMuted ();
        if (bMuted)
        {
            // Unmute
            m_systemVolumeController->mute (false);
            int iVol = int (m_systemVolumeController->volume () * 100.01f);
            qDebug() << "Current master volume:" << iVol;
            ui->volumeSlider->setValue (iVol);
            ui->volumeLabel->setText(QString::number(iVol) + "%");
        }
        else
        {
            // Mute
            m_systemVolumeController->mute (true);
            ui->volumeLabel->setText("0%");
        }
        // float fVol = float (float(value) / 100.0f);
        // int iVol = int (fVol * 100.01f);
        //       // qDebug() << "Current master volume f:" << fVol;
        //       // qDebug() << "Current master volume i:" << iVol;
        // m_systemVolumeController->setVolume (fVol);
        // ui->volumeSlider->setValue (iVol);
        // ui->volumeLabel->setText(QString::number(iVol) + "%");
    }
    updateMuteButtonIcon();
    //saveSettings(); // optional: still save the mute state if you want
}

void Widget::updateMuteButtonIcon()
{
    if (m_bSystemVolumeSlider == false)
    {
        if (m_isMuted)
        {
            ui->muteButton->setChecked (true);
            ui->muteButton->setIcon(QIcon(":/img/img/icons8-mute-48.png"));
        }
        else
        {
            ui->muteButton->setChecked (false);
            ui->muteButton->setIcon(QIcon(":/img/img/icons8-sound-48.png"));
        }
    }
    else
    {
        bool bMuted = m_systemVolumeController->isMuted ();
        if (bMuted)
        {
            ui->muteButton->setChecked (true);
            ui->muteButton->setIcon(QIcon(":/img/img/icons8-mute-48.png"));
        }
        else
        {
            ui->muteButton->setChecked (false);
            ui->muteButton->setIcon(QIcon(":/img/img/icons8-sound-48.png"));
        }
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
    int currentIndex = m_playlist->currentIndex();
    if (currentMode == QMediaPlaylist::Random)
    {
        int count = m_playlist->mediaCount();
        if (count > 1)
        {
            // Remember current track in history
            if (currentIndex >= 0)
                m_shuffleHistory.push(currentIndex);
            int newIndex;
            do
            {
                newIndex = QRandomGenerator::global()->bounded(count);
            }
            while (newIndex == currentIndex); // avoid repeating immediately
            m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
            m_playlist->setCurrentIndex(newIndex);
            m_playlist->setPlaybackMode(currentMode);
        }
    }
    else if (currentMode == QMediaPlaylist::CurrentItemInLoop)
    {
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
    int currentIndex = m_playlist->currentIndex();
    if (currentMode == QMediaPlaylist::Random)
    {
        if (!m_shuffleHistory.isEmpty())
        {
            int lastIndex = m_shuffleHistory.pop(); // go back to last played track
            m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
            m_playlist->setCurrentIndex(lastIndex);
            m_playlist->setPlaybackMode(currentMode);
        }
    }
    else if (currentMode == QMediaPlaylist::CurrentItemInLoop)
    {
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
    Settings settingsDialog;       //=new configureDialog(this);
    connect(&settingsDialog, SIGNAL(accepted()), this, SLOT(settingsDialogAccepted()));
    connect(&settingsDialog, SIGNAL(applyClicked()), this, SLOT(settingsDialogAccepted()));
    // cd.setParent (this);
    settingsDialog.setWindowTitle("Configure");
    settingsDialog.exec();
}

void Widget::settingsDialogAccepted()
{
    //qDebug() << __PRETTY_FUNCTION__ ;
    QSettings settings;
    QString colorName = settings.value("PlayedTextColor", "#000080").toString();
    m_playedTextColor = QColor(colorName);
    int iTotalItems = ui->listWidget->count();
    int iCount = m_playedList.count ();
    for (int iIdx = 0; iIdx < iCount; iIdx++)
    {
        const QString &playedName = m_playedList.at(iIdx);
        for (int iPlaylistItem = 0; iPlaylistItem < iTotalItems; iPlaylistItem++)
        {
            QListWidgetItem *item = ui->listWidget->item(iPlaylistItem);
            if (item->text () == playedName)
            {
                //qDebug() << "found: " << playedName;
                item->setForeground(QBrush(m_playedTextColor));
                // ui->listWidget->item (iPlaylistItem)->setTextColor (m_playedTextColor);
                break;
            }
        }
    }
    m_bAutoplay = settings.value("AutoPlay", true).toBool ();
    m_bShowInfo = settings.value("ShowInfo", true).toBool ();
    if (m_bShowInfo == false)
    {
        if (m_infoWidget != nullptr) m_infoWidget->hide ();
    }
    else
    {
        if (m_bInfoWindowHasBeenClosed == false)
        {
            m_bInfoWindowHasBeenMinimized = false;
            m_infoWidget->show ();
            //m_infoWidget->showNormal();
            //m_infoWidget->show ();
            m_infoWidget->raise();
        }
    }
    //m_infoWidget->setStyle (this->style ());
}

void Widget::handleModeButton()
{
    QMediaPlaylist::PlaybackMode mode = m_playlist->playbackMode();
    switch (mode)
    {
    case QMediaPlaylist::CurrentItemOnce:
        m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);  // Loop All
        break;
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
        m_playlist->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);
        break;
    default:
        m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        break;
    }
    updateModeButtonIcon();
    QSettings settings;
    settings.setValue("playbackMode", static_cast<int>(m_playlist->playbackMode()));
    //saveSettings();  // Save current mode
}

void Widget::updateModeButtonIcon()
{
    switch (m_playlist->playbackMode())
    {
    case QMediaPlaylist::Sequential:
        ui->modeButton->setIcon(QIcon(":/img/img/icons8-right-48.png"));
        ui->modeButton->setToolTip("Sequential");
        break;
    case QMediaPlaylist::Loop:
        ui->modeButton->setIcon(QIcon(":/img/img/icons8-loop-48.png"));
        ui->modeButton->setToolTip("Loop all");
        break;
    case QMediaPlaylist::CurrentItemInLoop:
        ui->modeButton->setIcon(QIcon(":/img/img/icons8-repeat-one-48.png"));
        ui->modeButton->setToolTip("Loop current");
        break;
    case QMediaPlaylist::CurrentItemOnce:
        ui->modeButton->setIcon(QIcon(":/img/img/icons8-once-48.png"));
        ui->modeButton->setToolTip("Play current once");
        break;
    case QMediaPlaylist::Random:
        ui->modeButton->setIcon(QIcon(":/img/img/icons8-random-48.png"));
        ui->modeButton->setToolTip("Shuffle (Random)");
        break;
    default:
        break;
    }
}

void Widget::handleLoadPlaylist()
{
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QSettings settings;
    m_lastDialogPlaylistPath = settings.value("m_lastDialogPlaylistPath", documentsPath).toString ();
    // QFileInfo fileInfo(m_lastDialogPlaylistPath);
    // QString path = fileInfo.absolutePath(); // or .path()
    QString fileName = QFileDialog::getOpenFileName(
            this, tr("Load Playlist"), m_lastDialogPlaylistPath, tr("Playlist (*.m3u);;All Files (*)"));
    if (fileName.isEmpty())
        return;
    loadPlaylistFile(fileName);
    // if you want to immediately select/play the last track, you can:
    if (m_playlist->mediaCount() > 0)
    {
        if (m_bAutoplay)
        {
            m_player->play();
            m_playlist->setCurrentIndex(0);
        }
    }
    saveSettings(); // store lastPlaylistPath etc.
}

void Widget::handleSavePlaylist()
{
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QSettings settings;
    m_lastDialogPlaylistPath = settings.value("m_lastDialogPlaylistPath", documentsPath).toString ();
    QString fileName = QFileDialog::getSaveFileName(
            this, tr("Save Playlist"), m_lastDialogPlaylistPath, tr("M3U Playlist (*.m3u);;All Files (*)"));
    if (fileName.isEmpty())
        return;
    savePlaylistFile(fileName);
}

void Widget::loadPlaylistFile(const QString &filePath, bool restoreLastTrack )
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    m_playlist->clear();
    ui->listWidget->clear();
    ui->listWidget->viewport()->update();
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#"))
            continue;
        QFileInfo fi(line);
        if (fi.exists())
            addFileToPlaylist(fi.absoluteFilePath());
    }
    file.close();
    if (restoreLastTrack && m_playlist->mediaCount() > 0)
    {
        // Restore last track index safely
        int idx = qBound(0, m_lastTrackIndex, m_playlist->mediaCount() - 1);
        m_playlist->setCurrentIndex(idx);
        ui->listWidget->setCurrentRow(idx);
        ui->listWidget->scrollToItem(ui->listWidget->item(idx), QAbstractItemView::PositionAtCenter);
        if (m_lastTrackPosition > 0)
            m_player->setPosition(m_lastTrackPosition);
    }
    if (m_playlist->mediaCount() <= 1 && m_playlist->playbackMode() == QMediaPlaylist::Random)
    {
        m_shuffleHistory.clear(); // no history needed
        m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        updateModeButtonIcon();
    }
    QSettings settings;
    settings.setValue("m_lastDialogPlaylistPath", filePath);
}

void Widget::savePlaylistFile(const QString &path)
{
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot save playlist: %1").arg(path));
        return;
    }
    QTextStream out(&f);
    for (int i = 0; i < m_playlist->mediaCount(); ++i)
    {
        QUrl url = m_playlist->media(i).canonicalUrl();
        if (url.isLocalFile())
            out << url.toLocalFile() << "\n";
    }
    f.close();
    m_lastPlaylistPath = path;
    QSettings settings;
    settings.setValue("m_lastDialogPlaylistPath", path);
    saveSettings(); // persist lastPlaylistPath and playlist if you want immediacy
}

void Widget::showPlaylistContextMenu(const QPoint &pos)
{
    QMenu contextMenu(this);
    QAction *scrollToPlayingAction = contextMenu.addAction(tr("Scroll to currently playing"));
    scrollToPlayingAction->setIcon(QIcon(":/img/img/icons8-search-in-list-48.png"));
    contextMenu.addSeparator();
    QAction *removeSelectedAction = contextMenu.addAction(tr("Remove selected"));
    QAction *clearExceptSelectedAction = contextMenu.addAction(tr("Clear all except selected"));
    QAction *clearAction = contextMenu.addAction(tr("Clear playlist"));
    contextMenu.addSeparator();
    QAction* searchAction = contextMenu.addAction(tr("Search on Google"));
    searchAction->setIcon(QIcon(":/img/img/icons8-google-48.png"));
    contextMenu.addSeparator();
    QAction *loadAction = contextMenu.addAction(tr("Load playlist"));
    QAction *saveAction = contextMenu.addAction(tr("Save playlist"));
    saveAction->setIcon(QIcon(":/img/img/icons8-folder-save-48.png"));
    loadAction->setIcon(QIcon(":/img/img/icons8-folder-load-48.png"));
    clearAction->setIcon(QIcon(":/img/img/icons8-clear-48.png"));
    removeSelectedAction->setIcon(QIcon(":/img/img/icons8-delete-48.png"));
    QAction *selectedAction = contextMenu.exec(ui->listWidget->mapToGlobal(pos));
    if (selectedAction == saveAction)
    {
        handleSavePlaylist();
    }
    else if (selectedAction == loadAction)
    {
        handleLoadPlaylist();
    }
    else if (selectedAction == removeSelectedAction)
    {
        handleRemoveSelected();
    }
    else if (selectedAction == clearExceptSelectedAction)
    {
        clearExceptSelected();
    }
    else if (selectedAction == clearAction)
    {
        clearPlaylist(false); // ask confirmation
    }
    else if (selectedAction == scrollToPlayingAction)
    {
        scrollToCurrentTrack ();
    }
    else if (selectedAction == searchAction)
    {
        QString sTrackWithoutExtension = ui->listWidget->currentItem ()->text ();
        QFileInfo info(sTrackWithoutExtension);
        sTrackWithoutExtension = info.completeBaseName();
        openGoogleSearch (sTrackWithoutExtension);
    }
}

void Widget::showModeButtonContextMenu(const QPoint &pos)
{
    QMenu contextMenu(this);
    QAction *playModeCurrentItemOnceAction = contextMenu.addAction(tr("Play current once"));
    QAction *playModeSequentialAction = contextMenu.addAction(tr("Sequential"));
    QAction *playModeLoopAction = contextMenu.addAction(tr("Loop all"));
    QAction *playModeCurrentItemLoopAction = contextMenu.addAction(tr("Loop current"));
    QAction *playModeRandomAction = contextMenu.addAction(tr("Shuffle (Random)"));
    playModeSequentialAction->setIcon(QIcon(":/img/img/icons8-right-48.png"));
    playModeLoopAction->setIcon(QIcon(":/img/img/icons8-loop-48.png"));
    playModeCurrentItemLoopAction->setIcon(QIcon(":/img/img/icons8-repeat-one-48.png"));
    playModeCurrentItemOnceAction->setIcon(QIcon(":/img/img/icons8-once-48.png"));
    playModeRandomAction->setIcon(QIcon(":/img/img/icons8-random-48.png"));
    QAction *selectedAction = contextMenu.exec(ui->modeButton->mapToGlobal(pos));
    if (selectedAction == playModeSequentialAction)
    {
        m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
    }
    else if (selectedAction == playModeLoopAction)
    {
        m_playlist->setPlaybackMode(QMediaPlaylist::Loop);  // Loop All
    }
    else if (selectedAction == playModeCurrentItemLoopAction)
    {
        m_playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);  // Loop One
    }
    else if (selectedAction == playModeCurrentItemOnceAction)
    {
        m_playlist->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);  // Loop One
    }
    else if (selectedAction == playModeRandomAction)
    {
        m_playlist->setPlaybackMode(QMediaPlaylist::Random);  // Shuffle
    }
    updateModeButtonIcon();
    QSettings settings;
    settings.setValue("playbackMode", static_cast<int>(m_playlist->playbackMode()));
}

void Widget::showVolumeSliderContextMenu(const QPoint &pos)
{
    QMenu contextMenu(this);
    QAction *systemVolumeAction = contextMenu.addAction(tr("System volume"));
    QAction *playerVolumeAction = contextMenu.addAction(tr("AudioPlayer volume"));
    systemVolumeAction->setCheckable(true);
    playerVolumeAction->setCheckable(true);
    if (m_bSystemVolumeSlider)
    {
        systemVolumeAction->setChecked (true);
        playerVolumeAction->setChecked (false);
    }
    else
    {
        systemVolumeAction->setChecked (false);
        playerVolumeAction->setChecked (true);
    }
    QAction *selectedAction = contextMenu.exec(ui->volumeSlider->mapToGlobal(pos));
    if (selectedAction == systemVolumeAction)
    {
        systemVolumeAction->setChecked (true);
        playerVolumeAction->setChecked (false);
        m_bSystemVolumeSlider = true;
        int iVol = int (m_systemVolumeController->volume () * 100.01f);
        qDebug() << "Current master volume:" << iVol;
        ui->volumeSlider->setValue (iVol);
        ui->volumeLabel->setText(QString::number(iVol) + "%");
    }
    else if (selectedAction == playerVolumeAction)
    {
        systemVolumeAction->setChecked (false);
        playerVolumeAction->setChecked (true);
        m_bSystemVolumeSlider = false;
        ui->volumeSlider->setValue (m_lastVolume);
    }
    updateMuteButtonIcon ();
}

void Widget::clearExceptSelected()
{
    int selectedRow = ui->listWidget->currentRow();
    if (selectedRow < 0 || selectedRow >= m_playlist->mediaCount())
        return;
    QUrl currentTrackUrl = m_playlist->media(selectedRow).canonicalUrl();
    qint64 currentPosition = m_player->position();
    // Remove all items except the selected one
    for (int i = m_playlist->mediaCount() - 1; i >= 0; --i)
    {
        if (i == selectedRow)
            continue;
        // Remove from shuffle history if used
        for (int j = m_shuffleHistory.size() - 1; j >= 0; --j)
        {
            if (m_shuffleHistory[j] == i)
                m_shuffleHistory.remove(j);
            else if (m_shuffleHistory[j] > i)
                m_shuffleHistory[j]--; // shift down
        }
        delete ui->listWidget->takeItem(i);
        m_playlist->removeMedia(i);
    }
    // Keep the selected track
    m_playlist->setCurrentIndex(0);
    m_player->setPosition(currentPosition);
    if (m_player->state() == QMediaPlayer::PlayingState)
    {
        handlePlay ();
    }
    if (m_playlist->mediaCount() <= 1 && m_playlist->playbackMode() == QMediaPlaylist::Random)
    {
        m_shuffleHistory.clear(); // no history needed
        m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        updateModeButtonIcon();
    }
    saveSettings();
}

void Widget::handleRemoveSelected()
{
    QList<QListWidgetItem *> selectedItems = ui->listWidget->selectedItems();
    if (selectedItems.isEmpty())
        return;
    QUrl currentTrackUrl = m_playlist->currentMedia().canonicalUrl();
    qint64 currentPosition = m_player->position();
    // Sort items from bottom to top to avoid row shifting
    std::sort(selectedItems.begin(), selectedItems.end(),
        [this](QListWidgetItem * a, QListWidgetItem * b)
    {
        return ui->listWidget->row(a) > ui->listWidget->row(b);
    });
    bool currentRemoved = false;
    for (QListWidgetItem *item : selectedItems)
    {
        int row = ui->listWidget->row(item);
        QUrl trackUrl = m_playlist->media(row).canonicalUrl();
        if (trackUrl == currentTrackUrl)
            currentRemoved = true;
        // Remove any reference in shuffle history
        for (int i = m_shuffleHistory.size() - 1; i >= 0; --i)
        {
            if (m_shuffleHistory[i] == row)
                m_shuffleHistory.remove(i);
            else if (m_shuffleHistory[i] > row)
                m_shuffleHistory[i]--; // shift down indices
        }
        delete ui->listWidget->takeItem(row);
        m_playlist->removeMedia(row);
    }
    if (m_playlist->mediaCount() == 0)
    {
        m_player->stop();
        saveSettings(); // update saved playlist
        return;
    }
    if (currentRemoved)
    {
        // Current track removed → play next logical track
        int newIndex = m_playlist->currentIndex();
        if (newIndex >= m_playlist->mediaCount())
            newIndex = m_playlist->mediaCount() - 1;
        if (m_playlist->playbackMode() == QMediaPlaylist::Random)
            m_shuffleHistory.clear();
        m_playlist->setCurrentIndex(newIndex);
        if (m_player->state() == QMediaPlayer::PlayingState)
        {
            handlePlay ();
        }
    }
    else
    {
        // Current track still exists → continue playing without restarting
        int existingIndex = -1;
        for (int i = 0; i < m_playlist->mediaCount(); ++i)
        {
            if (m_playlist->media(i).canonicalUrl() == currentTrackUrl)
            {
                existingIndex = i;
                break;
            }
        }
        if (existingIndex >= 0)
        {
            if (existingIndex != m_playlist->currentIndex())
                m_playlist->setCurrentIndex(existingIndex);
            m_player->setPosition(currentPosition);
            if (m_player->state() == QMediaPlayer::PlayingState)
            {
                handlePlay ();
            }
        }
    }
    if (m_playlist->mediaCount() <= 1 && m_playlist->playbackMode() == QMediaPlaylist::Random)
    {
        m_shuffleHistory.clear(); // no history needed
        m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        updateModeButtonIcon();
    }
    saveSettings(); // persist updated playlistFiles
}

void Widget::setTheme()
{
    //qDebug() << __PRETTY_FUNCTION__ ;
    if (m_sTheme != "")
    {
        QApplication::setStyle(QStyleFactory::create(m_sTheme));
    }
    if (m_sPalette != "")
    {
        if (m_sPalette == "Dark")
        {
            // QPalette darkPalette;
            // darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
            // darkPalette.setColor(QPalette::WindowText, Qt::white);
            // darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
            // darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
            // darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
            // darkPalette.setColor(QPalette::ToolTipText, Qt::white);
            // darkPalette.setColor(QPalette::Text, Qt::white);
            // darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
            // darkPalette.setColor(QPalette::ButtonText, Qt::white);
            // darkPalette.setColor(QPalette::BrightText, Qt::red);
            // darkPalette.setColor(QPalette::Highlight, QColor(142, 45, 197).lighter());
            // darkPalette.setColor(QPalette::HighlightedText, Qt::black);
            QPalette darkPalette;
            darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
            darkPalette.setColor(QPalette::WindowText, Qt::white);
            darkPalette.setColor(QPalette::Base, QColor(35, 35, 35));
            darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
            darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
            darkPalette.setColor(QPalette::ToolTipText, Qt::white);
            darkPalette.setColor(QPalette::Text, Qt::white);
            darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
            darkPalette.setColor(QPalette::ButtonText, Qt::white);
            darkPalette.setColor(QPalette::BrightText, Qt::red);
            darkPalette.setColor(QPalette::Highlight, QColor(173, 216, 230));
            darkPalette.setColor(QPalette::HighlightedText, Qt::black);
            QApplication::setPalette(darkPalette);
        }
        else
        {
            QApplication::setPalette(QApplication::style()->standardPalette());
        }
    }
    if (m_sTheme != "" )
    {
        // Force widgets to re-polish
        foreach (QWidget *widget, QApplication::allWidgets())
        {
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
            widget->update();
        }
    }
}

void Widget::scrollToCurrentTrack()
{
    int currentIndex = m_playlist->currentIndex();
    if (currentIndex >= 0 && currentIndex < ui->listWidget->count())
    {
        QListWidgetItem *item = ui->listWidget->item(currentIndex);
        ui->listWidget->setCurrentRow(currentIndex);  // highlight
        ui->listWidget->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    }
}

void Widget::onSystemVolumeChanged(float newVolume)
{
    qDebug() << "System volume changed: " << newVolume;
    if (m_bSystemVolumeSlider)
    {
        int iVol = int (m_systemVolumeController->volume () * 100.01f);
        qDebug() << "Current master volume:" << iVol;
        QSignalBlocker blocker(ui->volumeSlider);
        ui->volumeSlider->setValue (iVol);
        ui->volumeLabel->setText(QString::number(iVol) + "%");
    }
}

void Widget::onSystemMuteChanged(bool muted)
{
    qDebug() << "System mute state changed: " << muted;
    if (m_bSystemVolumeSlider)
    {
        updateMuteButtonIcon ();
        if (muted) ui->volumeLabel->setText("0%");
    }
}

void Widget::onDefaultDeviceChanged()
{
    qDebug() << "Widget: Detected new default output device";
    bool wasPlaying = false;
    if (m_player && m_player->state() == QMediaPlayer::PlayingState)
    {
        wasPlaying = true;
        m_player->pause (); // Stop to release old device
    }
    // Recreate QAudioOutput or QMediaPlayer
    // delete m_audioOutput;
    // m_audioOutput = new QAudioOutput(this);
    // m_player->setAudioOutput(m_audioOutput);
    // 2. Reinitialize SystemVolumeController safely
    m_systemVolumeController->blockSignals(true);
    m_systemVolumeController->cleanup();
    m_systemVolumeController->initialize();
    m_systemVolumeController->blockSignals(false);
    // Sync slider/label with new device volume
    //handleVolumeChanged(int(m_systemVolumeController->volume() * 100.0f));
    if (wasPlaying)
    {
        m_player->play(); // Resume playback
    }
    // Refresh slider & label with new device volume
    onSystemVolumeChanged(m_systemVolumeController->volume());
    QMessageBox::warning (this, "Audio device changed", "Default audio playback device changed.");
}

void Widget::onDeviceChanged(const QString &deviceId, const QString &friendlyName)
{
    qDebug() << __PRETTY_FUNCTION__ << "Device name: " << friendlyName;
    QString sToolTipMessage = "Default audio playback device changed to:<br><b>" + friendlyName + "</b>";
    QPoint globalPos = ui->volumeSlider->mapToGlobal(QPoint(ui->volumeSlider->width() / 2, ui->volumeSlider->height() / 2));
    QToolTip::showText(globalPos, sToolTipMessage, ui->volumeSlider);
    //QMessageBox::warning (this, "Audio device changed", "Default audio playback device changed.");
    bool wasPlaying = false;
    if (m_player && m_player->state() == QMediaPlayer::PlayingState)
    {
        wasPlaying = true;
        m_player->pause (); // Stop to release old device
    }
    m_systemVolumeController->blockSignals(true);
    m_systemVolumeController->cleanup();
    m_systemVolumeController->initialize();
    m_systemVolumeController->blockSignals(false);
    // Sync slider/label with new device volume
    //handleVolumeChanged(int(m_systemVolumeController->volume() * 100.0f));
    if (wasPlaying)
    {
        m_player->play(); // Resume playback
    }
    onSystemVolumeChanged(m_systemVolumeController->volume());
}
