#include "tag_loader_worker.h"
#include <QDebug>

//TagLoaderWorker::TagLoaderWorker(QObject *parent)
//{
//    m_bStop = false;
//}

void TagLoaderWorker::processFiles(const QStringList& fileList)
{
    AudioTag tag;
    for (const QString& filePath : fileList)
    {
        if (m_bStop) // check if we should abort
            break;

        tag.resetTag();
        tag.setFile(filePath);
        AudioTagInfo info = tag.tagInfo();
        emit tagLoaded(filePath, info);
    }
    emit finished();
}
