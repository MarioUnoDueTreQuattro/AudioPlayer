#ifndef AUDIOCOVER_H
#define AUDIOCOVER_H

#include <QString>
#include <QPixmap>
#include <QObject>

class AudioCover :public QObject
{
    Q_OBJECT

public:
    AudioCover( QObject *parent = nullptr);
//    SingleInstance(const QString &serverName, QObject *parent = nullptr);
    QPixmap extractCover(const QString &filePath);

private:
    QPixmap extractMP3Cover(const QString &filePath);
    QPixmap extractFLACCover(const QString &filePath);
    QPixmap extractMP4Cover(const QString &filePath);
    QPixmap extractAPECover(const QString &filePath);
    QPixmap extractWMACover(const QString &filePath);
};

#endif // AUDIOCOVER_H
