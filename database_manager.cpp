#include "database_manager.h"

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

bool DatabaseManager::openDatabase(const QString &dbPath)
{
    if (QSqlDatabase::contains("audio_connection"))
        m_db = QSqlDatabase::database("audio_connection");
    else
        m_db = QSqlDatabase::addDatabase("QSQLITE", "audio_connection");
    m_db.setDatabaseName(dbPath);
    if (!m_db.open())
    {
        qDebug() << "Database open error:" << m_db.lastError().text();
        return false;
    }
    return createTables();
}

void DatabaseManager::closeDatabase()
{
    if (m_db.isOpen())
        m_db.close();
}

//bool DatabaseManager::createTables()
//{
// QSqlQuery query(m_db);
//    // Tracks
// QString tracksSql = QStringLiteral(
// "CREATE TABLE IF NOT EXISTS Tracks ("
// "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
// "FullFilePath TEXT UNIQUE NOT NULL,"
////            "FileName TEXT,"
// "BaseFileName TEXT,"
// "Extension TEXT,"
// "Path TEXT,"
// "Duration INTEGER,"
// "Artist TEXT,"
// "Title TEXT,"
// "Album TEXT,"
// "Genre TEXT,"
// "TrackNum INTEGER,"
// "Year INTEGER,"
// "Comment TEXT,"
// "Bitrate INTEGER,"
// "Samplerate INTEGER,"
// "Channels INTEGER,"
// "Bits INTEGER,"
// "Format TEXT,"
// "CoverSize TEXT,"
// "FileSize INTEGER,"
// "LastModified INTEGER,"
// "Rating INTEGER DEFAULT 0,"
// "PlayCount INTEGER DEFAULT 0);"
// );
// if (!query.exec(tracksSql))
// {
// qDebug() << "Error creating Tracks:" << query.lastError().text();
// return false;
// }
//    // Playlists
// QString playlistsSql = QStringLiteral(
// "CREATE TABLE IF NOT EXISTS Playlists ("
// "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
// "Name TEXT UNIQUE NOT NULL,"
// "Created DATETIME DEFAULT CURRENT_TIMESTAMP,"
// "PlayCount INTEGER DEFAULT 0);"
// );
// if (!query.exec(playlistsSql))
// {
// qDebug() << "Error creating Playlists:" << query.lastError().text();
// return false;
// }
//    // PlaylistItems
// QString playlistItemsSql = QStringLiteral(
// "CREATE TABLE IF NOT EXISTS PlaylistItems ("
// "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
// "PlaylistId INTEGER NOT NULL,"
// "TrackId INTEGER NOT NULL,"
// "Position INTEGER,"
// "FOREIGN KEY (PlaylistId) REFERENCES Playlists(Id),"
// "FOREIGN KEY (TrackId) REFERENCES Tracks(Id));"
// );
// if (!query.exec(playlistItemsSql))
// {
// qDebug() << "Error creating PlaylistItems:" << query.lastError().text();
// return false;
// }
//    // History
// QString historySql = QStringLiteral(
// "CREATE TABLE IF NOT EXISTS History ("
// "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
// "TrackId INTEGER NOT NULL,"
// "PlayDate INTEGER,"
// "PlayPosition INTEGER,"
// "FOREIGN KEY (TrackId) REFERENCES Tracks(Id));"
// );
// if (!query.exec(historySql))
// {
// qDebug() << "Error creating History:" << query.lastError().text();
// return false;
// }
//    // Favorites
// QString favoritesSql = QStringLiteral(
// "CREATE TABLE IF NOT EXISTS Favorites ("
// "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
// "TrackId INTEGER NOT NULL,"
// "DateAdded INTEGER,"
// "FOREIGN KEY (TrackId) REFERENCES Tracks(Id));"
// );
// if (!query.exec(favoritesSql))
// {
// qDebug() << "Error creating Favorites:" << query.lastError().text();
// return false;
// }
//    // SessionPlaylist
// QString sessionSql = QStringLiteral(
// "CREATE TABLE IF NOT EXISTS SessionPlaylist ("
// "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
// "TrackId INTEGER NOT NULL,"
// "Position INTEGER,"
// "FOREIGN KEY (TrackId) REFERENCES Tracks(Id));"
// );
// if (!query.exec(sessionSql))
// {
// qDebug() << "Error creating SessionPlaylist:" << query.lastError().text();
// return false;
// }
// return true;
//}

bool DatabaseManager::isTrackInFavorites(int trackId) const
{
    if (!m_db.isOpen())
        return false;

    QSqlQuery query(m_db);
    query.prepare("SELECT 1 FROM Favorites WHERE TrackId = ? LIMIT 1");
    query.addBindValue(trackId);

    if (!query.exec())
    {
        qWarning() << "DatabaseManager::isTrackInFavorites failed:" << query.lastError().text();
        return false;
    }

    return query.next(); // se c'è almeno una riga, il track è nei favorites
}

bool DatabaseManager::addToFavorites(const QString &fullFilePath)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT OR IGNORE INTO Favorites (TrackId, DateAdded) "
                  "SELECT Id, ? FROM Tracks WHERE FullFilePath = ?");
    query.addBindValue(QDateTime::currentSecsSinceEpoch());
    query.addBindValue(fullFilePath);
    return query.exec();
}

bool DatabaseManager::removeFromFavorites(const QString &fullFilePath)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM Favorites WHERE TrackId = (SELECT Id FROM Tracks WHERE FullFilePath = ?)");
    query.addBindValue(fullFilePath);
    return query.exec();
}

QList<AudioTagInfo> DatabaseManager::favoriteTracks()
{
    QList<AudioTagInfo> list;
    QSqlQuery query(m_db);
    query.prepare("SELECT T.* FROM Tracks T JOIN Favorites F ON T.Id = F.TrackId ORDER BY F.DateAdded DESC");
    if (!query.exec()) return list;
    while (query.next())
    {
        AudioTagInfo info;
        info.sFileName = query.value("FileName").toString();
        info.sBaseFileName = query.value("BaseFileName").toString();
        info.sExtension = query.value("Extension").toString();
        info.sPath = query.value("Path").toString();
        info.iDuration = query.value("Duration").toInt();
        info.sArtist = query.value("Artist").toString();
        info.sTitle = query.value("Title").toString();
        info.sAlbum = query.value("Album").toString();
        info.sGenre = query.value("Genre").toString();
        info.iTrackNum = query.value("TrackNum").toInt();
        info.iYear = query.value("Year").toInt();
        info.sComment = query.value("Comment").toString();
        info.iBitrate = query.value("Bitrate").toInt();
        info.iSamplerate = query.value("Samplerate").toInt();
        info.iChannels = query.value("Channels").toInt();
        info.iBits = query.value("Bits").toInt();
        info.sFormat = query.value("Format").toString();
        info.sCoverSize = query.value("CoverSize").toString();
        info.iFileSize = query.value("FileSize").toInt();
        info.iRating = query.value("Rating").toInt();
        info.iPlayCount = query.value("PlayCount").toInt();
        list.append(info);
    }
    return list;
}

QList<AudioTagInfo> DatabaseManager::historyTracks()
{
    QList<AudioTagInfo> list;
    QSqlQuery query(m_db);
    query.prepare("SELECT T.* FROM Tracks T JOIN History F ON T.Id = F.TrackId ORDER BY F.PlayDate DESC");
    if (!query.exec()) return list;
    while (query.next())
    {
        AudioTagInfo info;
        info.sFileName = query.value("FileName").toString();
        info.sBaseFileName = query.value("BaseFileName").toString();
        info.sExtension = query.value("Extension").toString();
        info.sPath = query.value("Path").toString();
        info.iDuration = query.value("Duration").toInt();
        info.sArtist = query.value("Artist").toString();
        info.sTitle = query.value("Title").toString();
        info.sAlbum = query.value("Album").toString();
        info.sGenre = query.value("Genre").toString();
        info.iTrackNum = query.value("TrackNum").toInt();
        info.iYear = query.value("Year").toInt();
        info.sComment = query.value("Comment").toString();
        info.iBitrate = query.value("Bitrate").toInt();
        info.iSamplerate = query.value("Samplerate").toInt();
        info.iChannels = query.value("Channels").toInt();
        info.iBits = query.value("Bits").toInt();
        info.sFormat = query.value("Format").toString();
        info.sCoverSize = query.value("CoverSize").toString();
        info.iFileSize = query.value("FileSize").toInt();
        info.iRating = query.value("Rating").toInt();
        info.iPlayCount = query.value("PlayCount").toInt();
        list.append(info);
    }
    return list;
}

//bool DatabaseManager::addToHistory(const QString &fullFilePath, int playPosition)
//{
// QSqlQuery query(m_db);
// query.prepare("INSERT INTO History (TrackId, PlayDate, PlayPosition) "
// "SELECT Id, ?, ? FROM Tracks WHERE FullFilePath = ?");
// query.addBindValue(QDateTime::currentSecsSinceEpoch());
// query.addBindValue(playPosition);
// query.addBindValue(fullFilePath);
// return query.exec();
//}

//bool DatabaseManager::addToHistory(const QString &fullFilePath, int playPosition)
//{
// if (!m_db.isOpen())
// {
// qWarning() << "DatabaseManager::addToHistory - database not open";
// return false;
// }

//    // Prepare query
// QSqlQuery query(m_db);
// query.prepare(
// "INSERT INTO History (TrackId, PlayDate, PlayPosition) "
// "SELECT Id, ?, ? FROM Tracks WHERE FullFilePath = ?"
// );

// const qint64 playDate = QDateTime::currentSecsSinceEpoch();
// query.addBindValue(playDate);      // PlayDate
// query.addBindValue(playPosition);  // PlayPosition (in seconds)
// query.addBindValue(fullFilePath);  // Track identification

// if (!query.exec())
// {
// qWarning() << "DatabaseManager::addToHistory failed:"
// << query.lastError().text()
// << "SQL:" << query.lastQuery()
// << "File:" << fullFilePath;
// return false;
// }

// if (query.numRowsAffected() == 0)
// {
// qWarning() << "DatabaseManager::addToHistory - no matching track for"
// << fullFilePath;
// return false;
// }

// return true;
//}

bool DatabaseManager::addToHistory(const QString &fullFilePath, int playPosition)
{
    if (!m_db.isOpen())
    {
        qWarning() << "DatabaseManager::addToHistory - database not open";
        return false;
    }
    // Prepare query: PlayDate as TEXT (DATETIME readable)
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO History (TrackId, PlayDate, PlayPosition) "
        "SELECT Id, ?, ? FROM Tracks WHERE FullFilePath = ?"
    );
    // Current date-time as ISO 8601 string: "YYYY-MM-DD HH:MM:SS"
    QString playDateStr = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    query.addBindValue(playDateStr);  // PlayDate as readable text
    query.addBindValue(playPosition);  // PlayPosition (in seconds)
    query.addBindValue(fullFilePath);  // Track identification
    if (!query.exec())
    {
        qWarning() << "DatabaseManager::addToHistory failed:"
            << query.lastError().text()
            << "SQL:" << query.lastQuery()
            << "File:" << fullFilePath;
        return false;
    }
    if (query.numRowsAffected() == 0)
    {
        qWarning() << "DatabaseManager::addToHistory - no matching track for"
            << fullFilePath;
        return false;
    }
    return true;
}

QList<AudioTagInfo> DatabaseManager::recentHistory(int limit)
{
    QList<AudioTagInfo> list;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT T.* FROM Tracks T JOIN History H ON T.Id = H.TrackId "
        "ORDER BY H.PlayDate DESC LIMIT %1").arg(limit)
    );
    if (!query.exec()) return list;
    while (query.next())
    {
        AudioTagInfo info;
        info.sFileName = query.value("FileName").toString();
        info.sBaseFileName = query.value("BaseFileName").toString();
        info.sExtension = query.value("Extension").toString();
        info.sPath = query.value("Path").toString();
        info.iDuration = query.value("Duration").toInt();
        info.sArtist = query.value("Artist").toString();
        info.sTitle = query.value("Title").toString();
        info.sAlbum = query.value("Album").toString();
        info.sGenre = query.value("Genre").toString();
        info.iTrackNum = query.value("TrackNum").toInt();
        info.iYear = query.value("Year").toInt();
        info.sComment = query.value("Comment").toString();
        info.iBitrate = query.value("Bitrate").toInt();
        info.iSamplerate = query.value("Samplerate").toInt();
        info.iChannels = query.value("Channels").toInt();
        info.iBits = query.value("Bits").toInt();
        info.sFormat = query.value("Format").toString();
        info.sCoverSize = query.value("CoverSize").toString();
        info.iFileSize = query.value("FileSize").toInt();
        info.iRating = query.value("Rating").toInt();
        info.iPlayCount = query.value("PlayCount").toInt();
        list.append(info);
    }
    return list;
}
bool DatabaseManager::clearSessionPlaylist()
{
    QSqlQuery query(m_db);
    return query.exec("DELETE FROM SessionPlaylist");
}

bool DatabaseManager::saveSessionPlaylist(const QList<QString> &fullFilePaths)
{
    if (!clearSessionPlaylist()) return false;
    QSqlQuery query(m_db);
    for (int pos = 0; pos < fullFilePaths.size(); ++pos)
    {
        query.prepare("INSERT INTO SessionPlaylist (TrackId, Position) "
                      "SELECT Id, ? FROM Tracks WHERE FullFilePath = ?");
        query.addBindValue(pos);
        query.addBindValue(fullFilePaths[pos]);
        if (!query.exec()) return false;
    }
    return true;
}

QList<AudioTagInfo> DatabaseManager::loadSessionPlaylist()
{
    QList<AudioTagInfo> list;
    QSqlQuery query(m_db);
    if (!query.exec(
        "SELECT T.* FROM Tracks T JOIN SessionPlaylist S ON T.Id = S.TrackId "
        "ORDER BY S.Position ASC")) return list;
    while (query.next())
    {
        AudioTagInfo info;
        info.sFileName = query.value("FileName").toString();
        info.sBaseFileName = query.value("BaseFileName").toString();
        info.sExtension = query.value("Extension").toString();
        info.sPath = query.value("Path").toString();
        info.iDuration = query.value("Duration").toInt();
        info.sArtist = query.value("Artist").toString();
        info.sTitle = query.value("Title").toString();
        info.sAlbum = query.value("Album").toString();
        info.sGenre = query.value("Genre").toString();
        info.iTrackNum = query.value("TrackNum").toInt();
        info.iYear = query.value("Year").toInt();
        info.sComment = query.value("Comment").toString();
        info.iBitrate = query.value("Bitrate").toInt();
        info.iSamplerate = query.value("Samplerate").toInt();
        info.iChannels = query.value("Channels").toInt();
        info.iBits = query.value("Bits").toInt();
        info.sFormat = query.value("Format").toString();
        info.sCoverSize = query.value("CoverSize").toString();
        info.iFileSize = query.value("FileSize").toInt();
        info.iRating = query.value("Rating").toInt();
        info.iPlayCount = query.value("PlayCount").toInt();
        list.append(info);
    }
    return list;
}
QList<AudioTagInfo> DatabaseManager::topPlayed(int limit)
{
    QList<AudioTagInfo> list;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT * FROM Tracks ORDER BY PlayCount DESC LIMIT %1").arg(limit));
    if (!query.exec()) return list;
    while (query.next())
    {
        AudioTagInfo info;
        info.sFileName = query.value("FileName").toString();
        info.sBaseFileName = query.value("BaseFileName").toString();
        info.sExtension = query.value("Extension").toString();
        info.sPath = query.value("Path").toString();
        info.iDuration = query.value("Duration").toInt();
        info.sArtist = query.value("Artist").toString();
        info.sTitle = query.value("Title").toString();
        info.sAlbum = query.value("Album").toString();
        info.sGenre = query.value("Genre").toString();
        info.iTrackNum = query.value("TrackNum").toInt();
        info.iYear = query.value("Year").toInt();
        info.sComment = query.value("Comment").toString();
        info.iBitrate = query.value("Bitrate").toInt();
        info.iSamplerate = query.value("Samplerate").toInt();
        info.iChannels = query.value("Channels").toInt();
        info.iBits = query.value("Bits").toInt();
        info.sFormat = query.value("Format").toString();
        info.sCoverSize = query.value("CoverSize").toString();
        info.iFileSize = query.value("FileSize").toInt();
        info.iRating = query.value("Rating").toInt();
        info.iPlayCount = query.value("PlayCount").toInt();
        list.append(info);
    }
    return list;
}
//bool DatabaseManager::trackExists(const QString &fullFilePath)
//{
// QSqlQuery query(m_db);
// query.prepare("SELECT COUNT(*) FROM Tracks WHERE FullFilePath = ?");
// query.addBindValue(fullFilePath);
// if (!query.exec()) return false;
// if (query.next()) return query.value(0).toInt() > 0;
// return false;
//}

//bool DatabaseManager::loadTrack(const QString &fullFilePath, AudioTagInfo &info)
//{
// QSqlQuery query(m_db);
// query.prepare("SELECT * FROM Tracks WHERE FullFilePath = ?");
// query.addBindValue(fullFilePath);
// if (!query.exec()) return false;
// if (!query.next()) return false;

// info.sFileName = query.value("FileName").toString();
// info.sBaseFileName = query.value("BaseFileName").toString();
// info.sExtension = query.value("Extension").toString();
// info.sPath = query.value("Path").toString();
// info.iDuration = query.value("Duration").toInt();
// info.sArtist = query.value("Artist").toString();
// info.sTitle = query.value("Title").toString();
// info.sAlbum = query.value("Album").toString();
// info.sGenre = query.value("Genre").toString();
// info.iTrackNum = query.value("TrackNum").toInt();
// info.iYear = query.value("Year").toInt();
// info.sComment = query.value("Comment").toString();
// info.iBitrate = query.value("Bitrate").toInt();
// info.iSamplerate = query.value("Samplerate").toInt();
// info.iChannels = query.value("Channels").toInt();
// info.iBits = query.value("Bits").toInt();
// info.sFormat = query.value("Format").toString();
// info.sCoverSize = query.value("CoverSize").toString();
// info.iFileSize = query.value("FileSize").toInt();
// info.iRating = query.value("Rating").toInt();
// info.iPlayCount = query.value("PlayCount").toInt();

// return true;
//}

//bool DatabaseManager::insertTrack(const QString &fullFilePath, const AudioTagInfo &info)
//{
// if (!m_db.isOpen()) {
// qDebug() << "Database not open";
// return false;
// }

// QFileInfo fi(fullFilePath);

// QSqlQuery query(m_db);
// query.prepare(
// "INSERT INTO Tracks ("
// "FullFilePath, BaseFileName, Extension, Path, Duration, "
// "Artist, Title, Album, Genre, TrackNum, Year, Comment, "
// "Bitrate, Samplerate, Bits, Channels, Format, CoverSize, "
// "FileSize, LastModified, Rating, PlayCount"
// ") VALUES ("
// "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
// );

//    // Full file information
// query.addBindValue(fullFilePath);
// query.addBindValue(fi.completeBaseName());   // BaseFileName
// query.addBindValue(fi.suffix());             // Extension
// query.addBindValue(fi.path());               // Path

//    // Tag info
// query.addBindValue(info.iDuration);
// query.addBindValue(info.sArtist);
// query.addBindValue(info.sTitle);
// query.addBindValue(info.sAlbum);
// query.addBindValue(info.sGenre);
// query.addBindValue(info.iTrackNum);
// query.addBindValue(info.iYear);
// query.addBindValue(info.sComment);

//    // Technical info
// query.addBindValue(info.iBitrate);
// query.addBindValue(info.iSamplerate);
// query.addBindValue(info.iBits);
// query.addBindValue(info.iChannels);
// query.addBindValue(info.sFormat);
// query.addBindValue(info.sCoverSize);
// query.addBindValue(info.iFileSize);
// query.addBindValue(fi.lastModified().toSecsSinceEpoch());

//    // Player data (default 0 if empty)
// query.addBindValue(info.iRating);
// query.addBindValue(info.iPlayCount);

// if (!query.exec()) {
// qDebug() << "Insert error:" << query.lastError().text();
// qDebug() << "Query:" << query.lastQuery();
// return false;
// }

// return true;
//}

bool DatabaseManager::incrementPlayCount(const QString &fullFilePath)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE Tracks SET PlayCount = PlayCount + 1 WHERE FullFilePath = ?");
    query.addBindValue(fullFilePath);
    return query.exec();
}

bool DatabaseManager::setRating(const QString &fullFilePath, int iRating)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE Tracks SET Rating = ? WHERE FullFilePath = ?");
    query.addBindValue(iRating);
    query.addBindValue(fullFilePath);
    return query.exec();
}

//bool DatabaseManager::loadOrUpdateTrack(const QString &fullFilePath, AudioTagInfo &info)
//{
// QFileInfo fi(fullFilePath);
// qint64 fileModified = fi.lastModified().toSecsSinceEpoch();

// if(trackExists(fullFilePath)) {
// loadTrack(fullFilePath, info);
//        // Controllo se il file è stato modificato dopo l'ultima lettura
// if(fileModified != info.iLastModified) {
// AudioTag tag(fullFilePath);
// info = tag.tagInfo();
// updateTrack(fullFilePath, info); // Aggiorna DB
// }
// return true;
// } else {
//        // Nuovo file
// AudioTag tag(fullFilePath);
// info = tag.tagInfo();
// insertTrack(fullFilePath, info);
// return true;
// }
//}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_db);
    // Tracks
    QString tracksSql = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS Tracks ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "FullFilePath TEXT UNIQUE NOT NULL,"
            "BaseFileName TEXT,"
            "Extension TEXT,"
            "Path TEXT,"
            "Duration INTEGER,"
            "Artist TEXT,"
            "Title TEXT,"
            "Album TEXT,"
            "Genre TEXT,"
            "TrackNum INTEGER,"
            "Year INTEGER,"
            "Comment TEXT,"
            "Bitrate INTEGER,"
            "Samplerate INTEGER,"
            "Channels INTEGER,"
            "Bits INTEGER,"
            "Format TEXT,"
            "CoverSize TEXT,"
            "FileSize INTEGER,"
            "LastModified INTEGER,"
            "Rating INTEGER DEFAULT 0,"
            "PlayCount INTEGER DEFAULT 0);"
        );
    if (!query.exec(tracksSql))
    {
        qDebug() << "Error creating Tracks:" << query.lastError().text();
        return false;
    }
    // Playlists
    QString playlistsSql = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS Playlists ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "Name TEXT UNIQUE NOT NULL,"
            "Created DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "PlayCount INTEGER DEFAULT 0);"
        );
    if (!query.exec(playlistsSql))
    {
        qDebug() << "Error creating Playlists:" << query.lastError().text();
        return false;
    }
    // PlaylistItems
    QString playlistItemsSql = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS PlaylistItems ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "PlaylistId INTEGER NOT NULL,"
            "TrackId INTEGER NOT NULL,"
            "Position INTEGER,"
            "FOREIGN KEY (PlaylistId) REFERENCES Playlists(Id),"
            "FOREIGN KEY (TrackId) REFERENCES Tracks(Id));"
        );
    if (!query.exec(playlistItemsSql))
    {
        qDebug() << "Error creating PlaylistItems:" << query.lastError().text();
        return false;
    }
    // History
    QString historySql = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS History ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "TrackId INTEGER NOT NULL,"
            "PlayDate TEXT,"           // <-- salvata come DATETIME leggibile
            "PlayPosition INTEGER,"
            "FOREIGN KEY (TrackId) REFERENCES Tracks(Id)"
            ");"
        );
    if (!query.exec(historySql))
    {
        qDebug() << "Error creating History:" << query.lastError().text();
        return false;
    }
    // Favorites
    QString favoritesSql = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS Favorites ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "TrackId INTEGER NOT NULL,"
            "DateAdded INTEGER,"
            "FOREIGN KEY (TrackId) REFERENCES Tracks(Id));"
        );
    if (!query.exec(favoritesSql))
    {
        qDebug() << "Error creating Favorites:" << query.lastError().text();
        return false;
    }
    // SessionPlaylist
    QString sessionSql = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS SessionPlaylist ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "TrackId INTEGER NOT NULL,"
            "Position INTEGER,"
            "FOREIGN KEY (TrackId) REFERENCES Tracks(Id));"
        );
    if (!query.exec(sessionSql))
    {
        qDebug() << "Error creating SessionPlaylist:" << query.lastError().text();
        return false;
    }
    return true;
}

int DatabaseManager::getTrackId(const QString &fullFilePath) const
{
    if (!m_db.isOpen())
    {
        qWarning() << "DatabaseManager::getTrackId - database not open";
        return -1;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT Id FROM Tracks WHERE FullFilePath = ?");
    query.addBindValue(fullFilePath);

    if (!query.exec())
    {
        qWarning() << "DatabaseManager::getTrackId failed:" << query.lastError().text()
                   << "File:" << fullFilePath;
        return -1;
    }

    if (query.next())
        return query.value(0).toInt();  // Id trovato

    return -1; // traccia non trovata
}

bool DatabaseManager::trackExists(const QString &fullFilePath)
{
    if (!m_db.isOpen())
        return false;
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM Tracks WHERE FullFilePath = ?");
    query.addBindValue(fullFilePath);
    if (!query.exec())
        return false;
    return (query.next() && query.value(0).toInt() > 0);
}

bool DatabaseManager::loadTrack(const QString &fullFilePath, AudioTagInfo &info)
{
    if (!m_db.isOpen())
        return false;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM Tracks WHERE FullFilePath = ?");
    query.addBindValue(fullFilePath);
    if (!query.exec() || !query.next())
        return false;
    info.sFileName = fullFilePath;
    info.sBaseFileName = query.value("BaseFileName").toString();
    info.sExtension = query.value("Extension").toString();
    info.sPath = query.value("Path").toString();
    info.iDuration = query.value("Duration").toInt();
    info.sArtist = query.value("Artist").toString();
    info.sTitle = query.value("Title").toString();
    info.sAlbum = query.value("Album").toString();
    info.sGenre = query.value("Genre").toString();
    info.iTrackNum = query.value("TrackNum").toInt();
    info.iYear = query.value("Year").toInt();
    info.sComment = query.value("Comment").toString();
    info.iBitrate = query.value("Bitrate").toInt();
    info.iSamplerate = query.value("Samplerate").toInt();
    info.iChannels = query.value("Channels").toInt();
    info.iBits = query.value("Bits").toInt();
    info.sFormat = query.value("Format").toString();
    info.sCoverSize = query.value("CoverSize").toString();
    info.iFileSize = query.value("FileSize").toInt();
    info.iRating = query.value("Rating").toInt();
    info.iPlayCount = query.value("PlayCount").toInt();
    info.iLastModified = query.value("LastModified").toLongLong();
    return true;
}

bool DatabaseManager::insertTrack(const QString &fullFilePath, const AudioTagInfo &info)
{
    if (!m_db.isOpen())
    {
        qDebug() << "Database not open";
        return false;
    }
    QFileInfo fi(fullFilePath);
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO Tracks ("
        "FullFilePath, BaseFileName, Extension, Path, Duration, "
        "Artist, Title, Album, Genre, TrackNum, Year, Comment, "
        "Bitrate, Samplerate, Bits, Channels, Format, CoverSize, "
        "FileSize, LastModified, Rating, PlayCount"
        ") VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"
    );
    query.addBindValue(fullFilePath);
    query.addBindValue(fi.completeBaseName());
    query.addBindValue(fi.suffix());
    query.addBindValue(fi.path());
    query.addBindValue(info.iDuration);
    query.addBindValue(info.sArtist);
    query.addBindValue(info.sTitle);
    query.addBindValue(info.sAlbum);
    query.addBindValue(info.sGenre);
    query.addBindValue(info.iTrackNum);
    query.addBindValue(info.iYear);
    query.addBindValue(info.sComment);
    query.addBindValue(info.iBitrate);
    query.addBindValue(info.iSamplerate);
    query.addBindValue(info.iBits);
    query.addBindValue(info.iChannels);
    query.addBindValue(info.sFormat);
    query.addBindValue(info.sCoverSize);
    query.addBindValue(info.iFileSize);
    query.addBindValue(fi.lastModified().toSecsSinceEpoch());
    query.addBindValue(info.iRating);
    query.addBindValue(info.iPlayCount);
    if (!query.exec())
    {
        qDebug() << "Insert error:" << query.lastError().text();
        qDebug() << "Query:" << query.lastQuery();
        return false;
    }
    return true;
}

bool DatabaseManager::updateTrack(const QString &fullFilePath, const AudioTagInfo &info)
{
    if (!m_db.isOpen())
    {
        qDebug() << "Database not open";
        return false;
    }
    QFileInfo fi(fullFilePath);
    QSqlQuery query(m_db);
    query.prepare(
        "UPDATE Tracks SET "
        "BaseFileName=?, Extension=?, Path=?, Duration=?, "
        "Artist=?, Title=?, Album=?, Genre=?, TrackNum=?, Year=?, Comment=?, "
        "Bitrate=?, Samplerate=?, Bits=?, Channels=?, Format=?, CoverSize=?, "
        "FileSize=?, LastModified=?, Rating=?, PlayCount=? "
        "WHERE FullFilePath=?"
    );
    query.addBindValue(info.sBaseFileName);
    query.addBindValue(info.sExtension);
    query.addBindValue(info.sPath);
    query.addBindValue(info.iDuration);
    query.addBindValue(info.sArtist);
    query.addBindValue(info.sTitle);
    query.addBindValue(info.sAlbum);
    query.addBindValue(info.sGenre);
    query.addBindValue(info.iTrackNum);
    query.addBindValue(info.iYear);
    query.addBindValue(info.sComment);
    query.addBindValue(info.iBitrate);
    query.addBindValue(info.iSamplerate);
    query.addBindValue(info.iBits);
    query.addBindValue(info.iChannels);
    query.addBindValue(info.sFormat);
    query.addBindValue(info.sCoverSize);
    query.addBindValue(info.iFileSize);
    query.addBindValue(fi.lastModified().toSecsSinceEpoch());
    query.addBindValue(info.iRating);
    query.addBindValue(info.iPlayCount);
    query.addBindValue(fullFilePath);
    if (!query.exec())
    {
        qDebug() << "Update error:" << query.lastError().text();
        qDebug() << "Query:" << query.lastQuery();
        return false;
    }
    return true;
}

bool DatabaseManager::loadOrUpdateTrack(const QString &fullFilePath, AudioTagInfo &info)
{
    QFileInfo fi(fullFilePath);
    qint64 fileModified = fi.lastModified().toSecsSinceEpoch();
    if (trackExists(fullFilePath))
    {
        loadTrack(fullFilePath, info);
        // File changed since last read?
        if (fileModified != info.iLastModified)
        {
            AudioTag tag(fullFilePath);
            info = tag.tagInfo();
            info.iLastModified = fileModified;
            updateTrack(fullFilePath, info);
        }
    }
    else
    {
        // New file
        AudioTag tag(fullFilePath);
        info = tag.tagInfo();
        info.iLastModified = fileModified;
        insertTrack(fullFilePath, info);
    }
    return true;
}
