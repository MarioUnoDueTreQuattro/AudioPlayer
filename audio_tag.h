#ifndef AUDIOTAG_H
#define AUDIOTAG_H

#include <QString>
#include <QPixmap>
#include <QStandardItem>
#include <QObject>
#include <fileref.h>
#include <flacfile.h>
#include <mp4file.h>
#include <mpegfile.h>
#include <mpegproperties.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <opusfile.h>
#include <speexfile.h>
#include <tag.h>
#include <vorbisfile.h>
#include <wavfile.h>
#include <wavproperties.h>
//#include <mp4properties.h>
//#include <tpropertymap.h>

class AudioTagInfo
{
public:
    AudioTagInfo();
    void reset();
    QString sFileName;
    QString sBaseFileName;
    QString sExtension;
    QString sPath;
    int iDuration;
    QString sArtist;
    QString sTitle;
    QString sAlbum;
    QString sGenre;
    int iTrackNum;
    int iYear;
    QString sComment;
    int iBitrate;
    int iSamplerate;
    int iChannels;
    QString sFormat;
    QString sCoverSize;
    int iFileSize;
    int iBits;
    int iRating;
    int iPlayCount;
    int iLastModified;
    QList<QStandardItem *> toStandardItems() const;
};

Q_DECLARE_METATYPE(AudioTagInfo)

class AudioTag : public QObject
{
    Q_OBJECT

public:
    AudioTag(QObject *parent = nullptr);
    AudioTag(QString sFileName, QObject *parent = nullptr);
    QPixmap extractCover(const QString &filePath);
    void setFile(const QString &localFile, bool bExtractCover = true);
//    QString formatFileSize(qint64 bytes);
    void setInfo(QString);
    AudioTagInfo tagInfo() const;
    void resetTag();
private:
    //QString formatTime(int totalSeconds);
    QPixmap extractMP3Cover(TagLib::MPEG::File *mp3File);
    QPixmap extractMP4Cover(TagLib::MP4::File *file);
    QPixmap extractFLACCover(TagLib::FLAC::File *file);
    // QPixmap extractAPECover(const QString &filePath);
    QPixmap extractWMACover(const QString &filePath);
    TagLib::FileRef *m_FileRef;
    QString m_Info;
    QPixmap m_pix;
    QString m_sPixSize;
    AudioTagInfo m_TagInfo;
    bool m_bEnableLog;
};

#endif // AUDIOTAG_H
