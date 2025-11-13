#include "playlist_helper.h"
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QApplication>
#include <QClipboard>
#include <QDebug>

#ifdef Q_OS_WIN
    #include <windows.h>
    #include <shlobj.h>
    #include <objbase.h>
    #include <QDesktopServices>
#endif

QString PlaylistHelper::getCurrentFullPath(QTableView *tableView)
{
    QModelIndex currentIndex = tableView->currentIndex();
    if (currentIndex.isValid())
    {
        // Get full path from UserRole + 1
        QString fullPath = tableView->model()->data(currentIndex, Qt::UserRole + 1).toString();
        if (!fullPath.isEmpty())
        {
            return fullPath;
        }
        else
        {
            qDebug() << "Selected cell has no file path";
            return QString();
        }
    }
    else
    {
        qDebug() << "No item selected for searching.";
        return QString();
    }
}

QString PlaylistHelper::getCurrentFullPath(QMediaPlaylist *playlist, QListWidget *listWidget)
{
    QString sCurrentQUrl = playlist->media(listWidget->currentRow()).canonicalUrl().toLocalFile();
    if (sCurrentQUrl.isEmpty() == false)
    {
        sCurrentQUrl = QDir::toNativeSeparators(sCurrentQUrl);
        return sCurrentQUrl;
    }
    else
    {
        qDebug() << "No item selected for copying.";
        return QString();
    }
}

void PlaylistHelper::copyCurrentName(QListWidget *playlist)
{
    QListWidgetItem *currentItem = playlist->currentItem();
    if (currentItem)
    {
        QApplication::clipboard()->setText(currentItem->text());
    }
    else
    {
        qDebug() << "No item selected for copying.";
    }
}

void PlaylistHelper::copyCurrentName(QTableView *tableView)
{
    if (!tableView)
    {
        qDebug() << "QTableView is null";
        return;
    }
    QModelIndex currentIndex = tableView->currentIndex();
    if (currentIndex.isValid())
    {
        // Get full path from UserRole + 1
        QString fullPath = tableView->model()->data(currentIndex, Qt::UserRole + 1).toString();
        if (!fullPath.isEmpty())
        {
            QFileInfo fi(fullPath);
            QString fileName = fi.fileName(); // Complete filename with extension
            QApplication::clipboard()->setText(fileName);
        }
        else
        {
            qDebug() << "Selected cell has no file path";
        }
    }
    else
    {
        qDebug() << "No item selected for copying.";
    }
}

void PlaylistHelper::copyCurrentFullPath(QMediaPlaylist *playlist, QListWidget *listWidget)
{
    QString sCurrentQUrl = playlist->media(listWidget->currentRow()).canonicalUrl().toLocalFile();
    if (sCurrentQUrl.isEmpty() == false)
    {
        sCurrentQUrl = QDir::toNativeSeparators(sCurrentQUrl);
        QApplication::clipboard()->setText(sCurrentQUrl);
    }
    else
    {
        qDebug() << "No item selected for copying.";
    }
}

void PlaylistHelper::copyCurrentFullPath(QMediaPlaylist *playlist, QTableView *tableView)
{
    if (!playlist || !tableView)
    {
        qDebug() << "Playlist or TableView is null";
        return;
    }
    QModelIndex currentIndex = tableView->currentIndex();
    if (!currentIndex.isValid())
    {
        qDebug() << "No item selected for copying.";
        return;
    }
    // Get the full path from UserRole + 1
    QString sCurrentQUrl = tableView->model()->data(currentIndex, Qt::UserRole + 1).toString();
    if (sCurrentQUrl.isEmpty() == false)
    {
        sCurrentQUrl = QDir::toNativeSeparators(sCurrentQUrl);
        QApplication::clipboard()->setText(sCurrentQUrl);
    }
    else
    {
        qDebug() << "No item selected for copying.";
    }
}

void PlaylistHelper::openGoogleSearch(const QString &text)
{
    // Percent-encode the query text
    const QByteArray encodedQuery = QUrl::toPercentEncoding(text);
    // Build google search URL (uses 'q' parameter)
    const QUrl searchUrl(QStringLiteral("https://www.google.com/search?q=%1").arg(QString::fromUtf8(encodedQuery)));
    // Open default browser
    bool success = QDesktopServices::openUrl(searchUrl);
    if (!success)
    {
        qWarning() << "Failed to open URL:" << searchUrl.toString();
    }
}

void PlaylistHelper::openFolderAndSelectFile(const QString &filePath)
{
    //    // Use Windows Explorer to open and select the file
    // QStringList args;
    //    // args << "/select," ;
    // args << QDir::toNativeSeparators(filePath);
    //    // QProcess::startDetached("explorer.exe", args);
    // QProcess::startDetached("explorer.exe", QStringList() << QDir::toNativeSeparators(filePath));
    // LPCWSTR path = (LPCWSTR)filePath.utf16();
    // PIDLIST_ABSOLUTE pidl;
    // if (SUCCEEDED(SHParseDisplayName(path, nullptr, &pidl, 0, nullptr)))
    // {
    //        // Select the item in its folder
    // SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
    // CoTaskMemFree(pidl);
    // }
    // Convert QString to UTF-16 (LPCWSTR)
    QFileInfo fi(filePath);
    if (!fi.exists())
    {
        qDebug() << "File does not exist:" << filePath;
        return;
    }
    QString targetFilePath = fi.absoluteFilePath();
    // Resolve file symlink if it exists
    if (fi.isSymLink())
    {
        qDebug() << "File is symlink";
        QString realFile = fi.symLinkTarget();
        if (!realFile.isEmpty())
        {
            targetFilePath = realFile;
        }
        else
        {
            qDebug() << "Failed to resolve file symlink:" << filePath;
            return;
        }
    }
    // Resolve folder symlink if it exists
    QFileInfo folderInfo(QFileInfo(targetFilePath).absolutePath());
    QString folderPath = folderInfo.absoluteFilePath();
    if (folderInfo.isSymLink())
    {
        qDebug() << "Folder is symlink";
        QString realFolder = folderInfo.symLinkTarget();
        if (!realFolder.isEmpty())
        {
            folderPath = realFolder;
        }
        else
        {
            qDebug() << "Failed to resolve folder symlink:" << folderInfo.absoluteFilePath();
            return;
        }
    }
    // Combine resolved folder and file name
    QString nativePath = QDir::toNativeSeparators(folderPath + "\\" + QFileInfo(targetFilePath).fileName());
    qDebug() << "Resolved path=" << nativePath;
    LPCWSTR path = reinterpret_cast<LPCWSTR>(nativePath.utf16());
    PIDLIST_ABSOLUTE pidl = nullptr;
    HRESULT hr = SHParseDisplayName(path, nullptr, &pidl, 0, nullptr);
    if (SUCCEEDED(hr) && pidl != nullptr)
    {
        hr = SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
        if (FAILED(hr))
        {
            qDebug() << "Failed to open folder and select item:" << nativePath;
        }
        CoTaskMemFree(pidl);
    }
    else
    {
        qDebug() << "Failed to parse path:" << nativePath;
    }
}

/*
 * Opens the folder containing the given file in the user's default file manager
 * and selects the file.
 *
 * Features:
 * - Works with Explorer, Directory Opus, and other shell-integrated file managers.
 * - Resolves symlinks for both the file and its containing folder.
 * - If the file or folder cannot be resolved, still opens the folder.
 * - Safe for Windows 7 32-bit and Qt5.
 */
void PlaylistHelper::openFolderAndSelectFileEx(const QString &filePath)
{
    QFileInfo fi(filePath);
    if (!fi.exists())
    {
        qDebug() << "File does not exist:" << filePath;
        return;
    }
    // Step 1: Resolve file symlink if it exists
    QString targetFilePath = fi.absoluteFilePath();
    if (fi.isSymLink())
    {
        QString realFile = fi.symLinkTarget();
        if (!realFile.isEmpty())
        {
            targetFilePath = realFile;
        }
        else
        {
            qDebug() << "File is a broken symlink:" << filePath;
            targetFilePath = fi.absoluteFilePath(); // fallback: select the symlink itself
        }
    }
    // Step 2: Resolve folder symlink if it exists
    QFileInfo folderInfo(QFileInfo(targetFilePath).absolutePath());
    QString folderPath = folderInfo.absoluteFilePath();
    if (folderInfo.isSymLink())
    {
        QString realFolder = folderInfo.symLinkTarget();
        if (!realFolder.isEmpty())
        {
            folderPath = realFolder;
        }
        else
        {
            qDebug() << "Folder is a broken symlink:" << folderInfo.absoluteFilePath();
            folderPath = folderInfo.absoluteFilePath(); // fallback: open the symlink folder
        }
    }
    // Step 3: Combine resolved folder path and file name
    QString nativePath = QDir::toNativeSeparators(folderPath + "\\" + QFileInfo(targetFilePath).fileName());
    LPCWSTR path = reinterpret_cast<LPCWSTR>(nativePath.utf16());
    // Step 4: Try using the Windows Shell API to select the file
    PIDLIST_ABSOLUTE pidl = nullptr;
    HRESULT hr = SHParseDisplayName(path, nullptr, &pidl, 0, nullptr);
    if (SUCCEEDED(hr) && pidl != nullptr)
    {
        hr = SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
        if (FAILED(hr))
        {
            qDebug() << "Shell API failed, falling back to opening folder:" << nativePath;
            QProcess::startDetached("explorer.exe", QStringList() << QDir::toNativeSeparators(folderPath));
        }
        CoTaskMemFree(pidl);
    }
    else
    {
        // Step 5: Shell API failed (e.g., path contains special characters), fallback
        qDebug() << "Failed to parse path with Shell API, opening folder:" << nativePath;
        QProcess::startDetached("explorer.exe", QStringList() << QDir::toNativeSeparators(folderPath));
    }
}

void PlaylistHelper::openFolderAndSelectFileInExplorer(const QString &filePath)
{
    // Use Windows Explorer to open and select the file
    QStringList args;
    args << "/select,";
    args << QDir::toNativeSeparators(filePath);
    QProcess::startDetached("explorer.exe", args);
    //QProcess::startDetached("explorer.exe", QStringList() << QDir::toNativeSeparators(filePath));
}

void PlaylistHelper::copyFilePathToClipboard(const QString &filePath)
{
    if (filePath.isEmpty())
    {
        qDebug() << "No file path to copy.";
        return;
    }
    QString nativePath = QDir::toNativeSeparators(filePath);
    QApplication::clipboard()->setText(nativePath);
}

QString PlaylistHelper::getFilePathFromPlaylist(QMediaPlaylist *playlist, int index)
{
    if (!playlist || index < 0 || index >= playlist->mediaCount())
    {
        qDebug() << "Invalid playlist or index:" << index;
        return QString();
    }
    return playlist->media(index).canonicalUrl().toLocalFile();
}

QString PlaylistHelper::getCurrentFilePath(QMediaPlaylist *playlist, int currentIndex)
{
    return getFilePathFromPlaylist(playlist, currentIndex);
}
