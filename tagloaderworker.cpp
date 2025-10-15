#include "tagloaderworker.h"
#include <QDebug>

TagLoaderWorker::TagLoaderWorker(QObject *parent)
{
}

void TagLoaderWorker::processFiles(const QStringList& fileList)
{
    AudioTag tag;
    for (const QString& filePath : fileList)
    {
        tag.resetTag();
        tag.setFile(filePath); // Your actual tag loading logic
        AudioTagInfo info = tag.tagInfo();
        emit tagLoaded(filePath, info);
        qDebug() << "Processed:" << filePath;
    }
    emit finished();
}
