#include "tagloaderworker.h"
#include <QDebug>

TagLoaderWorker::TagLoaderWorker(QObject *parent)
{
    m_bStop = false;
}

void TagLoaderWorker::processFiles(const QStringList& fileList)
{
    if (m_bStop)
    {
        emit finished();
        return;
    }
    AudioTag tag;
    for (const QString& filePath : fileList)
    {
        tag.resetTag();
        tag.setFile(filePath); // Your actual tag loading logic
        AudioTagInfo info = tag.tagInfo();
        emit tagLoaded(filePath, info);
        //qDebug() << "Processed:" << filePath;
    }
    emit finished();
}
