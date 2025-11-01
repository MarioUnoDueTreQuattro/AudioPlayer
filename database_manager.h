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
#include "audio_tag.h"

/**
 * @class DatabaseManager
 * @brief Singleton class that manages the SQLite database for audio tracks, playlists, favorites, history, and statistics.
 *
 * DatabaseManager provides a central interface to:
 * - Cache and retrieve audio tag information
 * - Manage playlists and session playback order
 * - Track user favorites and playback history
 * - Maintain statistics such as play counts and top tracks
 *
 * The class uses SQLite via Qt SQL module and ensures that tables are created
 * automatically on first run.
 */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Returns the singleton instance of the DatabaseManager.
     *
     * Ensures that only one instance exists during the lifetime of the application.
     * @return Reference to the singleton DatabaseManager.
     */
    static DatabaseManager &instance()
    {
        static DatabaseManager instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to enforce singleton
    DatabaseManager(DatabaseManager const &) = delete;
    void operator=(DatabaseManager const &) = delete;

    /**
     * @brief Opens (or creates) the SQLite database file.
     *
     * If the database file does not exist, it is created automatically.
     * All necessary tables are created if missing.
     *
     * @param dbPath Path to the SQLite database file.
     * @return true if the database was opened successfully, false otherwise.
     */
    bool openDatabase(const QString &dbPath);

    /**
     * @brief Closes the database connection.
     */
    void closeDatabase();

    // =======================
    // Track management
    // =======================

    /**
     * @brief Checks if a track already exists in the database.
     * @param fullFilePath Absolute file path of the track.
     * @return true if the track exists, false otherwise.
     */
    bool trackExists(const QString &fullFilePath);

    /**
     * @brief Loads track information from the database.
     * @param fullFilePath Absolute path to the audio file.
     * @param info Reference to an AudioTagInfo object to populate.
     * @return true if the track exists and was loaded successfully.
     */
    bool loadTrack(const QString &fullFilePath, AudioTagInfo &info);

    /**
     * @brief Inserts a new track into the database.
     * @param fullFilePath Absolute path to the audio file.
     * @param info AudioTagInfo containing tag metadata.
     * @return true if insertion succeeded.
     */
    bool insertTrack(const QString &fullFilePath, const AudioTagInfo &info);

    /**
     * @brief Updates an existing track in the database.
     * @param fullFilePath Absolute path to the audio file.
     * @param info AudioTagInfo containing updated tag metadata.
     * @return true if update succeeded.
     */
    bool updateTrack(const QString &fullFilePath, const AudioTagInfo &info);

    /**
     * @brief Increments the play count of a track by 1.
     * @param fullFilePath Absolute path to the audio file.
     * @return true if successful.
     */
    bool incrementPlayCount(const QString &fullFilePath);
    bool setRating(const QString &fullFilePath, int iRating);

    /**
     * @brief Loads or updates a track from the database or the file system.
     *
     * Loads the tag information for a track from the database if it exists,
     * or inserts it if it does not exist. Additionally, if the file has been
     * modified since the last read, it updates the database with the new tag info.
     *
     * @param fullFilePath Absolute path to the audio file.
     * @param info Reference to an AudioTagInfo object to populate.
     * @return true if the operation was successful.
     *
     * @note Can be used in QtConcurrent::run() to avoid blocking the GUI.
     */
    bool loadOrUpdateTrack(const QString &fullFilePath, AudioTagInfo &info);

    // =======================
    // Favorites management
    // =======================

    /**
     * @brief Adds a track to the user's favorites.
     * @param fullFilePath Absolute path to the audio file.
     * @return true if successful.
     */
    bool addToFavorites(const QString &fullFilePath);

    /**
     * @brief Removes a track from the user's favorites.
     * @param fullFilePath Absolute path to the audio file.
     * @return true if successful.
     */
    bool removeFromFavorites(const QString &fullFilePath);

    /**
     * @brief Retrieves the list of favorite tracks.
     * @return List of AudioTagInfo objects in the favorites.
     */
    QList<AudioTagInfo> favoriteTracks();
    QList<AudioTagInfo> historyTracks();

    // =======================
    // History management
    // =======================

    /**
     * @brief Adds a track to the playback history.
     * @param fullFilePath Absolute path to the audio file.
     * @param playPosition Playback position in milliseconds (default: 0).
     * @return true if successful.
     */
    bool addToHistory(const QString &fullFilePath, int playPosition = 0);

    /**
     * @brief Retrieves the most recently played tracks.
     * @param limit Maximum number of tracks to return (default: 20).
     * @return List of AudioTagInfo objects.
     */
    QList<AudioTagInfo> recentHistory(int limit = 20);

    // =======================
    // Playlist session management
    // =======================

    /**
     * @brief Clears the current session playlist in the database.
     * @return true if successful.
     */
    bool clearSessionPlaylist();

    /**
     * @brief Saves the current session playlist to the database.
     * @param fullFilePaths List of absolute file paths in playlist order.
     * @return true if successful.
     */
    bool saveSessionPlaylist(const QList<QString> &fullFilePaths);

    /**
     * @brief Loads the session playlist from the database in saved order.
     * @return List of AudioTagInfo objects in playlist order.
     */
    QList<AudioTagInfo> loadSessionPlaylist();

    // =======================
    // Statistics
    // =======================

    /**
     * @brief Returns the top played tracks ordered by play count descending.
     * @param limit Maximum number of tracks to return (default: 10).
     * @return List of AudioTagInfo objects.
     */
    QList<AudioTagInfo> topPlayed(int limit = 10);

    bool isTrackInFavorites(int trackId) const;
    int getTrackId(const QString &fullFilePath) const;
    bool deleteInexistentFiles();
    bool resetAllPlayCounts();
private:
    /**
     * @brief Private constructor for singleton.
     * @param parent Optional parent QObject.
     */
    explicit DatabaseManager(QObject *parent = nullptr);

    /**
     * @brief Destructor closes the database.
     */
    ~DatabaseManager();

    /**
     * @brief Creates all necessary tables if they do not exist.
     * @return true if all tables were created or already exist.
     */
    bool createTables();

private:
    QSqlDatabase m_db; /**< SQLite database connection */
};

#endif // DATABASEMANAGER_H
