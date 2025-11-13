#ifndef PLAYLISTHELPER_H
#define PLAYLISTHELPER_H

#include <QString>
#include <QMediaPlaylist>
#include <QTableView>
#include <QListWidget>

class PlaylistHelper
{
public:
    // Copy file path to clipboard
    static void copyFilePathToClipboard(const QString &filePath);
    // Get file path from playlist at given index
    static QString getFilePathFromPlaylist(QMediaPlaylist *playlist, int index);
    // Get currently selected file path (helper for context menus)
    static QString getCurrentFilePath(QMediaPlaylist *playlist, int currentIndex);
    static void openFolderAndSelectFileInExplorer(const QString &filePath);
    static void openFolderAndSelectFile(const QString &filePath);
    static void openFolderAndSelectFileEx(const QString &filePath);
    static void openGoogleSearch(const QString &text);
    static void copyCurrentName(QListWidget *playlist);
    static void copyCurrentName(QTableView *tableView);
    static void copyCurrentFullPath(QMediaPlaylist *playlist, QListWidget *listWidget);
    static void copyCurrentFullPath(QMediaPlaylist *playlist, QTableView *tableView);
    static QString getCurrentFullPath(QTableView *tableView);
    static QString getCurrentFullPath(QMediaPlaylist *playlist, QListWidget *listWidget);
private:
    PlaylistHelper(); // Private constructor - utility class, no instances
};

#endif // PLAYLISTHELPER_H
