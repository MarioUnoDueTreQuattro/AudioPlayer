#include "database_manager.h"

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

//bool DatabaseManager::openDatabase(const QString &dbPath)
//{
// if (QSqlDatabase::contains("audio_connection"))
// m_db = QSqlDatabase::database("audio_connection");
// else
// m_db = QSqlDatabase::addDatabase("QSQLITE", "audio_connection");
// m_db.setDatabaseName(dbPath);
// if (!m_db.open())
// {
// qDebug() << "Database open error:" << m_db.lastError().text();
// return false;
// }
// return createTables();
//}

bool DatabaseManager::openDatabase(const QString &dbPath, bool bDebug)
{
    // Se già aperto, chiudi prima
    if (m_db.isOpen())
    {
        closeDatabase();
    }
    // Verifica se il database esiste
    bool dbExists = QFile::exists(dbPath);
    // Crea connessione
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
    if (!m_db.open())
    {
        qCritical() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }
    if (bDebug) qDebug() << "Database opened:" << dbPath;
    if (bDebug) qDebug() << "Database existed:" << (dbExists ? "Yes" : "No (creating new)");
    // ===== ABILITA FOREIGN KEYS =====
    QSqlQuery query(m_db);
    if (!query.exec("PRAGMA foreign_keys = ON"))
    {
        qWarning() << "Failed to enable foreign keys:" << query.lastError().text();
        return false;
    }
    if (bDebug) qDebug() << "Foreign keys enabled";
    // Verifica che siano effettivamente abilitate
    query.exec("PRAGMA foreign_keys");
    if (query.next())
    {
        bool enabled = query.value(0).toBool();
        if (!enabled)
        {
            qCritical() << "Foreign keys not enabled!";
            return false;
        }
    }
    // ===== CREA TABELLE SE NON ESISTONO =====
    if (!createTables(bDebug))
    {
        qCritical() << "Failed to create tables";
        return false;
    }
    // ===== CONTROLLA E AGGIORNA VERSIONE DATABASE =====
    if (!checkAndUpgradeDatabase(bDebug))
    {
        qCritical() << "Database upgrade failed";
        return false;
    }
    // ===== VERIFICA INTEGRITA'=====
    if (!verifyDatabaseIntegrity(bDebug))
    {
        qWarning() << "Database integrity check failed";
        // Decidi se continuare o meno
    }
    if (bDebug) qDebug() << "Database ready";
    return true;
}

void DatabaseManager::closeDatabase()
{
    // if (m_db.isOpen())
    // m_db.close();
    if (m_db.isOpen())
    {
        QString dbName = m_db.databaseName();
        m_db.close();
        qDebug() << "Database closed:" << dbName;
    }
    // Rimuovi la connessione
    QString connectionName = m_db.connectionName();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
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
    {
        qWarning() << "Database not open";
        return false;
    }
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
    if (!m_db.isOpen())
    {
        qWarning() << "Database not open";
        return false;
    }
    QSqlQuery query(m_db);
    query.prepare("INSERT OR IGNORE INTO Favorites (TrackId, DateAdded) "
                  "SELECT Id, ? FROM Tracks WHERE FullFilePath = ?");
    QString dateAddedStr = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    query.addBindValue(dateAddedStr);
    query.addBindValue(fullFilePath);
    if (!query.exec())
    {
        qWarning() << "Failed to add/remove favorite:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::removeFromFavorites(const QString &fullFilePath)
{
    if (!m_db.isOpen())
    {
        qWarning() << "Database not open";
        return false;
    }
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM Favorites WHERE TrackId = (SELECT Id FROM Tracks WHERE FullFilePath = ?)");
    query.addBindValue(fullFilePath);
    if (!query.exec())
    {
        qWarning() << "Failed to add/remove favorite:" << query.lastError().text();
        return false;
    }
    return true;
}

QList<AudioTagInfo> DatabaseManager::favoriteTracks()
{
    QList<AudioTagInfo> list;
    if (!m_db.isOpen())
    {
        qWarning() << "Database not open";
        return list;
    }
    QSqlQuery query(m_db);
    query.prepare("SELECT T.* FROM Tracks T JOIN Favorites F ON T.Id = F.TrackId ORDER BY F.DateAdded DESC");
    if (!query.exec()) return list;
    while (query.next())
    {
        AudioTagInfo info;
        info.sFileName = query.value("FullFilePath").toString();
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
        info.sLastModified = query.value("LastModified").toString();
        list.append(info);
    }
    return list;
}

QList<AudioTagInfo> DatabaseManager::historyTracks()
{
    QList<AudioTagInfo> list;
    if (!m_db.isOpen())
    {
        qWarning() << "Database not open";
        return list;
    }
    QSqlQuery query(m_db);
    //query.prepare("SELECT T.* FROM Tracks T JOIN History F ON T.Id = F.TrackId ORDER BY F.PlayDate DESC");
    query.prepare("WITH RankedHistory AS (SELECT F.TrackId, F.PlayDate,ROW_NUMBER() OVER (PARTITION BY F.TrackId ORDER BY F.PlayDate DESC) as rn FROM History F) SELECT T.* FROM Tracks T JOIN RankedHistory RH ON T.Id = RH.TrackId WHERE RH.rn = 1 ORDER BY RH.PlayDate DESC;");
    if (!query.exec()) return list;
    while (query.next())
    {
        AudioTagInfo info;
        info.sFileName = query.value("FullFilePath").toString();
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
        info.sLastModified = query.value("LastModified").toString();
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
        info.sFileName = query.value("FullFilePath").toString();
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
        info.sLastModified = query.value("LastModified").toString();
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
    if (!m_db.transaction())   // Inizia la transazione
    {
        qDebug() << "Impossibile avviare la transazione:" << m_db.lastError().text();
        return false;
    }
    if (!clearSessionPlaylist())
    {
        m_db.rollback(); // Esegue il rollback in caso di fallimento
        return false;
    }
    QSqlQuery query(m_db);
    // Prepara la query una sola volta
    query.prepare("INSERT INTO SessionPlaylist (TrackId, Position) "
                  "SELECT Id, ? FROM Tracks WHERE FullFilePath = ?");
    for (int pos = 0; pos < fullFilePaths.size(); ++pos)
    {
        query.addBindValue(pos);
        query.addBindValue(fullFilePaths[pos]);
        if (!query.exec())
        {
            qDebug() << "Errore nell'esecuzione della query:" << query.lastError().text();
            m_db.rollback(); // Esegue il rollback se l'INSERT fallisce
            return false;
        }
    }
    if (!m_db.commit())   // Conclude la transazione
    {
        qDebug() << "Impossibile eseguire il commit della transazione:" << m_db.lastError().text();
        return false;
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
        info.sFileName = query.value("FullFilePath").toString();
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
        info.sLastModified = query.value("LastModified").toString();
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
        info.sFileName = query.value("FullFilePath").toString();
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
        info.sLastModified = query.value("LastModified").toString();
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
    qDebug() << __PRETTY_FUNCTION__ << fullFilePath;
    QSqlQuery query(m_db);
    query.prepare("UPDATE Tracks SET PlayCount = PlayCount + 1 WHERE FullFilePath = ?");
    query.addBindValue(fullFilePath);
    return query.exec();
}

bool DatabaseManager::resetAllPlayCounts()
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE Tracks SET PlayCount = 0 WHERE PlayCount > 0");
    //query.addBindValue(fullFilePath);
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

bool DatabaseManager::createTables(bool bDebug)
{
    if (bDebug) qDebug() << "Creating database tables...";
    QSqlQuery query(m_db);
    // ========== METADATA (per versioning) ==========
    QString metadataSql = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS DatabaseMetadata ("
            "Key TEXT PRIMARY KEY,"
            "Value TEXT);"
        );
    if (!query.exec(metadataSql))
    {
        qCritical() << "Error creating DatabaseMetadata:" << query.lastError().text();
        return false;
    }
    // ========== TRACKS ==========
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
            "LastModified TEXT,"
            "Rating INTEGER DEFAULT 0,"
            "PlayCount INTEGER DEFAULT 0);"
        );
    if (!query.exec(tracksSql))
    {
        qCritical() << "Error creating Tracks:" << query.lastError().text();
        return false;
    }
    // ========== PLAYLISTS ==========
    QString playlistsSql = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS Playlists ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "Name TEXT UNIQUE NOT NULL,"
            "Created TEXT DEFAULT CURRENT_TIMESTAMP,"
            "PlayCount INTEGER DEFAULT 0);"
        );
    if (!query.exec(playlistsSql))
    {
        qCritical() << "Error creating Playlists:" << query.lastError().text();
        return false;
    }
    // ========== PLAYLIST ITEMS ==========
    QString playlistItemsSql = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS PlaylistItems ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "PlaylistId INTEGER NOT NULL,"
            "TrackId INTEGER NOT NULL,"
            "Position INTEGER,"
            "FOREIGN KEY (PlaylistId) REFERENCES Playlists(Id) ON DELETE CASCADE,"
            "FOREIGN KEY (TrackId) REFERENCES Tracks(Id) ON DELETE CASCADE,"
            "UNIQUE(PlaylistId, TrackId));"
        );
    if (!query.exec(playlistItemsSql))
    {
        qCritical() << "Error creating PlaylistItems:" << query.lastError().text();
        return false;
    }
    // ========== HISTORY ==========
    QString historySql = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS History ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "TrackId INTEGER NOT NULL,"
            "PlayDate TEXT NOT NULL,"
            "PlayPosition INTEGER DEFAULT 0,"
            "FOREIGN KEY (TrackId) REFERENCES Tracks(Id) ON DELETE CASCADE);"
        );
    if (!query.exec(historySql))
    {
        qCritical() << "Error creating History:" << query.lastError().text();
        return false;
    }
    // ========== FAVORITES ==========
    QString favoritesSql = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS Favorites ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "TrackId INTEGER NOT NULL UNIQUE,"
            "DateAdded TEXT NOT NULL,"
            "FOREIGN KEY (TrackId) REFERENCES Tracks(Id) ON DELETE CASCADE);"
        );
    if (!query.exec(favoritesSql))
    {
        qCritical() << "Error creating Favorites:" << query.lastError().text();
        return false;
    }
    // ========== SESSION PLAYLIST ==========
    QString sessionSql = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS SessionPlaylist ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "TrackId INTEGER NOT NULL,"
            "Position INTEGER NOT NULL,"
            "FOREIGN KEY (TrackId) REFERENCES Tracks(Id) ON DELETE CASCADE);"
        );
    if (!query.exec(sessionSql))
    {
        qCritical() << "Error creating SessionPlaylist:" << query.lastError().text();
        return false;
    }
    if (bDebug) qDebug() << "All tables created successfully";
    return true;
}

bool DatabaseManager::createIndexes()
{
    qDebug() << "Creating database indexes...";
    QSqlQuery query(m_db);
    QStringList indexes =
    {
        // Tracks
        "CREATE INDEX IF NOT EXISTS idx_tracks_filepath ON Tracks(FullFilePath)",
        "CREATE INDEX IF NOT EXISTS idx_tracks_artist ON Tracks(Artist)",
        "CREATE INDEX IF NOT EXISTS idx_tracks_album ON Tracks(Album)",
        "CREATE INDEX IF NOT EXISTS idx_tracks_genre ON Tracks(Genre)",
        "CREATE INDEX IF NOT EXISTS idx_tracks_title ON Tracks(Title)",

        // PlaylistItems
        "CREATE INDEX IF NOT EXISTS idx_playlistitems_playlist ON PlaylistItems(PlaylistId)",
        "CREATE INDEX IF NOT EXISTS idx_playlistitems_track ON PlaylistItems(TrackId)",
        "CREATE INDEX IF NOT EXISTS idx_playlistitems_position ON PlaylistItems(Position)",

        // History
        "CREATE INDEX IF NOT EXISTS idx_history_track ON History(TrackId)",
        "CREATE INDEX IF NOT EXISTS idx_history_date ON History(PlayDate DESC)",

        // Favorites
        "CREATE INDEX IF NOT EXISTS idx_favorites_track ON Favorites(TrackId)",
        "CREATE INDEX IF NOT EXISTS idx_favorites_date ON Favorites(DateAdded DESC)",

        // SessionPlaylist
        "CREATE INDEX IF NOT EXISTS idx_session_track ON SessionPlaylist(TrackId)",
        "CREATE INDEX IF NOT EXISTS idx_session_position ON SessionPlaylist(Position)"
    };
    int created = 0;
    for (const QString &sql : indexes)
    {
        if (!query.exec(sql))
        {
            qWarning() << "Failed to create index:" << query.lastError().text();
            qWarning() << "   SQL:" << sql;
            // Continua con gli altri indici
        }
        else
        {
            created++;
        }
    }
    qDebug() << "Created" << created << "of" << indexes.size() << "indexes";
    return created > 0;
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
    info.sLastModified = query.value("LastModified").toString();
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
    query.addBindValue(fi.lastModified().toString("yyyy-MM-dd HH:mm:ss"));
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
    // QFileInfo fi(fullFilePath);
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
    query.addBindValue(info.sLastModified);
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
    QString fileModified = fi.lastModified().toString("yyyy-MM-dd HH:mm:ss");
    if (trackExists(fullFilePath))
    {
        loadTrack(fullFilePath, info);
        // File changed since last read?
        if (fileModified != info.sLastModified)
        {
            AudioTag tag(fullFilePath);
            info = tag.tagInfo();
            info.sLastModified = fileModified;
            updateTrack(fullFilePath, info);
        }
    }
    else
    {
        // New file
        AudioTag tag(fullFilePath);
        info = tag.tagInfo();
        info.sLastModified = fileModified;
        insertTrack(fullFilePath, info);
    }
    return true;
}

bool DatabaseManager::deleteInexistentFiles()
{
    if (!m_db.isOpen())
    {
        qWarning() << "DatabaseManager::deleteInexistents - database not open";
        return false;
    }
    // Get all tracks from database
    QSqlQuery selectQuery(m_db);
    if (!selectQuery.exec("SELECT Id, FullFilePath FROM Tracks"))
    {
        qWarning() << "Failed to query tracks:" << selectQuery.lastError().text();
        return false;
    }
    QList<int> idsToDelete;
    // Check which files don't exist on filesystem
    while (selectQuery.next())
    {
        int id = selectQuery.value("Id").toInt();
        QString filePath = selectQuery.value("FullFilePath").toString();
        if (!QFile::exists(filePath))
        {
            idsToDelete.append(id);
            qDebug() << "File not found, marking for deletion:" << filePath;
        }
    }
    if (idsToDelete.isEmpty())
    {
        qDebug() << "No nonexistent files found in database";
        return true;
    }
    qDebug() << "Found" << idsToDelete.size() << "nonexistent files";
    // Start transaction for data integrity
    if (!m_db.transaction())
    {
        qWarning() << "Failed to start transaction:" << m_db.lastError().text();
        return false;
    }
    QSqlQuery deleteQuery(m_db);
    // Delete from related tables first (due to foreign key constraints)
    for (int id : idsToDelete)
    {
        // Delete from Favorites
        deleteQuery.prepare("DELETE FROM Favorites WHERE TrackId = ?");
        deleteQuery.addBindValue(id);
        if (!deleteQuery.exec())
        {
            qWarning() << "Failed to delete from Favorites:" << deleteQuery.lastError().text();
            m_db.rollback();
            return false;
        }
        // Delete from History
        deleteQuery.prepare("DELETE FROM History WHERE TrackId = ?");
        deleteQuery.addBindValue(id);
        if (!deleteQuery.exec())
        {
            qWarning() << "Failed to delete from History:" << deleteQuery.lastError().text();
            m_db.rollback();
            return false;
        }
        // Delete from SessionPlaylist
        deleteQuery.prepare("DELETE FROM SessionPlaylist WHERE TrackId = ?");
        deleteQuery.addBindValue(id);
        if (!deleteQuery.exec())
        {
            qWarning() << "Failed to delete from SessionPlaylist:" << deleteQuery.lastError().text();
            m_db.rollback();
            return false;
        }
        // Delete from PlaylistItems
        deleteQuery.prepare("DELETE FROM PlaylistItems WHERE TrackId = ?");
        deleteQuery.addBindValue(id);
        if (!deleteQuery.exec())
        {
            qWarning() << "Failed to delete from PlaylistItems:" << deleteQuery.lastError().text();
            m_db.rollback();
            return false;
        }
        // Finally delete from Tracks table
        deleteQuery.prepare("DELETE FROM Tracks WHERE Id = ?");
        deleteQuery.addBindValue(id);
        if (!deleteQuery.exec())
        {
            qWarning() << "Failed to delete from Tracks:" << deleteQuery.lastError().text();
            m_db.rollback();
            return false;
        }
    }
    // Commit transaction
    if (!m_db.commit())
    {
        qWarning() << "Failed to commit transaction:" << m_db.lastError().text();
        return false;
    }
    qDebug() << "Successfully deleted" << idsToDelete.size() << "nonexistent track(s)";
    return true;
}

int DatabaseManager::getDatabaseVersion(bool bDebug)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT Value FROM DatabaseMetadata WHERE Key = 'version'");
    if (query.exec() && query.next())
    {
        int version = query.value(0).toInt();
        if (bDebug) qDebug() << "Current database version:" << version;
        return version;
    }
    // Se la tabella DatabaseMetadata non esiste o è vuota,
    // controlla se ci sono altre tabelle (database esistente ma senza versioning)
    query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='Tracks'");
    if (query.next() && query.value(0).toInt() > 0)
    {
        if (bDebug) qDebug() << "Database exists but has no version (assuming version 1)";
        return 1; // Database esistente senza versioning
    }
    if (bDebug) qDebug() << "New database (version will be set to" << CURRENT_DB_VERSION << ")";
    return 0; // Nuovo database
}

void DatabaseManager::setDatabaseVersion(int version)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO DatabaseMetadata (Key, Value) VALUES ('version', ?)");
    query.addBindValue(QString::number(version));
    if (query.exec())
    {
        qDebug() << "Database version set to:" << version;
    }
    else
    {
        qWarning() << "Failed to set database version:" << query.lastError().text();
    }
}

bool DatabaseManager::checkAndUpgradeDatabase(bool bDebug)
{
    int currentVersion = getDatabaseVersion();
    if (currentVersion == CURRENT_DB_VERSION)
    {
        if (bDebug) qDebug() << "Database is up to date (version" << CURRENT_DB_VERSION << ")";
        return true;
    }
    if (currentVersion == 0)
    {
        // Nuovo database, imposta versione corrente
        setDatabaseVersion(CURRENT_DB_VERSION);
        createIndexes(); // Crea indici per nuovo database
        return true;
    }
    if (currentVersion > CURRENT_DB_VERSION)
    {
        qCritical() << "Database version" << currentVersion
            << "is newer than supported version" << CURRENT_DB_VERSION;
        qCritical() << "   Please update the application!";
        return false;
    }
    // Esegui migrazioni in sequenza
    if (bDebug) qDebug() << "Upgrading database from version" << currentVersion
            << "to" << CURRENT_DB_VERSION;
    // Backup prima della migrazione (opzionale ma consigliato)
    QString dbPath = m_db.databaseName();
    QString backupPath = dbPath + ".backup_v" + QString::number(currentVersion)
        + "_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    if (QFile::copy(dbPath, backupPath))
    {
        if (bDebug) qDebug() << "Backup created:" << backupPath;
    }
    else
    {
        qWarning() << "Failed to create backup";
        // Decidi se continuare o meno
    }
    // Applica migrazioni
    if (currentVersion < 2)
    {
        if (!upgradeToVersion2())
        {
            qCritical() << "Failed to upgrade to version 2";
            return false;
        }
    }
    // Aggiungi future migrazioni qui
    // if (currentVersion < 3)
    // {
    // if (!upgradeToVersion3())
    // {
    // qCritical() << "[ERROR] Failed to upgrade to version 3";
    // return false;
    // }
    // }
    // Future migrazioni v3 → v4
    // if (currentVersion < 4)
    // {
    // if (!upgradeToVersion4())
    // {
    // return false;
    // }
    // }
    // Aggiungi future migrazioni qui
    // if (currentVersion < 3)
    // {
    // if (!upgradeToVersion3())
    // {
    // return false;
    // }
    // }
    setDatabaseVersion(CURRENT_DB_VERSION);
    if (bDebug) qDebug() << "Database upgrade completed successfully";
    return true;
}

bool DatabaseManager::upgradeToVersion2()
{
    qDebug() << "Upgrading to version 2...";
    qDebug() << "   - Add ON DELETE CASCADE to all foreign keys";
    qDebug() << "   - Add UNIQUE constraint to Favorites.TrackId";
    qDebug() << "   - Add UNIQUE(PlaylistId, TrackId) to PlaylistItems";
    qDebug() << "   - Create performance indexes";
    if (!m_db.transaction())
    {
        qCritical() << "Failed to start transaction";
        return false;
    }
    QSqlQuery query(m_db);
    // ========== FAVORITES ==========
    qDebug() << "   Migrating Favorites table...";
    if (!query.exec("ALTER TABLE Favorites RENAME TO Favorites_old"))
    {
        qCritical() << "Failed to rename Favorites:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    QString favoritesSql = QStringLiteral(
            "CREATE TABLE Favorites ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "TrackId INTEGER NOT NULL UNIQUE,"
            "DateAdded TEXT NOT NULL,"
            "FOREIGN KEY (TrackId) REFERENCES Tracks(Id) ON DELETE CASCADE)"
        );
    if (!query.exec(favoritesSql))
    {
        qCritical() << "Failed to create new Favorites:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    // Copia dati rimuovendo duplicati
    if (!query.exec("INSERT INTO Favorites (TrackId, DateAdded) "
                    "SELECT TrackId, MIN(COALESCE(DateAdded, datetime('now'))) "
                    "FROM Favorites_old "
                    "GROUP BY TrackId"))
    {
        qCritical() << "Failed to copy Favorites data:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    int favoritesCount = query.numRowsAffected();
    if (!query.exec("DROP TABLE Favorites_old"))
    {
        qCritical() << "Failed to drop old Favorites:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    qDebug() << "   Favorites migrated (" << favoritesCount << "records)";
    // ========== PLAYLIST ITEMS ==========
    qDebug() << "   Migrating PlaylistItems table...";
    if (!query.exec("ALTER TABLE PlaylistItems RENAME TO PlaylistItems_old"))
    {
        qCritical() << "Failed to rename PlaylistItems:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    QString playlistItemsSql = QStringLiteral(
            "CREATE TABLE PlaylistItems ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "PlaylistId INTEGER NOT NULL,"
            "TrackId INTEGER NOT NULL,"
            "Position INTEGER,"
            "FOREIGN KEY (PlaylistId) REFERENCES Playlists(Id) ON DELETE CASCADE,"
            "FOREIGN KEY (TrackId) REFERENCES Tracks(Id) ON DELETE CASCADE,"
            "UNIQUE(PlaylistId, TrackId))"
        );
    if (!query.exec(playlistItemsSql))
    {
        qCritical() << "Failed to create new PlaylistItems:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    // Copia dati rimuovendo duplicati (mantieni la prima occorrenza per posizione)
    if (!query.exec("INSERT OR IGNORE INTO PlaylistItems (PlaylistId, TrackId, Position) "
                    "SELECT PlaylistId, TrackId, MIN(Position) "
                    "FROM PlaylistItems_old "
                    "GROUP BY PlaylistId, TrackId"))
    {
        qCritical() << "Failed to copy PlaylistItems data:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    int playlistItemsCount = query.numRowsAffected();
    if (!query.exec("DROP TABLE PlaylistItems_old"))
    {
        qCritical() << "Failed to drop old PlaylistItems:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    qDebug() << "   PlaylistItems migrated (" << playlistItemsCount << "records)";
    // ========== HISTORY ==========
    qDebug() << "   Migrating History table...";
    if (!query.exec("ALTER TABLE History RENAME TO History_old"))
    {
        qCritical() << "Failed to rename History:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    QString historySql = QStringLiteral(
            "CREATE TABLE History ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "TrackId INTEGER NOT NULL,"
            "PlayDate TEXT NOT NULL,"
            "PlayPosition INTEGER DEFAULT 0,"
            "FOREIGN KEY (TrackId) REFERENCES Tracks(Id) ON DELETE CASCADE)"
        );
    if (!query.exec(historySql))
    {
        qCritical() << "Failed to create new History:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    if (!query.exec("INSERT INTO History (Id, TrackId, PlayDate, PlayPosition) "
                    "SELECT Id, TrackId, COALESCE(PlayDate, datetime('now')), "
                    "COALESCE(PlayPosition, 0) FROM History_old"))
    {
        qCritical() << "Failed to copy History data:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    int historyCount = query.numRowsAffected();
    if (!query.exec("DROP TABLE History_old"))
    {
        qCritical() << "Failed to drop old History:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    qDebug() << "   History migrated (" << historyCount << "records)";
    // ========== SESSION PLAYLIST ==========
    qDebug() << "   Migrating SessionPlaylist table...";
    if (!query.exec("ALTER TABLE SessionPlaylist RENAME TO SessionPlaylist_old"))
    {
        qCritical() << "Failed to rename SessionPlaylist:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    QString sessionSql = QStringLiteral(
            "CREATE TABLE SessionPlaylist ("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "TrackId INTEGER NOT NULL,"
            "Position INTEGER NOT NULL,"
            "FOREIGN KEY (TrackId) REFERENCES Tracks(Id) ON DELETE CASCADE)"
        );
    if (!query.exec(sessionSql))
    {
        qCritical() << "Failed to create new SessionPlaylist:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    if (!query.exec("INSERT INTO SessionPlaylist (Id, TrackId, Position) "
                    "SELECT Id, TrackId, COALESCE(Position, 0) FROM SessionPlaylist_old"))
    {
        qCritical() << "Failed to copy SessionPlaylist data:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    int sessionCount = query.numRowsAffected();
    if (!query.exec("DROP TABLE SessionPlaylist_old"))
    {
        qCritical() << "Failed to drop old SessionPlaylist:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    qDebug() << "   SessionPlaylist migrated (" << sessionCount << "records)";
    // ========== CREA INDICI ==========
    qDebug() << "   Creating indexes...";
    if (!createIndexes())
    {
        qWarning() << "Some indexes failed to create (non-critical)";
        // Continua comunque
    }
    // ========== COMMIT ==========
    if (!m_db.commit())
    {
        qCritical() << "Failed to commit transaction:" << m_db.lastError().text();
        m_db.rollback();
        return false;
    }
    qDebug() << "Successfully upgraded to version 2";
    return true;
}

bool DatabaseManager::upgradeToVersion3()
{
    qDebug() << "[UPGRADE] Upgrading to version 3...";
    qDebug() << "          - Add 'Lyrics' column to Tracks table";
    qDebug() << "          - Add 'Color' column to Playlists table";
    if (!m_db.transaction())
    {
        qCritical() << "[ERROR] Failed to start transaction";
        return false;
    }
    QSqlQuery query(m_db);
    // ===== AGGIUNGI COLONNA 'Lyrics' A TRACKS =====
    if (!query.exec("ALTER TABLE Tracks ADD COLUMN Lyrics TEXT"))
    {
        qCritical() << "[ERROR] Failed to add Lyrics column:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    qDebug() << "          [OK] Added Lyrics column to Tracks";
    // ===== AGGIUNGI COLONNA 'Color' A PLAYLISTS =====
    if (!query.exec("ALTER TABLE Playlists ADD COLUMN Color TEXT DEFAULT '#FFFFFF'"))
    {
        qCritical() << "[ERROR] Failed to add Color column:" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    qDebug() << "          [OK] Added Color column to Playlists";
    // Commit
    if (!m_db.commit())
    {
        qCritical() << "[ERROR] Failed to commit transaction";
        m_db.rollback();
        return false;
    }
    qDebug() << "[OK] Successfully upgraded to version 3";
    return true;
}

bool DatabaseManager::recreateTableWithChanges(
    const QString &tableName,
    const QString &newTableSql,
    const QString &dataCopySql)
{
    QSqlQuery query(m_db);
    QString oldTableName = tableName + "_old";
    // 1. Rinomina vecchia tabella
    QString renameSql = QString("ALTER TABLE %1 RENAME TO %2").arg(tableName, oldTableName);
    if (!query.exec(renameSql))
    {
        qWarning() << "Failed to rename table" << tableName << ":" << query.lastError().text();
        return false;
    }
    // 2. Crea nuova tabella
    if (!query.exec(newTableSql))
    {
        qWarning() << "Failed to create new table" << tableName << ":" << query.lastError().text();
        return false;
    }
    // 3. Copia dati (se fornito SQL personalizzato, altrimenti copia tutto)
    QString copySql = dataCopySql.isEmpty()
        ? QString("INSERT INTO %1 SELECT * FROM %2").arg(tableName, oldTableName)
        : dataCopySql;
    if (!query.exec(copySql))
    {
        qWarning() << "Failed to copy data to" << tableName << ":" << query.lastError().text();
        return false;
    }
    // 4. Elimina vecchia tabella
    QString dropSql = QString("DROP TABLE %1").arg(oldTableName);
    if (!query.exec(dropSql))
    {
        qWarning() << "Failed to drop old table" << oldTableName << ":" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::verifyDatabaseIntegrity(bool bDebug)
{
    if (bDebug) qDebug() << "Verifying database integrity...";
    QSqlQuery query(m_db);
    // 1. PRAGMA integrity_check
    if (!query.exec("PRAGMA integrity_check"))
    {
        qWarning() << "Failed to run integrity check";
        return false;
    }
    if (query.next())
    {
        QString result = query.value(0).toString();
        if (result != "ok")
        {
            qCritical() << "Database integrity check failed:" << result;
            return false;
        }
    }
    // 2. PRAGMA foreign_key_check
    if (!query.exec("PRAGMA foreign_key_check"))
    {
        qWarning() << "Failed to run foreign key check";
        return false;
    }
    if (query.next())
    {
        qCritical() << "Foreign key violations detected:";
        do
        {
            qCritical() << "   Table:" << query.value(0).toString()
                << "Row:" << query.value(1).toString()
                << "Parent:" << query.value(2).toString();
        }
        while (query.next());
        return false;
    }
    // 3. Verifica che le tabelle esistano
    QStringList expectedTables =
    {
        "DatabaseMetadata", "Tracks", "Playlists", "PlaylistItems",
        "History", "Favorites", "SessionPlaylist"
    };
    for (const QString &tableName : expectedTables)
    {
        query.exec(QString("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%1'")
            .arg(tableName));
        if (!query.next() || query.value(0).toInt() == 0)
        {
            qCritical() << "Missing table:" << tableName;
            return false;
        }
    }
    if (bDebug) qDebug() << "Database integrity verified";
    return true;
}
