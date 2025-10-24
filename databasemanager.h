#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include "audiotag.h"

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    // Singleton access
    static DatabaseManager &instance()
    {
        static DatabaseManager instance;
        return instance;
    }

    // Delete copy/assign
    DatabaseManager(DatabaseManager const &) = delete;
    void operator=(DatabaseManager const &) = delete;

    // Database
    bool openDatabase(const QString &dbPath);
    void closeDatabase();

    // Tracks
    bool trackExists(const QString &fullFilePath);
    bool loadTrack(const QString &fullFilePath, AudioTagInfo &info);
    bool insertTrack(const QString &fullFilePath, const AudioTagInfo &info);
    bool updateTrack(const QString &fullFilePath, const AudioTagInfo &info);
    bool incrementPlayCount(const QString &fullFilePath);

    // Favorites
    bool addToFavorites(const QString &fullFilePath);
    bool removeFromFavorites(const QString &fullFilePath);
    QList<AudioTagInfo> favoriteTracks();

    // History
    bool addToHistory(const QString &fullFilePath, int playPosition = 0);
    QList<AudioTagInfo> recentHistory(int limit = 20);

    // Playlist session
    bool clearSessionPlaylist();
    bool saveSessionPlaylist(const QList<QString> &fullFilePaths);
    QList<AudioTagInfo> loadSessionPlaylist();

    // Statistics
    QList<AudioTagInfo> topPlayed(int limit = 10);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool createTables();

    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
