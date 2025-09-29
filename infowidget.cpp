#include "infowidget.h"
#include "ui_infowidget.h"
#include <QMouseEvent>
#include <QTextEdit>
#include <QSize>
#include <QAbstractTextDocumentLayout>
#include <QTextDocument>
#include <QDebug>
//#include <QScreen>
#include <QSettings>
#include <QPalette>
#include <QIcon>
#include <mpegfile.h>
#include <mpegproperties.h>
#include <mp4file.h>
#include <flacfile.h>
#include <opusfile.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <vorbisfile.h>
#include <speexfile.h>
#include <wavfile.h>
#include <wavproperties.h>

InfoWidget::InfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoWidget),
    m_Info("")
{
    ui->setupUi(this);
    setWindowIcon (QIcon(":/img/img/icons8-play-32.ico"));
    setWindowFlags(Qt::Tool | Qt::MSWindowsFixedSizeDialogHint);
    loadSettings();
    QPalette palette = ui->textEditInfo->palette();
    palette.setColor(QPalette::Base, palette.color(QPalette::Window));
    ui->textEditInfo->setPalette (palette);
    // Imposta la forma del frame a nessuna forma
    ui->textEditInfo->setFrameShape(QFrame::NoFrame);
    // Imposta l'ombra del frame a nessuna ombra
    ui->textEditInfo->setFrameShadow(QFrame::Plain);
    ui->textEditInfo->move(0, 0);
    ui->textEditInfo-> setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    // 1. Disabilita il ritorno a capo per far contare la larghezza
    ui->textEditInfo-> setLineWrapMode(QTextEdit::NoWrap);
    // 2. Disattiva le barre di scorrimento su entrambi gli assi
    ui->textEditInfo-> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->textEditInfo-> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QTextDocument *doc = ui->textEditInfo->document ();
    QAbstractTextDocumentLayout *docLayout = doc->documentLayout ();
    connect( docLayout, &QAbstractTextDocumentLayout::documentSizeChanged,
        this, updateSize);
}

InfoWidget::~InfoWidget()
{
    delete ui;
}

void InfoWidget::updateSize(const QSizeF &newSize)
{
    // Calcola l'altezza come prima
    int contentHeight = newSize.height();
    // Calcola la larghezza ideale (la riga più lunga)
    int contentWidth = ui->textEditInfo->document()->idealWidth();
    // Aggiungi margini, bordi e scroll bar (anche se disabilitati, a volte l'overhead resta)
    int totalHeight = contentHeight + 2 * ui->textEditInfo->frameWidth();;
    int totalWidth = contentWidth;// + 2 * ui->textEditInfo->frameWidth();
    // Imposta la dimensione fissa (sia altezza che larghezza)
    ui->textEditInfo->setFixedSize(totalWidth, totalHeight);
    setFixedSize(totalWidth, totalHeight);
}

void InfoWidget::setInfo(QString sInfo)
{
    ui->textEditInfo->setText (sInfo);
}

QString InfoWidget::getInfo()
{
    return m_Info;
}

void InfoWidget::setFile(const QString &localFile)
{
    bool bFomatFound = false;
    m_Info = "Info is empty.";
    if (!localFile.isEmpty())
    {
        // TagLib::FileRef f(TagLib::FileName(localFile.toUtf8().constData()));
        std::wstring wpath = localFile.toStdWString();
        m_FileRef = new TagLib::FileRef(TagLib::FileName(wpath.c_str()));
        // m_FileRef = new TagLib::FileRef(TagLib::FileName(localFile.toUtf8().constData()));
        if (!m_FileRef->isNull() && m_FileRef->tag())
        {
            TagLib::Tag *tag = m_FileRef->tag();
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
            // qDebug() << "Title:" << (title.isEmpty() ? "[Unknown]" : title);
            // qDebug() << "Artist:" << (artist.isEmpty() ? "[Unknown]" : artist);
            // qDebug() << "Album:" << (album.isEmpty() ? "[Unknown]" : album);
            // qDebug() << "Year:" << (year == 0 ? "[Unknown]" : QString::number(year));
            // qDebug() << "Track:" << (trackNum == 0 ? "[Unknown]" : QString::number(trackNum));
            // qDebug() << "Genre:" << (genre.isEmpty() ? "[Unknown]" : genre);
            // Optional: Format string for UI
            // QString info = QString("Artist: %1\nTitle: %2\nAlbum: %3\nYear: %4").arg(artist.isEmpty() ? "[Unknown Artist]" : artist,
            // title.isEmpty() ? "[Unknown Title]" : title, album.isEmpty() ? "[Unknown Album]" : album, year == 0 ? "[Unknown Year]" : QString::number(year));
            // QString info = QString("<b>Artist:</b> %1<br><b>Title:</b> %2<br>Album: %3\nYear: %4").arg(artist.isEmpty() ? "[Unknown Artist]" : artist,
            // title.isEmpty() ? "[Unknown Title]" : title, album.isEmpty() ? "[Unknown Album]" : album, year == 0 ? "[Unknown Year]" : QString::number(year));
            QString info;
            if (artist.isEmpty () == false)
            {
                info.append ("Artist: ");
                info.append (artist);
            }
            //info.append (artist.isEmpty() ? "[Unknown artist]" : artist);
            if (title.isEmpty () == false)
            {
                info.append ("\n");
                info.append ("Title: ");
                info.append (title);
            }
            // info.append ( title.isEmpty() ? "[Unknown title]" : title);
            if (album.isEmpty () == false)
            {
                info.append ("\n");
                info.append ("Album: ");
                info.append (album);
            }
            // info.append ( album.isEmpty() ? "[Unknown album]" : album);
            if (trackNum != 0)
            {
                info.append ("\n");
                info.append ("Track: ");
                info.append (QString::number(trackNum));
            }
            // info.append (trackNum == 0 ? "[Unknown track number]" : QString::number(trackNum));
            if (year != 0)
            {
                info.append ("\n");
                info.append ("Year: ");
                info.append (QString::number(year));
            }
            // info.append ( year == 0 ? "[Unknown year]" : QString::number(year));
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
            if (m_FileRef->isNull() || !m_FileRef->audioProperties())
            {
                qDebug() << "Errore: File non valido o proprietà audio non trovate.";
                return;
            }
            // 2. Ottieni l'oggetto AudioProperties
            TagLib::AudioProperties *properties = m_FileRef->audioProperties();
            int lengthSeconds = properties->lengthInSeconds();
            int bitrate = properties->bitrate();
            int sampleRate = properties->sampleRate();
            int channels = properties->channels();
            // 4. Stampa i dettagli
            info.append ("\n");
            info.append ("Lenght: ");
            info.append (formatTime (lengthSeconds));
            // info.append (QString::number (lengthSeconds));
            // info.append (" seconds");
            info.append ("\n");
            info.append ("Bitrate (kbit/s): ");
            info.append (QString::number (bitrate));
            info.append ("\n");
            info.append ("Sample Rate (Hz): ");
            info.append (QString::number (sampleRate));
            info.append ("\n");
            info.append ("Channels: ");
            info.append (QString::number (channels));
            //delete properties;
            TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File *>(m_FileRef->file());
            if (mpegFile)
            {
                TagLib::MPEG::Properties *prop = mpegFile->audioProperties ();
                qDebug() << "Formato: MPEG (MP3)";
                info.append ("\n");
                info.append ("Channel mode: ");
                //TagLib::MPEG::Header::ChannelMode chanMode = mpegFile->audioProperties ()->channelMode ();
                int chanMode=prop->channelMode ();
                switch (chanMode)
                {
                case 0:
                    info.append ("Stereo");
                    break;
                case 1:
                    info.append ("Joint stereo");
                    break;
                case 2:
                    info.append ("Dual channel (Dual mono)");
                    break;
                case 3:
                    info.append ("Single channel (Mono)");
                    break;
                default:
                    break;
                }
                info.append ("\n");
                info.append ("Format: MPEG");
                // int layer=prop->layer ();
                int layer = prop->layer ();
                switch (layer)
                {
                case 0:
                    info.append(" Layer 0");
                    break;
                case 1:
                    info.append(" Layer 1");
                    break;
                case 2:
                    info.append(" Layer 2 (MP2)");
                    break;
                case 3:
                    info.append(" Layer 3 (MP3)");
                    break;
                default:
                    break;
                }
                // TagLib::MPEG::Header::Version vers = mpegFile->audioProperties ()->version ();
                int vers = prop->version ();
                switch (vers)
                {
                case 0:
                    info.append(" Version 1");
                    break;
                case 1:
                    info.append(" Version 2");
                    break;
                case 2:
                    info.append(" Version 2.5");
                    break;
                case 3:
                    info.append(" Version 4");
                    break;
                default:
                    break;
                }
                // info.append (mpegFile->audioProperties ()->version ());
                // Puoi ottenere anche più dettagli specifici qui, come il bit per campione
                // (properties->bitsPerSample() non è sempre affidabile in TagLib,
                // specialmente per MP3)
                // delete mpegFile;
                bFomatFound = true;
            }
            if (!bFomatFound)
            {
                TagLib::MP4::File *mp4File = dynamic_cast<TagLib::MP4::File *>(m_FileRef->file());
                if (mp4File)
                {
                    // Se il cast ha successo, è un file MP4/M4A.
                    qDebug() << "Il file è un MP4 (M4A)";
                    // Qui puoi continuare a lavorare con 'mp4File' per accedere
                    // a proprietà specifiche MP4 come i tag 'covr' (immagine)
                    info.append ("\n");
                    info.append ("Format: MP4 (AAC, ALAC, M4A...) ");
                    bFomatFound = true;
                }
            }
            if (!bFomatFound)
            {
                TagLib::FLAC::File *flacFile = dynamic_cast<TagLib::FLAC::File *>(m_FileRef->file());
                {
                    if (flacFile)
                    {
                        // Se il cast ha successo, è un file FLAC.
                        qDebug() << "Il file è un formato FLAC";
                        // Qui puoi continuare a lavorare con 'flacFile' per accedere
                        // a proprietà specifiche del FLAC se necessario.
                        info.append ("\n");
                        info.append ("Format: FLAC");
                        bFomatFound = true;
                    }
                }
            }
            if (!bFomatFound)
            {
                TagLib::Ogg::Opus::File *opusFile = dynamic_cast<TagLib::Ogg::Opus::File *>(m_FileRef->file());
                if (opusFile)
                {
                    // Se il cast ha successo, è un file FLAC.
                    qDebug() << "Il file è un formato Ogg Opus";
                    // Qui puoi continuare a lavorare con 'flacFile' per accedere
                    // a proprietà specifiche del FLAC se necessario.
                    info.append ("\n");
                    info.append ("Format: Ogg Opus");
                    bFomatFound = true;
                }
            }
            if (!bFomatFound)
            {
                TagLib::Ogg::Vorbis::File *oggFile = dynamic_cast<TagLib::Ogg::Vorbis::File *>(m_FileRef->file());
                if (oggFile)
                {
                    // Se il cast ha successo, è un file FLAC.
                    qDebug() << "Il file è un formato Ogg Vorbis";
                    // Qui puoi continuare a lavorare con 'flacFile' per accedere
                    // a proprietà specifiche del FLAC se necessario.
                    info.append ("\n");
                    info.append ("Format: Ogg Vorbis");
                    bFomatFound = true;
                }
            }
            if (!bFomatFound)
            {
                TagLib::Ogg::FLAC::File *oggFlacFile = dynamic_cast<TagLib::Ogg::FLAC::File *>(m_FileRef->file());
                if (oggFlacFile)
                {
                    // Se il cast ha successo, è un file FLAC.
                    qDebug() << "Il file è un formato Ogg FLAC";
                    // Qui puoi continuare a lavorare con 'flacFile' per accedere
                    // a proprietà specifiche del FLAC se necessario.
                    info.append ("\n");
                    info.append ("Format: Ogg FLAC");
                    bFomatFound = true;
                }
            }
            if (!bFomatFound)
            {
                TagLib::Ogg::Speex::File *speexFile = dynamic_cast<TagLib::Ogg::Speex::File *>(m_FileRef->file());
                if (speexFile)
                {
                    // Se il cast ha successo, è un file FLAC.
                    qDebug() << "Il file è un formato Ogg Speex";
                    // Qui puoi continuare a lavorare con 'flacFile' per accedere
                    // a proprietà specifiche del FLAC se necessario.
                    info.append ("\n");
                    info.append ("Format: Ogg Speex");
                    bFomatFound = true;
                }
            }
            if (!bFomatFound)
            {
                TagLib::RIFF::WAV::File *wavFile = dynamic_cast<TagLib::RIFF::WAV::File *>(m_FileRef->file());
                if (wavFile)
                {
                    info.append ("\n");
                    info.append ("Bits: ");
                    info.append (QString::number (wavFile->audioProperties ()->bitsPerSample()));
                    int iFormat = wavFile->audioProperties ()->format ();
                    // Se il cast ha successo, è un file WAV.
                    qDebug() << "Il file è un formato WAV (RIFF)";
                    info.append ("\n");
                    info.append ("Format: ");
                    if (iFormat == 1) info.append ("PCM");
                    else if (iFormat == 0) info.append ("Unknown format (0)");
                    else if (iFormat == 2) info.append ("Compressed ADPCM");
                    else if (iFormat == 3) info.append ("IEEE float");
                    else if (iFormat == 6) info.append ("A-law");
                    else if (iFormat == 7) info.append ("µ-law");
                    else info.append ("Unknown fomat");
                    info.append (" WAV (RIFF)");
                    bFomatFound = true;
                }
            }
            QFile file(localFile);
            if (file.exists())
            {
                qint64 size = file.size();
                info.append ("\n");
                info.append ("File size: ");
                info.append (formatFileSize (size));
            }
            else
            {
                qDebug() << "File does not exist.";
            }
            m_Info = info;
            setInfo (info);
            // qDebug() << "--- Dettagli Audio ---";
            // qDebug() << "Durata:" << lengthSeconds << "secondi";
            // qDebug() << "Bitrate (kbit/s):" << bitrate;
            // qDebug() << "Sample Rate (Hz):" << sampleRate;
            // qDebug() << "Canali:" << channels;
            // if (m_infoWidget != nullptr) m_infoWidget->setInfo (info);
            // if (m_bShowInfo == false)
            // {
            // QPoint globalPos = ui->listWidget->mapToGlobal(QPoint(ui->listWidget->width() / 2, ui->listWidget->height() / 2));
            // QToolTip::showText(globalPos, info, ui->listWidget);
            // }
            // else
            // {
            // if (m_infoWidget == nullptr) m_infoWidget = new InfoWidget();
            // m_infoWidget->raise ();
            //                // m_infoWidget->setInfo (info);
            // m_infoWidget->show ();
            // }
            m_FileRef->file ()->clear ();
            delete m_FileRef;
            m_FileRef = nullptr;
        }
        else
        {
            qDebug() << "Failed to read metadata!";
            setInfo ("Failed to read metadata!");
        }
    }
    else
    {
        qDebug() << "File path is empty!";
    }
}

//void InfoWidget::on_pushButtonClose_clicked()
//{
// this->close ();
//}

QString InfoWidget::formatFileSize(qint64 bytes)
{
    const double KB = 1024.0;
    const double MB = KB * 1024.0;
    if (bytes < MB)
    {
        return QString::number(bytes / KB, 'f', 2) + " KB";
    }
    else
    {
        return QString::number(bytes / MB, 'f', 2) + " MB";
    }
}

void InfoWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // Chiamiamo la funzione close() per chiudere il widget di primo livello (la finestra).
        //qDebug("Finestra chiusa tramite click del mouse!");
        //close();
        //deleteLater();
    }
    // È buona norma chiamare l'implementazione della classe base
    // nel caso in cui ci siano altre elaborazioni da eseguire (anche se in questo caso non fa nulla di essenziale per la chiusura).
    QWidget::mousePressEvent(event);
}

void InfoWidget::moveEvent(QMoveEvent *event)
{
    // Chiamiamo la funzione di base per garantire che l'evento venga gestito normalmente
    QWidget::moveEvent(event);
    // 2. Chiama la funzione di salvataggio
    saveSettings();
    // (Opzionale) Puoi usare event->pos() per vedere la nuova posizione, ma
    // QWidget::pos() è più diretto dopo la chiamata alla base.
    //qDebug() << "Window moved to: " << pos();
}

void InfoWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::StyleChange || event->type() == QEvent::PaletteChange)
    {
        //qDebug() << "Stile del widget cambiato!";
        // Inserisci qui il tuo codice per reagire al cambio di stile
        //QPalette palette = ui->textEditInfo->palette();
        QPalette palette = this->palette();
        palette.setColor(QPalette::Base, palette.color(QPalette::Window));
        ui->textEditInfo->setPalette (palette);
    }
    else if (event->type() == QEvent::ActivationChange)
    {
        if (isActiveWindow())
        {
            emit focusReceived ();
        }
    }
    // 2. Chiama l'implementazione della classe base
    QWidget::changeEvent(event);
}

void InfoWidget::closeEvent(QCloseEvent *event)
{
    emit windowClosed();
    event->accept();
    QWidget::closeEvent(event);
}

void InfoWidget::saveSettings()
{
    // QSettings richiede il nome dell'Organizzazione e dell'Applicazione
    // per creare un percorso di salvataggio univoco.
    QSettings settings;
    // Salva la posizione corrente del widget (coordinate x, y)
    settings.setValue("InfoWidgetPosition", pos());
    // Potresti anche salvare la dimensione (larghezza, altezza)
    settings.setValue("InfoWidgetSsize", size());
    // In Qt, le impostazioni vengono salvate automaticamente
}

void InfoWidget::loadSettings()
{
    QSettings settings;
    // 1. Carica la posizione
    QPoint savedPos = settings.value("InfoWidgetPosition", QPoint(100, 100)).toPoint();
    // 2. Carica la dimensione
    QSize savedSize = settings.value("InfoWidgetSsize", QSize(300, 100)).toSize();
    // 3. Applica le impostazioni
    resize(savedSize);
    // Controlla se la posizione salvata è visibile su qualsiasi schermo
    // (Utile se l'utente ha staccato un monitor).
    QScreen *screen = QGuiApplication::screenAt(savedPos);
    if (screen)
    {
        move(savedPos);
    }
    else
    {
        // Se la posizione non è valida, centra la finestra
        // La posizione di default (100, 100) verrà usata se la chiave non esiste
        move(QPoint(100, 100));
    }
}

QString InfoWidget::formatTime(int totalSeconds)
{
    // 1. Calcola i minuti
    int minutes = totalSeconds / 60;
    // 2. Calcola i secondi rimanenti
    int seconds = totalSeconds % 60;
    // 3. Converte i minuti in stringa
    // Il formato è semplice, non è necessario il padding per i minuti
    QString minutesStr = QString::number(minutes);
    // 4. Converte i secondi in stringa, assicurando due cifre (padding)
    // - campo di 2 cifre (width = 2)
    // - carattere '0' per il riempimento (fillChar = '0')
    // - allineamento a destra (right justification)
    QString secondsStr = QString::number(seconds).rightJustified(2, '0');
    // 5. Combina il risultato nel formato finale
    return minutesStr + ":" + secondsStr;
}
