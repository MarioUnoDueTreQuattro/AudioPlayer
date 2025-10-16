#include "audiotag.h"

#include <QByteArray>
#include <QFileInfo>
#include <QDir>

//// TagLib includes
////#include <fileref.h>
////#include <tfile.h>
//#include <mpegfile.h>
//#include <id3v2tag.h>
////#include <id3v2frame.h>
//#include <flacfile.h>
//#include <flacpicture.h>
//#include <mp4file.h>
//#include <mp4tag.h>
//#include <mp4coverart.h>
//#include <apefile.h>
//#include <apetag.h>
////#include <wmafile.h>
//#include <asftag.h>
#include <QDebug>
//SingleInstance::SingleInstance(const QString &serverName, QObject *parent)
// : QObject(parent),

AudioTag::AudioTag(QObject *parent)
    : QObject(parent)
{
}

AudioTag::AudioTag(QString sFileName, QObject *parent)
{
    setFile(sFileName);
}

void AudioTag::setInfo(QString sInfo)
{
    // ui->textEditInfo->setText (sInfo);
}

QString AudioTag::formatFileSize(qint64 bytes)
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

QString AudioTag::formatTime(int totalSeconds)
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

void AudioTag::setFile(const QString &localFile, bool bExtractCover)
{
    m_TagInfo.reset();
    QString nativePath = QDir::toNativeSeparators(localFile);
    bool bFomatFound = false;
    m_Info = "Info is empty.";
    m_pix = QPixmap(); //empty
    if (!localFile.isEmpty())
    {
        // TagLib::FileRef f(TagLib::FileName(localFile.toUtf8().constData()));
        std::wstring wpath = localFile.toStdWString();
        //qDebug() << "wpath->" << wpath;
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
            struct Field
            {
                QString label;
                QString value;
            };
            QList<Field> fields;
            if (artist.isEmpty() == false)
            {
                fields.append({"Artist: ", artist});
                // info.append ("Artist: ");
                // info.append (artist);
            }
            //info.append (artist.isEmpty() ? "[Unknown artist]" : artist);
            if (title.isEmpty() == false)
            {
                fields.append({"Title: ", title});
                // info.append ("\n");
                // info.append ("Title: ");
                // info.append (title);
            }
            // info.append ( title.isEmpty() ? "[Unknown title]" : title);
            if (album.isEmpty() == false)
            {
                fields.append({"Album: ", album});
                // info.append ("\n");
                // info.append ("Album: ");
                // info.append (album);
            }
            // info.append ( album.isEmpty() ? "[Unknown album]" : album);
            if (trackNum != 0)
            {
                fields.append({"Track: ", QString::number(trackNum)});
                // info.append ("\n");
                // info.append ("Track: ");
                // info.append (QString::number(trackNum));
            }
            // info.append (trackNum == 0 ? "[Unknown track number]" : QString::number(trackNum));
            if (year != 0)
            {
                fields.append({"Year: ", QString::number(year)});
                // info.append ("\n");
                // info.append ("Year: ");
                // info.append (QString::number(year));
            }
            // info.append ( year == 0 ? "[Unknown year]" : QString::number(year));
            if (tag->genre().isEmpty() == false)
            {
                fields.append({"Genre: ", safeString(tag->genre())});
                // info.append ("\n");
                // info.append ("Genre: ");
                // info.append (safeString(tag->genre ()));
                m_TagInfo.sGenre = safeString(tag->genre());
            }
            if (tag->comment().isEmpty() == false)
            {
                fields.append({"Comment: ", safeString(tag->comment())});
                // info.append ("\n");
                // info.append ("Comment: ");
                // info.append (safeString(tag->comment ()));
                m_TagInfo.sComment = safeString(tag->comment());
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
            fields.append({"Lenght: ", formatTime(lengthSeconds)});
            // info.append ("\n");
            // info.append ("Lenght: ");
            // info.append (formatTime (lengthSeconds));
            // info.append (QString::number (lengthSeconds));
            // info.append (" seconds");
            fields.append({"Bitrate (kbit/s): ", QString::number(bitrate)});
            // info.append ("\n");
            // info.append ("Bitrate (kbit/s): ");
            // info.append (QString::number (bitrate));
            fields.append({"Sample Rate (Hz): ", QString::number(sampleRate)});
            // info.append ("\n");
            // info.append ("Sample Rate (Hz): ");
            // info.append (QString::number (sampleRate));
            fields.append({"Channels: ", QString::number(channels)});
            // info.append ("\n");
            // info.append ("Channels: ");
            // info.append (QString::number (channels));
            //delete properties;
            m_TagInfo.sFileName = localFile; //QString::fromStdWString (wpath);
            m_TagInfo.sTitle = title;
            m_TagInfo.sArtist = artist;
            m_TagInfo.sAlbum = album;
            m_TagInfo.sGenre = genre;
            m_TagInfo.iYear = year;
            m_TagInfo.iTrackNum = trackNum;
            m_TagInfo.iDuration = lengthSeconds;
            m_TagInfo.iBitrate = bitrate;
            m_TagInfo.iSamplerate = sampleRate;
            m_TagInfo.iChannels = channels;
            TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File *>(m_FileRef->file());
            if (mpegFile)
            {
                TagLib::MPEG::Properties *prop = mpegFile->audioProperties();
                qDebug() << "MPEG (MP3)";
                // info.append ("\n");
                // info.append ("Channel mode: ");
                //TagLib::MPEG::Header::ChannelMode chanMode = mpegFile->audioProperties ()->channelMode ();
                int chanMode = prop->channelMode();
                switch (chanMode)
                {
                    case 0:
                        fields.append(Field{"Channel mode: ", "Stereo"});
                        // info.append ("Stereo");
                        break;
                    case 1:
                        fields.append(Field{"Channel mode: ", "Joint stereo"});
                        // info.append ("Joint stereo");
                        break;
                    case 2:
                        fields.append(Field{"Channel mode: ", "Dual channel (Dual mono)"});
                        // info.append ("Dual channel (Dual mono)");
                        break;
                    case 3:
                        fields.append(Field{"Channel mode: ", "Single channel (Mono)"});
                        // info.append ("Single channel (Mono)");
                        break;
                    default:
                        break;
                }
                QString sLayer = "";
                QString sVersion = "";
                // info.append ("\n");
                // info.append ("Format: MPEG");
                // int layer=prop->layer ();
                int layer = prop->layer();
                switch (layer)
                {
                    case 0:
                        // info.append(" Layer 0");
                        sLayer = " Layer 0";
                        break;
                    case 1:
                        // info.append(" Layer 1");
                        sLayer = " Layer 1";
                        break;
                    case 2:
                        // info.append(" Layer 2 (MP2)");
                        sLayer = " Layer 2";
                        break;
                    case 3:
                        // info.append(" Layer 3 (MP3)");
                        sLayer = " Layer 3";
                        break;
                    default:
                        break;
                }
                // TagLib::MPEG::Header::Version vers = mpegFile->audioProperties ()->version ();
                int vers = prop->version();
                switch (vers)
                {
                    case 0:
                        // info.append(" Version 1");
                        sVersion = " Version 1";
                        break;
                    case 1:
                        // info.append(" Version 2");
                        sVersion = " Version 2";
                        break;
                    case 2:
                        // info.append(" Version 2.5");
                        sVersion = " Version 2.5";
                        break;
                    case 3:
                        // info.append(" Version 4");
                        sVersion = " Version 4";
                        break;
                    default:
                        break;
                }
                m_TagInfo.sFormat = "MPEG" + sLayer + sVersion;
                fields.append(Field{"Format: MPEG", sLayer + sVersion});
                // info.append (mpegFile->audioProperties ()->version ());
                // Puoi ottenere anche più dettagli specifici qui, come il bit per campione
                // (properties->bitsPerSample() non è sempre affidabile in TagLib,
                // specialmente per MP3)
                // delete mpegFile;
                bFomatFound = true;
                if (bExtractCover) m_pix = extractMP3Cover(mpegFile);
            }
            if (!bFomatFound)
            {
                TagLib::MP4::File *mp4File = dynamic_cast<TagLib::MP4::File *>(m_FileRef->file());
                if (mp4File)
                {
                    // Se il cast ha successo, è un file MP4/M4A.
                    qDebug() << "MP4 (M4A)";
                    // Qui puoi continuare a lavorare con 'mp4File' per accedere
                    // a proprietà specifiche MP4 come i tag 'covr' (immagine)
                    // info.append ("\n");
                    // info.append ("Format: MP4 (AAC, ALAC, M4A...) ");
                    //TagLib::MP4::Properties *prop=mp4File->properties ();
                    // TagLib::PropertyMap prop=mp4File->properties ();
                    // int iCodec=prop.codec ();
                    m_TagInfo.iBits = 16;
                    int iCodec = mp4File->audioProperties()->codec();
                    switch (iCodec)
                    {
                        case 0:
                            fields.append(Field{"Format: ", "MP4 (Unknown codec)"});
                            m_TagInfo.sFormat = "MP4 (Unknown codec)";
                            break;
                        case 1:
                            fields.append(Field{"Format: ", "MP4 (AAC codec)"});
                            m_TagInfo.sFormat = "MP4 (AAC codec)";
                            break;
                        case 2:
                            // ALAC can be 16 or 24 bits
                            fields.append(Field{"Bits: ", QString::number(mp4File->audioProperties()->bitsPerSample())});
                            m_TagInfo.iBits = mp4File->audioProperties()->bitsPerSample();
                            m_TagInfo.sFormat = "MP4 (ALAC lossless codec)";
                            fields.append(Field{"Format: ", "MP4 (ALAC lossless codec)"});
                            break;
                        default:
                            m_TagInfo.sFormat = "MP4 (AAC, ALAC, M4A...)";
                            fields.append(Field{"Format: ", "MP4 (AAC, ALAC, M4A...)"});
                            break;
                    }
                    bFomatFound = true;
                    if (bExtractCover) m_pix = extractMP4Cover(mp4File);
                }
            }
            if (!bFomatFound)
            {
                TagLib::FLAC::File *flacFile = dynamic_cast<TagLib::FLAC::File *>(m_FileRef->file());
                {
                    if (flacFile)
                    {
                        qDebug() << "FLAC";
                        fields.append(Field{"Bits: ", QString::number(flacFile->audioProperties()->bitsPerSample())});
                        m_TagInfo.iBits = flacFile->audioProperties()->bitsPerSample();
                        // Se il cast ha successo, è un file FLAC.
                        // Qui puoi continuare a lavorare con 'flacFile' per accedere
                        // a proprietà specifiche del FLAC se necessario.
                        // info.append ("\n");
                        // info.append ("Format: FLAC");
                        fields.append(Field{"Format: ", "FLAC"});
                        m_TagInfo.sFormat = "FLAC";
                        bFomatFound = true;
                        if (bExtractCover) m_pix = extractFLACCover(flacFile);
                    }
                }
            }
            if (!bFomatFound)
            {
                TagLib::Ogg::Opus::File *opusFile = dynamic_cast<TagLib::Ogg::Opus::File *>(m_FileRef->file());
                if (opusFile)
                {
                    // Se il cast ha successo, è un file FLAC.
                    qDebug() << "Ogg Opus";
                    // Qui puoi continuare a lavorare con 'flacFile' per accedere
                    // a proprietà specifiche del FLAC se necessario.
                    // info.append ("\n");
                    // info.append ("Format: Ogg Opus");
                    fields.append(Field{"Format: ", "Ogg Opus"});
                    m_TagInfo.sFormat = "Ogg Opus";
                    bFomatFound = true;
                }
            }
            if (!bFomatFound)
            {
                TagLib::Ogg::Vorbis::File *oggFile = dynamic_cast<TagLib::Ogg::Vorbis::File *>(m_FileRef->file());
                if (oggFile)
                {
                    // Se il cast ha successo, è un file FLAC.
                    qDebug() << "Ogg Vorbis";
                    // Qui puoi continuare a lavorare con 'flacFile' per accedere
                    // a proprietà specifiche del FLAC se necessario.
                    // info.append ("\n");
                    // info.append ("Format: Ogg Vorbis");
                    fields.append(Field{"Format: ", "Ogg Vorbis"});
                    m_TagInfo.sFormat = "Ogg Vorbis";
                    bFomatFound = true;
                }
            }
            if (!bFomatFound)
            {
                TagLib::Ogg::FLAC::File *oggFlacFile = dynamic_cast<TagLib::Ogg::FLAC::File *>(m_FileRef->file());
                if (oggFlacFile)
                {
                    // Se il cast ha successo, è un file FLAC.
                    qDebug() << "Ogg FLAC";
                    // Qui puoi continuare a lavorare con 'flacFile' per accedere
                    // a proprietà specifiche del FLAC se necessario.
                    // info.append ("\n");
                    // info.append ("Format: Ogg FLAC");
                    fields.append(Field{"Format: ", "Ogg FLAC"});
                    m_TagInfo.sFormat = "Ogg FLAC";
                    bFomatFound = true;
                }
            }
            if (!bFomatFound)
            {
                TagLib::Ogg::Speex::File *speexFile = dynamic_cast<TagLib::Ogg::Speex::File *>(m_FileRef->file());
                if (speexFile)
                {
                    // Se il cast ha successo, è un file FLAC.
                    qDebug() << "Ogg Speex";
                    // Qui puoi continuare a lavorare con 'flacFile' per accedere
                    // a proprietà specifiche del FLAC se necessario.
                    // info.append ("\n");
                    // info.append ("Format: Ogg Speex");
                    fields.append(Field{"Format: ", "Ogg Speex"});
                    m_TagInfo.sFormat = "Ogg Speex";
                    bFomatFound = true;
                }
            }
            if (!bFomatFound)
            {
                TagLib::RIFF::WAV::File *wavFile = dynamic_cast<TagLib::RIFF::WAV::File *>(m_FileRef->file());
                if (wavFile)
                {
                    qDebug() << "WAV (RIFF)";
                    // info.append ("\n");
                    // info.append ("Bits: ");
                    // info.append (QString::number (wavFile->audioProperties ()->bitsPerSample()));
                    fields.append(Field{"Bits: ", QString::number(wavFile->audioProperties()->bitsPerSample())});
                    m_TagInfo.iBits = wavFile->audioProperties()->bitsPerSample();
                    int iFormat = wavFile->audioProperties()->format();
                    // Se il cast ha successo, è un file WAV.
                    // info.append ("\n");
                    // info.append ("Format: ");
                    // if (iFormat == 1) info.append ("PCM");
                    // else if (iFormat == 0) info.append ("Unknown format (0)");
                    // else if (iFormat == 2) info.append ("Compressed ADPCM");
                    // else if (iFormat == 3) info.append ("IEEE float");
                    // else if (iFormat == 6) info.append ("A-law");
                    // else if (iFormat == 7) info.append ("µ-law");
                    // else info.append ("Unknown fomat");
                    // info.append (" WAV (RIFF)");
                    if (iFormat == 1)
                    {
                        fields.append(Field{"Format: ", "PCM WAV (RIFF)"});
                        m_TagInfo.sFormat = "PCM WAV (RIFF)";
                    }
                    else if (iFormat == 0)
                    {
                        fields.append(Field{"Format: ", "Unknown format (0) WAV (RIFF)"});
                        m_TagInfo.sFormat = "Unknown format (0) WAV (RIFF)";
                    }
                    else if (iFormat == 2)
                    {
                        fields.append(Field{"Format: ", "Compressed ADPCM WAV (RIFF)"});
                        m_TagInfo.sFormat = "Compressed ADPCM WAV (RIFF)";
                    }
                    else if (iFormat == 3)
                    {
                        fields.append(Field{"Format: ", "IEEE float WAV (RIFF)"});
                        m_TagInfo.sFormat = "IEEE float WAV (RIFF)";
                    }
                    else if (iFormat == 6)
                    {
                        fields.append(Field{"Format: ", "A-law WAV (RIFF)"});
                        m_TagInfo.sFormat = "A-law WAV (RIFF)";
                    }
                    else if (iFormat == 7)
                    {
                        fields.append(Field{"Format: ", "µ-law WAV (RIFF)"});
                        m_TagInfo.sFormat = "µ-law WAV (RIFF)";
                    }
                    else
                    {
                        fields.append(Field{"Format: ", "Unknown format WAV (RIFF)"});
                        m_TagInfo.sFormat = "Unknown format WAV (RIFF)";
                    }
                    bFomatFound = true;
                }
            }
            if (!m_pix.isNull())
            {
                QSize originalSize = m_pix.size();
                int originalWidth = originalSize.width();
                int originalHeight = originalSize.height();
                m_sPixSize = QString::number(originalWidth) + "x" + QString::number(originalHeight) ;
                //QString tooltipText = QString("Original size: <b>%1x%2</b> pixels") .arg(originalWidth).arg(originalHeight);
                fields.append(Field{"Cover size: ", m_sPixSize + " pixels"});
                m_TagInfo.sCoverSize = m_sPixSize;
            }
            QFile file(localFile);
            if (file.exists())
            {
                qint64 size = file.size();
                // info.append ("\n");
                // info.append ("File size: ");
                // info.append (formatFileSize (size));
                fields.append(Field{"File size: ", formatFileSize(size)});
                m_TagInfo.iFileSize = size;
            }
            else
            {
                qDebug() << "File does not exist.";
            }
            // Append all valid fields separated by newlines
            for (int i = 0; i < fields.size(); ++i)
            {
                if (i > 0) info.append("\n");
                info.append(fields[i].label);
                info.append(fields[i].value);
            }
            m_FileRef->file()->clear();
            delete m_FileRef;
            m_FileRef = nullptr;
            m_Info = info;
            setInfo(info);
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
            // AudioCover coverHelper;
            // m_pix = coverHelper.extractCover (nativePath);
            if (!m_pix.isNull())
            {
                // QPixmap scaledCover = cover.scaled(
                // ui->coverLabel->size(),          // QLabel size
                // Qt::KeepAspectRatio,             // maintain aspect ratio
                // Qt::SmoothTransformation         // smooth scaling
                // );
                // ui->labelPix->show ();
                //ui->labelPix->setPixmap(m_pix.scaled(ui->labelPix->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
            else
            {
                // ui->labelPix->setText("No cover found");
                // ui->labelPix->hide ();
            }
        }
        else
        {
            qDebug() << "Failed to read metadata!";
            setInfo("Failed to read metadata!");
        }
    }
    else
    {
        qDebug() << "File path is empty!";
    }
}

QPixmap AudioTag::extractCover(const QString &filePath)
{
    // QFileInfo info(filePath);
    // QString ext = info.suffix().toLower();
    // if (ext == "mp3")
    // return extractMP3Cover(filePath);
    // else if (ext == "flac")
    // return extractFLACCover(filePath);
    // else if (ext == "m4a" || ext == "mp4" || ext == "aac")
    // return extractMP4Cover(filePath);
    // else if (ext == "ape")
    // return extractAPECover(filePath);
    // else if (ext == "wma")
    // return extractWMACover(filePath);
    return QPixmap(); // empty if no supported format
}

QPixmap AudioTag::extractFLACCover(TagLib::FLAC::File *file)
{
    // TagLib::FLAC::File file(filePath.toStdString().c_str());
    // if (!file.isValid()) return QPixmap();
    // const TagLib::List<TagLib::FLAC::Picture *> &pics = file.pictureList();
    // if (pics.isEmpty()) return QPixmap();
    // TagLib::FLAC::Picture *pic = pics.front();
    // QByteArray data(pic->data().data(), pic->data().size());
    // QPixmap pix;
    // pix.loadFromData(data);
    // return pix;
    //TagLib::FLAC::File file(filePath.toStdString().c_str());
    if (!file->isValid()) return QPixmap();  // early return if invalid
    auto pics = file->pictureList();
    if (pics.isEmpty()) return QPixmap();  // no pictures
    const TagLib::FLAC::Picture *pic = pics.front();
    const TagLib::ByteVector &bv = pic->data();
    QPixmap pix;
    pix.loadFromData(reinterpret_cast<const uchar*>(bv.data()), bv.size());
    return pix;
}

QPixmap AudioTag::extractMP4Cover(TagLib::MP4::File *file)
{
    // qDebug() << __PRETTY_FUNCTION__;
    //        // TagLib::FileRef f(TagLib::FileName(filePath.toUtf8().constData()));
    //        // qDebug() << "f is null:" << f.isNull () << "->" << filePath;
    //        // TagLib::MP4::File *file = dynamic_cast<TagLib::MP4::File *>(f.file());
    //        // TagLib::MP4::File file(filePath.toStdString().c_str());
    // TagLib::MP4::File file(filePath.toUtf8 ().constData ());
    // qDebug() << "Valid:" << file.isValid() << "->" << filePath;
    // if (!file.isValid()) return QPixmap();
    // TagLib::MP4::Tag *tag = file.tag();
    // if (!tag)
    // {
    // qDebug() << "!tag";
    // return QPixmap();
    // }
    // const TagLib::MP4::ItemMap &items = tag->itemMap();
    // if (!items.contains("covr"))
    // {
    // qDebug() << "!items.contains(\"covr\")";
    // return QPixmap();
    // }
    // qDebug() << "Has tag:" << (tag != nullptr);
    // qDebug() << "Contains covr:" << tag->itemMap().contains("covr");
    // TagLib::MP4::CoverArtList coverList = items["covr"].toCoverArtList();
    // if (coverList.isEmpty()) return QPixmap();
    // const TagLib::MP4::CoverArt &cover = coverList.front();
    // QByteArray data(cover.data().data(), cover.data().size());
    // QPixmap pix;
    // pix.loadFromData(data);
    // return pix;
    //TagLib::MP4::File file(filePath.toUtf8().constData());
    if (!file->isValid()) return QPixmap();  // early return if invalid
    TagLib::MP4::Tag *tag = file->tag();
    if (!tag) return QPixmap();  // no tag
    const auto &items = tag->itemMap();
    if (!items.contains("covr")) return QPixmap();  // no cover art
    auto coverList = items["covr"].toCoverArtList();
    if (coverList.isEmpty()) return QPixmap();  // empty list
    const TagLib::MP4::CoverArt &cover = coverList.front();
    const TagLib::ByteVector &bv = cover.data();
    QPixmap pix;
    pix.loadFromData(reinterpret_cast<const uchar*>(bv.data()), bv.size());
    return pix;
}

QPixmap AudioTag::extractMP3Cover(TagLib::MPEG::File *mp3File)
{
    //TagLib::MPEG::File mp3File(filePath.toStdString().c_str());
    // TagLib::FileRef f(TagLib::FileName(localFile.toUtf8().constData()));
    // TagLib::FileRef file(TagLib::FileName(filePath.toUtf8 ().constData ()));
    //TagLib::FileName file(filePath.toUtf8 ().constData ());
    // QPixmap pixmap;
    // TagLib::MPEG::File *mp3File = dynamic_cast<TagLib::MPEG::File*>(file.file ());
    if (!mp3File->isValid())
        return QPixmap();
    TagLib::ID3v2::Tag *tag = mp3File->ID3v2Tag();
    if (!tag)
        return QPixmap();
    const auto pictures = tag->complexProperties("PICTURE");
    for (const auto &propMap : pictures)
    {
        if (!propMap.contains("data")) continue;
        const TagLib::ByteVector &bv = propMap["data"].toByteVector();
        QPixmap pix;
        if (pix.loadFromData(reinterpret_cast<const uchar * >(bv.data()), bv.size()))
            return pix;  // return first valid cover
    }
    //qDebug() << "AudioCover: No MP3 cover found.";
    return QPixmap();  // empty if no cover
}

/* ================= APE ================= */
//QPixmap AudioTag::extractAPECover(const QString &filePath)
//{
//    // TagLib::APE::File file(filePath.toStdString().c_str());
//    // if (!file.isValid()) return QPixmap();
//    // TagLib::APE::Tag *tag = file.APETag();
//    // if (!tag) return QPixmap();
//    // TagLib::APE::ItemListMap items = tag->itemListMap();
//    // if (!items.contains("COVER ART (FRONT)"))
//    // return QPixmap();
//    // TagLib::ByteVector data = items["COVER ART (FRONT)"].binaryData();
//    //    // Sometimes the data starts with a null-terminated filename
//    // int offset = data.find("\0") + 1;
//    // if (offset <= 0) offset = 0;
//    // QByteArray img(data.data() + offset, data.size() - offset);
//    // QPixmap pix;
//    // pix.loadFromData(img);
//    // return pix;
// TagLib::APE::File file(filePath.toStdString().c_str());
// if (!file.isValid()) return QPixmap();  // invalid file
// TagLib::APE::Tag *tag = file.APETag();
// if (!tag) return QPixmap();  // no APE tag
// const auto &items = tag->itemListMap();
// if (!items.contains("COVER ART (FRONT)")) return QPixmap();  // no cover
// TagLib::ByteVector data = items["COVER ART (FRONT)"].binaryData();
//    // Skip optional null-terminated filename at start
// int offset = data.find("\0") + 1;
// if (offset <= 0) offset = 0;
// const TagLib::ByteVector coverData = data.mid(offset);
// QPixmap pix;
// pix.loadFromData(reinterpret_cast<const uchar*>(coverData.data()), coverData.size());
// return pix;
//}

/* ================= WMA ================= */
QPixmap AudioTag::extractWMACover(const QString &filePath)
{
    // TagLib::WMA::File file(filePath.toStdString().c_str());
    // if (!file.isValid()) return QPixmap();
    // TagLib::ASF::Tag *tag = file.tag();
    // if (!tag) return QPixmap();
    // const TagLib::ASF::AttributeListMap &attrs = tag->attributeListMap();
    // if (!attrs.contains("WM/Picture"))
    // return QPixmap();
    // const TagLib::ASF::AttributeList &picList = attrs["WM/Picture"];
    // if (picList.isEmpty()) return QPixmap();
    // TagLib::ASF::Picture pic = picList.front().toPicture();
    // QByteArray data(pic.picture().data(), pic.picture().size());
    QPixmap pix;
    // pix.loadFromData(data);
    return pix;
}

AudioTagInfo AudioTag::tagInfo() const
{
    return m_TagInfo;
}

void AudioTag::resetTag()
{
    m_TagInfo.reset();
}

AudioTagInfo::AudioTagInfo()
{
    reset();
}

void AudioTagInfo::reset()
{
    // Resetta tutte le variabili QString a stringhe vuote
    sFileName = "";
    sBaseFileName = "";
    sExtension = "";
    sPath = "";
    sArtist = "";
    sTitle = "";
    sAlbum = "";
    sComment = "";
    sFormat = "";
    sCoverSize = "";
    sGenre = "";
    // Resetta tutte le variabili intere a -1
    iDuration = -1;
    iYear = -1;
    iBitrate = -1;
    iSamplerate = -1;
    iChannels = -1;
    iFileSize = -1;
    iTrackNum = -1;
    iBits = -1;
}
