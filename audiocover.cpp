#include "audiocover.h"

#include <QByteArray>
#include <QFileInfo>

// TagLib includes
//#include <fileref.h>
//#include <tfile.h>
#include <mpegfile.h>
#include <id3v2tag.h>
//#include <id3v2frame.h>
#include <flacfile.h>
#include <flacpicture.h>
#include <mp4file.h>
#include <mp4tag.h>
#include <mp4coverart.h>
#include <apefile.h>
#include <apetag.h>
//#include <wmafile.h>
#include <asftag.h>
#include <QDebug>
//SingleInstance::SingleInstance(const QString &serverName, QObject *parent)
// : QObject(parent),

AudioCover::AudioCover(QObject *parent)
    : QObject(parent)
{
}

QPixmap AudioCover::extractCover(const QString &filePath)
{
    QFileInfo info(filePath);
    QString ext = info.suffix().toLower();
    if (ext == "mp3")
        return extractMP3Cover(filePath);
    else if (ext == "flac")
        return extractFLACCover(filePath);
    else if (ext == "m4a" || ext == "mp4" || ext == "aac")
        return extractMP4Cover(filePath);
    else if (ext == "ape")
        return extractAPECover(filePath);
    else if (ext == "wma")
        return extractWMACover(filePath);
    return QPixmap(); // empty if no supported format
}

/* ================= MP3 (ID3v2 APIC) ================= */
QPixmap AudioCover::extractMP3Cover(const QString &filePath)
{
    TagLib::MPEG::File mp3File(filePath.toStdString().c_str());
    // TagLib::FileRef f(TagLib::FileName(localFile.toUtf8().constData()));
    // TagLib::FileRef file(TagLib::FileName(filePath.toUtf8 ().constData ()));
    //TagLib::FileName file(filePath.toUtf8 ().constData ());
    // QPixmap pixmap;
    // TagLib::MPEG::File *mp3File = dynamic_cast<TagLib::MPEG::File*>(file.file ());
    if ( !mp3File.isValid())
        return QPixmap();
    TagLib::ID3v2::Tag *tag = mp3File.ID3v2Tag();
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
    qDebug() << "AudioCover: No MP3 cover found.";
    return QPixmap();  // empty if no cover
}

/* ================= FLAC ================= */
QPixmap AudioCover::extractFLACCover(const QString &filePath)
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
    TagLib::FLAC::File file(filePath.toStdString().c_str());
    if (!file.isValid()) return QPixmap();  // early return if invalid
    auto pics = file.pictureList();
    if (pics.isEmpty()) return QPixmap();  // no pictures
    const TagLib::FLAC::Picture *pic = pics.front();
    const TagLib::ByteVector &bv = pic->data();
    QPixmap pix;
    pix.loadFromData(reinterpret_cast<const uchar*>(bv.data()), bv.size());
    return pix;
}

/* ================= MP4 (m4a, mp4) ================= */
QPixmap AudioCover::extractMP4Cover(const QString &filePath)
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
    TagLib::MP4::File file(filePath.toUtf8().constData());
    if (!file.isValid()) return QPixmap();  // early return if invalid
    TagLib::MP4::Tag *tag = file.tag();
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

/* ================= APE ================= */
QPixmap AudioCover::extractAPECover(const QString &filePath)
{
    // TagLib::APE::File file(filePath.toStdString().c_str());
    // if (!file.isValid()) return QPixmap();
    // TagLib::APE::Tag *tag = file.APETag();
    // if (!tag) return QPixmap();
    // TagLib::APE::ItemListMap items = tag->itemListMap();
    // if (!items.contains("COVER ART (FRONT)"))
    // return QPixmap();
    // TagLib::ByteVector data = items["COVER ART (FRONT)"].binaryData();
    //    // Sometimes the data starts with a null-terminated filename
    // int offset = data.find("\0") + 1;
    // if (offset <= 0) offset = 0;
    // QByteArray img(data.data() + offset, data.size() - offset);
    // QPixmap pix;
    // pix.loadFromData(img);
    // return pix;
    TagLib::APE::File file(filePath.toStdString().c_str());
    if (!file.isValid()) return QPixmap();  // invalid file
    TagLib::APE::Tag *tag = file.APETag();
    if (!tag) return QPixmap();  // no APE tag
    const auto &items = tag->itemListMap();
    if (!items.contains("COVER ART (FRONT)")) return QPixmap();  // no cover
    TagLib::ByteVector data = items["COVER ART (FRONT)"].binaryData();
    // Skip optional null-terminated filename at start
    int offset = data.find("\0") + 1;
    if (offset <= 0) offset = 0;
    const TagLib::ByteVector coverData = data.mid(offset);
    QPixmap pix;
    pix.loadFromData(reinterpret_cast<const uchar*>(coverData.data()), coverData.size());
    return pix;
}

/* ================= WMA ================= */
QPixmap AudioCover::extractWMACover(const QString &filePath)
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
