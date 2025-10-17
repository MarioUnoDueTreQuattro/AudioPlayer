#ifndef TAGLOADERWORKER_H
#define TAGLOADERWORKER_H

#include <QObject>
#include "audiotag.h"

class TagLoaderWorker : public QObject
{
    Q_OBJECT

public:
    //explicit TagLoaderWorker(QObject* parent = nullptr);
     TagLoaderWorker(QObject* parent = nullptr)
        : QObject(parent), m_bStop(false) {}

    void stop() { m_bStop = true; }
    void processFiles(const QStringList& fileList);
signals:
    void finished();
    void tagLoaded(const QString& filePath, const AudioTagInfo& info);
private:
    std::atomic<bool> m_bStop;
    };

#endif // TAGLOADERWORKER_H
