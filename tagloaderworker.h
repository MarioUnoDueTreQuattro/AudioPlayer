#ifndef TAGLOADERWORKER_H
#define TAGLOADERWORKER_H

#include <QObject>
#include "audiotag.h"

class TagLoaderWorker : public QObject
{
    Q_OBJECT

public:
    explicit TagLoaderWorker(QObject* parent = nullptr);
    void processFiles(const QStringList& fileList);
bool m_bStop;
signals:
    void finished();
    void tagLoaded(const QString& filePath, const AudioTagInfo& info);
};

#endif // TAGLOADERWORKER_H
