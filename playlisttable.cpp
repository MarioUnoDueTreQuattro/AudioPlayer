#include "playlisttable.h"
#include "ui_playlisttable.h"
#include "playlistdelegates.h"
#include <QFileInfo>
#include <QVBoxLayout>
#include <QDebug>
#include <QHeaderView>
//#include <QSettings>
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>

//PlaylistTable::PlaylistTable(QWidget *parent) :
// QWidget(parent),
// ui(new Ui::PlaylistTable),
// m_player(player)
//{
// ui->setupUi(this);
//}

PlaylistTable::PlaylistTable(QMediaPlayer *player, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PlaylistTable),
      m_player(player)
{
    qRegisterMetaType<AudioTagInfo>("AudioTagInfo");
    ui->setupUi(this);
    setWindowFlags(Qt::Tool);
    setWindowTitle(qApp->applicationName() + " playlist");
    settingsMgr = SettingsManager::instance();
    // --- Create playlist ---
    m_playlist = new QMediaPlaylist(this);
    m_player->setPlaylist(m_playlist);
    // --- Create source model ---
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels(QStringList()
        << "Filename" << "Ext" << "Path" << "Duration" << "Artist"
        << "Title" << "Album" << "Track" << "Year" << "Genre"
        << "Comment" << "Bitrate" << "Samplerate" << "Bits" << "Channels"
        << "Format" << "Cover size" << "File size");
    // --- Create sort proxy ---
    m_sortModel = new PlaylistSortModel(this);
    m_sortModel->setSourceModel(m_model);
    //    // --- Create view ---
    // m_view = new QTableView(this);
    // m_view->setModel(m_sortModel);
    // m_view->horizontalHeader()->setStretchLastSection(true);
    // --- Create table view ---
    m_view = ui->tableView; // new QTableView(this);
    m_view->horizontalHeader()->setHighlightSections(false);
    //m_view->setModel(m_model);
    m_view->setModel(m_sortModel);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //m_view->horizontalHeader()->setStretchLastSection(true);
    // m_view->verticalHeader()->hide();
    // Enable sorting
    m_view->setSortingEnabled(true);
    m_view->horizontalHeader()->setSortIndicatorShown(true);
    m_view->setShowGrid(false);
    m_view->setAlternatingRowColors(true);
    // m_view->horizontalHeader()->setSectionsClickable(true);
    // --- Create and assign duration delegate ---
    PlaylistDurationDelegate *durationDelegate = new PlaylistDurationDelegate(this);
    m_view->setItemDelegateForColumn(3, durationDelegate);
    PlaylistFileSizeDelegate *FileSizeDelegate = new PlaylistFileSizeDelegate(this);
    m_view->setItemDelegateForColumn(17, FileSizeDelegate);
    // QHeaderView *header = m_view->horizontalHeader();
    // header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    // header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    // header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    // header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    // header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    // header->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    // --- Layout ---
    // QVBoxLayout *layout = new QVBoxLayout(this);
    // layout->addWidget(m_view);
    // setLayout(layout);
    // --- Connections ---
    connect(m_view->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
        this, SLOT(onHeaderSortChanged(int, Qt::SortOrder)));
    connect(m_view, SIGNAL(doubleClicked(const QModelIndex &)),
        this, SLOT(onDoubleClicked(const QModelIndex &)));
    connect(m_playlist, SIGNAL(currentIndexChanged(int)),
        this, SLOT(onCurrentTrackChanged(int)));
    loadsettings();
    // m_view->horizontalHeader ()->setSectionResizeMode (0, QHeaderView::Stretch);
    // m_view->horizontalHeader ()->setSectionResizeMode (1, QHeaderView::ResizeToContents);
    // m_view->horizontalHeader ()->setSectionResizeMode (2, QHeaderView::Stretch);
    // m_view->horizontalHeader ()->setSectionResizeMode (3, QHeaderView::ResizeToContents);
    // int iSortCol = settingsMgr->value("PlaylistViewSortColumn", 0).toInt();
    // Qt::SortOrder order = static_cast<Qt::SortOrder>(settingsMgr->value("PlaylistViewSortColumnOrder", 0).toInt());
    // m_sortModel->sort(iSortCol, order);
    // onHeaderSortChanged (iSortCol,order);
}

PlaylistTable::~PlaylistTable()
{
    delete ui;
}

void PlaylistTable::closeEvent(QCloseEvent *event)
{
    if (m_FutureWatcher != nullptr)
    {
        if (m_FutureWatcher->isRunning())
        {
            m_tagWorker->m_bStop = true;
            QFuture<void> future = m_FutureWatcher->future();
            future.cancel();
            future.waitForFinished();
        }
        m_FutureWatcher->waitForFinished();
        delete m_tagWorker;
        m_tagWorker = nullptr;
    }
    if (m_tagWorker != nullptr) delete m_tagWorker;
    emit windowClosed();
    event->accept();
    QWidget::closeEvent(event);
}

void PlaylistTable::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::StyleChange || event->type() == QEvent::PaletteChange)
    {
        //qDebug() << "Stile del widget cambiato!";
        // Inserisci qui il tuo codice per reagire al cambio di stile
        //QPalette palette = ui->textEditInfo->palette();
        QPalette palette = this->palette();
        palette.setColor(QPalette::Base, palette.color(QPalette::Window));
        //ui->textEditInfo->setPalette(palette);
    }
    else if (event->type() == QEvent::ActivationChange)
    {
        if (isActiveWindow())
        {
            emit focusReceived();
        }
    }
    // 2. Chiama l'implementazione della classe base
    QWidget::changeEvent(event);
}

void PlaylistTable::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    //QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    //settings.setValue("PlaylistViewPosition", pos());
    settingsMgr->setValue("PlaylistViewPosition", pos());
}

void PlaylistTable::resizeEvent(QResizeEvent *event)
{
    // QSize newSize = event->size();
    // QSize oldSize = event->oldSize();
    // qDebug() << "Widget resized from" << oldSize << "to" << newSize;
    //QSettings settings;
    // Save window geometry & state
    settingsMgr->setValue("PlaylistViewGeometry", saveGeometry());
    // settings.setValue("windowState", saveState());
    // Call base implementation (optional if QWidget)
    QWidget::resizeEvent(event);
}

void PlaylistTable::loadsettings()
{
    // Restore position
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int pos_x = (screenGeometry.width() - this->width()) / 2;
    int pos_y = (screenGeometry.height() - this->height()) / 2;
    //QSettings settings;
    QPoint savedPos = settingsMgr->value("PlaylistViewPosition", QPoint(pos_x, pos_y)).toPoint();
    QScreen *screen = QGuiApplication::screenAt(savedPos);
    if (screen)
    {
        move(savedPos);
    }
    else
    {
        // Se la posizione non è valida, centra la finestra
        move(QPoint(pos_x, pos_y));
    }
    QByteArray geometry = settingsMgr->value("PlaylistViewGeometry").toByteArray();
    if (!geometry.isEmpty())
        restoreGeometry(geometry);
}

void PlaylistTable::syncPlaylistOrder()
{
    qDebug() << "syncPlaylistOrder";
    int sortCol = m_view->horizontalHeader()->sortIndicatorSection();
    Qt::SortOrder sortOrder = m_view->horizontalHeader()->sortIndicatorOrder();
    qApp->processEvents();
    // Force re-sort now
    m_sortModel->sort(sortCol, sortOrder);
    if (m_player->state() != QMediaPlayer::StoppedState)
        m_player->stop();
    QMediaPlaylist *newPlaylist = new QMediaPlaylist(this);
    newPlaylist->clear();
    // Keep old current media URL so we can restore the same track (if present)
    QUrl oldCurrentUrl;
    int oldIndex = -1;
    if (m_playlist)
    {
        oldIndex = m_playlist->currentIndex();
        if (oldIndex >= 0 && oldIndex < m_playlist->mediaCount())
            oldCurrentUrl = m_playlist->media(oldIndex).canonicalUrl();
    }
    // Iterate through the proxy model, which reflects sorted order
    for (int i = 0; i < m_sortModel->rowCount(); ++i)
    {
        QModelIndex pathIndex = m_sortModel->index(i, 2); // column 2 = Path
        QString path = m_sortModel->data(pathIndex).toString() + "/";
        pathIndex = m_sortModel->index(i, 0);
        path.append(m_sortModel->data(pathIndex).toString());
        path.append(".");
        pathIndex = m_sortModel->index(i, 1);
        path.append(m_sortModel->data(pathIndex).toString());
        //path.append(".m4a");
        qDebug() << path;
        newPlaylist->addMedia(QUrl::fromLocalFile(path));
    }
    // Replace playlist
    // disconnect old playlist signals if necessary
    if (m_playlist)
    {
        disconnect(m_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentTrackChanged(int)));
        delete m_playlist;
    }
    m_playlist = newPlaylist;
    m_player->setPlaylist(m_playlist);
    // int oldIndex = m_playlist->currentIndex();
    int newCount = newPlaylist->mediaCount();
    // delete m_playlist;
    // m_playlist = newPlaylist;
    // m_player->setPlaylist(m_playlist);
    connect(m_playlist, SIGNAL(currentIndexChanged(int)),
        this, SLOT(onCurrentTrackChanged(int)));
    if (oldIndex >= 0 && oldIndex < newCount)
        m_playlist->setCurrentIndex(oldIndex);
    if (!oldCurrentUrl.isEmpty())
    {
        for (int i = 0; i < m_playlist->mediaCount(); ++i)
        {
            if (m_playlist->media(i).canonicalUrl() == oldCurrentUrl)
            {
                m_playlist->setCurrentIndex(i);
                break;
            }
        }
    }
    // align
    //m_view->setColumnWidth(3, 80);
    m_view->horizontalHeader()->setDefaultAlignment(Qt::AlignRight);
    m_view->setTextElideMode(Qt::ElideRight);
    for (int row = 0; row < m_model->rowCount(); ++row)
        m_model->item(row, 3)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    emit playlistUpdated(m_playlist);
}

void PlaylistTable::onHeaderSortChanged(int logicalIndex, Qt::SortOrder order)
{
    Q_UNUSED(logicalIndex);
    Q_UNUSED(order);
    //qDebug() << "onHeaderSortChanged logicalIndex" << logicalIndex;
    settingsMgr->setValue("PlaylistViewSortColumn", logicalIndex);
    settingsMgr->setValue("PlaylistViewSortColumnOrder", order);
    // After sorting the model, rebuild the QMediaPlaylist order.
    syncPlaylistOrder();
    qDebug() << "Playlist sorted and reordered to match table view.";
}

//void PlaylistView::addTrack(const QString &filePath)
//{
// QString fileName = extractFileName(filePath);

// QList<QStandardItem*> rowItems;
// rowItems << new QStandardItem(fileName)
// << new QStandardItem(filePath);

// m_model->appendRow(rowItems);
// m_playlist->addMedia(QUrl::fromLocalFile(filePath));
//}

void PlaylistTable::on_pushButton_clicked()
{
    ui->pushButton->setEnabled(false);
    m_view->setSortingEnabled(false);
    if (!m_sortModel)
        return;
    int rowCount = m_sortModel->rowCount();
    QStringList fileList;
    // Iterate through the proxy model (sorted/displayed order)
    for (int idx = 0; idx < rowCount; ++idx)
    {
        // Column 0 = Filename column; UserRole+1 stores the full path
        QModelIndex index = m_sortModel->index(idx, 0);
        QString fullPath = m_sortModel->data(index, Qt::UserRole + 1).toString();
        if (!fullPath.isEmpty())
            fileList.append(fullPath);
    }
    // Run the tag loading in a separate thread
    m_tagWorker = new TagLoaderWorker();
    QFuture<void> future = QtConcurrent::run(m_tagWorker, &TagLoaderWorker::processFiles, fileList);
    m_FutureWatcher = new QFutureWatcher<void>(this);
    m_FutureWatcher->setFuture(future);
    connect(m_tagWorker, SIGNAL(finished()), this, SLOT(onTagLoadingFinished()));
    connect(m_tagWorker, SIGNAL(tagLoaded(QString, AudioTagInfo)),
        this, SLOT(onTagLoaded(QString, AudioTagInfo)));
}

void PlaylistTable::onTagLoadingFinished()
{
    delete m_tagWorker;
    m_tagWorker = nullptr;
    delete m_FutureWatcher;
    m_FutureWatcher = nullptr;
    ui->pushButton->setEnabled(true);
    qDebug() << "All tags loaded.";
    m_view->setSortingEnabled(true);
}

void PlaylistTable::onTagLoaded(const QString& filePath, const AudioTagInfo& info)
{
    if (!m_model)
        return;
    // Find the source row by matching the stored full path in UserRole + 1
    int matchRow = -1;
    for (int r = 0; r < m_model->rowCount(); ++r)
    {
        QStandardItem* item = m_model->item(r, 0); // filename column
        if (!item) continue;
        QString storedPath = m_model->item(r, 0)->data(Qt::UserRole + 1).toString();
        if (QFileInfo(storedPath).canonicalFilePath() == QFileInfo(filePath).canonicalFilePath())
        {
            matchRow = r;
            break;
        }
    }
    if (matchRow < 0)
        return; // not found
    // Update the other columns
    m_model->setData(m_model->index(matchRow, 3), info.iDuration);
    m_model->setData(m_model->index(matchRow, 4), info.sArtist);
    m_model->setData(m_model->index(matchRow, 5), info.sTitle);
    m_model->setData(m_model->index(matchRow, 6), info.sAlbum);
    m_model->setData(m_model->index(matchRow, 7), info.iTrackNum);
    m_model->setData(m_model->index(matchRow, 8), info.iYear);
    m_model->setData(m_model->index(matchRow, 9), info.sGenre);
    m_model->setData(m_model->index(matchRow, 10), info.sComment);
    m_model->setData(m_model->index(matchRow, 11), info.iBitrate);
    m_model->setData(m_model->index(matchRow, 12), info.iSamplerate);
    m_model->setData(m_model->index(matchRow, 13), info.iBits);
    m_model->setData(m_model->index(matchRow, 14), info.iChannels);
    m_model->setData(m_model->index(matchRow, 15), info.sFormat);
    m_model->setData(m_model->index(matchRow, 16), info.sCoverSize);
    m_model->setData(m_model->index(matchRow, 17), info.iFileSize);
}

void PlaylistTable::playlistLoadFinished()
{
    on_pushButton_clicked();
    //on_pushButton_2_clicked ();
    //setSectionsResizeMode();
    int iSortCol = settingsMgr->value("PlaylistViewSortColumn", 0).toInt();
    Qt::SortOrder order = static_cast<Qt::SortOrder>(settingsMgr->value("PlaylistViewSortColumnOrder", 0).toInt());
    // m_sortModel->sort(iSortCol, order);
    onHeaderSortChanged(iSortCol, order);
}

void PlaylistTable::addTrack(const QString &filePath)
{
    QString fileName = extractFileName(filePath);
    QFileInfo info(filePath);
    // --- Default icon ---
    QIcon icon(":/img/img/icons8-music-48.png");
    // --- Create standard items for each column ---
    QStandardItem* fileItem = new QStandardItem(icon, fileName);
    fileItem->setData(info.canonicalFilePath(), Qt::UserRole + 1); // store full path
    QStandardItem* extensionItem = new QStandardItem(info.suffix());
    QStandardItem* pathItem = new QStandardItem(info.canonicalPath());
    QStandardItem* durationItem = new QStandardItem(QString::number(0));
    QStandardItem* artistItem = new QStandardItem("");
    QStandardItem* titleItem = new QStandardItem("");
    QStandardItem* albumItem = new QStandardItem("");
    QStandardItem* trackItem = new QStandardItem("");
    QStandardItem* yearItem = new QStandardItem("");
    QStandardItem* genreItem = new QStandardItem("");
    QStandardItem* commentItem = new QStandardItem("");
    QStandardItem* bitrateItem = new QStandardItem("");
    QStandardItem* samplerateItem = new QStandardItem("");
    QStandardItem* bitsItem = new QStandardItem("");
    QStandardItem* channelsItem = new QStandardItem("");
    QStandardItem* formatItem = new QStandardItem("");
    QStandardItem* coverSizeItem = new QStandardItem("");
    QStandardItem* fileSizeItem = new QStandardItem("");
    // Build row
    QList<QStandardItem *> rowItems;
    rowItems << fileItem << extensionItem << pathItem << durationItem
        << artistItem << titleItem << albumItem << trackItem
        << yearItem << genreItem << commentItem << bitrateItem
        << samplerateItem << bitsItem << channelsItem << formatItem
        << coverSizeItem << fileSizeItem;
    m_model->appendRow(rowItems);
    // --- Add to playlist ---
    m_playlist->addMedia(QUrl::fromLocalFile(filePath));
    // --- Align duration column (index 3) ---
    m_model->item(m_model->rowCount() - 1, 3)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

void PlaylistTable::setSectionsResizeMode()
{
    m_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(10, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(11, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(12, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(13, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(14, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(15, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(16, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(17, QHeaderView::ResizeToContents);
}

void PlaylistTable::clear()
{
    m_model->clear();
    m_model->setHorizontalHeaderLabels(QStringList()
        << "Filename" << "Ext" << "Path" << "Duration" << "Artist"
        << "Title" << "Album" << "Track" << "Year" << "Genre"
        << "Comment" << "Bitrate" << "Samplerate" << "Bits" << "Channels"
        << "Format" << "Cover size" << "File size"); m_playlist->clear();
    // setSectionsResizeMode();
}

QString PlaylistTable::extractFileName(const QString &filePath)
{
    QFileInfo info(filePath);
    return info.completeBaseName();
}

void PlaylistTable::onDoubleClicked(const QModelIndex &index)
{
    // qDebug() << __PRETTY_FUNCTION__ << "index.row()" << index.row();
    int row = index.row();
    m_playlist->setCurrentIndex(row);
    //m_player->play();
    onCurrentTrackChanged(row);
    emit trackActivated(row);
    // qDebug() << __PRETTY_FUNCTION__ << "proxy row:" << index.row();
    //    // Map proxy index to source model index
    // QModelIndex sourceIndex = m_sortModel->mapToSource(index);
    // if (!sourceIndex.isValid())
    // return;
    // int sourceRow = sourceIndex.row();
    // qDebug() << __PRETTY_FUNCTION__ << "source row:" << sourceRow;
    //    // If playlist order mirrors source model order, setCurrentIndex(sourceRow)
    // m_playlist->setCurrentIndex(sourceRow);
    // onCurrentTrackChanged(sourceRow);
    //    //m_player->play();
    // emit trackActivated(sourceRow);
}

void PlaylistTable::onCurrentTrackChanged(int index)
{
    qDebug() << __PRETTY_FUNCTION__ << "index:" << index;
    if (index < 0 || !m_playlist)
        return;
    // --- 1. Get the URL of the currently playing track ---
    QMediaContent media = m_playlist->media(index);
    if (media.isNull())
        return;
    QString currentPath = media.canonicalUrl().toLocalFile();
    if (currentPath.isEmpty())
        return;
    // --- 2. Reset icons for all rows ---
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        QStandardItem *item = m_model->item(i, 0); // "Filename" column
        if (item)
            item->setIcon(QIcon(":/img/img/icons8-music-48.png"));
    }
    // --- 3. Find the row in m_model with matching full path (column 18) ---
    int matchRow = -1;
    for (int row = 0; row < m_model->rowCount(); ++row)
    {
        QString path = m_model->data(m_model->index(row, 18)).toString();
        if (path.compare(currentPath, Qt::CaseInsensitive) == 0)
        {
            matchRow = row;
            break;
        }
    }
    // --- 4. If found, update icon and selection ---
    if (matchRow >= 0)
    {
        QStandardItem *currentItem = m_model->item(matchRow, 0);
        if (currentItem)
            currentItem->setIcon(QIcon(":/img/img/icons8-play-48.png"));
        // Map model row to proxy (sorted) row for correct view position
        QModelIndex sourceIndex = m_model->index(matchRow, 0);
        QModelIndex proxyIndex = m_sortModel->mapFromSource(sourceIndex);
        // --- Optional: center and select the current track in view ---
        m_view->scrollTo(proxyIndex, QAbstractItemView::EnsureVisible);
        m_view->selectRow(proxyIndex.row());
    }
    // QModelIndex modeIndex = m_sortModel->index(index, 0);
    // m_view->scrollTo(modeIndex, QAbstractItemView::PositionAtCenter);
    // m_view->selectRow(index);
}

void PlaylistTable::on_pushButton_2_clicked()
{
    QHeaderView *header = m_view->horizontalHeader();
    header-> resizeSections(QHeaderView::ResizeToContents);
}

int PlaylistTable::mapSourceRowToProxy(QAbstractItemModel* sourceModel, QSortFilterProxyModel* proxyModel, int sourceRow)
{
    QModelIndex sourceIndex = sourceModel->index(sourceRow, 0);
    return proxyModel->mapFromSource(sourceIndex).row();
}

int PlaylistTable::mapProxyRowToSource(QSortFilterProxyModel* proxyModel, int proxyRow)
{
    if (!proxyModel)
        return -1;
    QModelIndex proxyIndex = proxyModel->index(proxyRow, 0); // column 0 is arbitrary, just needs a valid index
    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    return sourceIndex.isValid() ? sourceIndex.row() : -1;
}
