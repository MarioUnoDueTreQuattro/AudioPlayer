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
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
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
    m_model->setHorizontalHeaderLabels(QStringList() << "Filename" << "Ext" << "Path" << "Duration" << "Artist" << "Title" << "Album" << "Track" << "Year" << "Genre" << "Comment" << "Bitrate" << "Samplerate" << "Bits" << "Channels" << "Format" << "Cover size" << "File size");
    // --- Create sort proxy ---
    m_sortModel = new PlaylistSortModel(this);
    m_sortModel->setSourceModel(m_model);
    //    // --- Create view ---
    // m_view = new QTableView(this);
    // m_view->setModel(m_sortModel);
    // m_view->horizontalHeader()->setStretchLastSection(true);
    // --- Create table view ---
    m_view = ui->tableView; // new QTableView(this);
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
    QMediaPlaylist *newPlaylist = new QMediaPlaylist(this);
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
        //qDebug() << path;
        newPlaylist->addMedia(QUrl::fromLocalFile(path));
    }
    int oldIndex = m_playlist->currentIndex();
    int newCount = newPlaylist->mediaCount();
    delete m_playlist;
    m_playlist = newPlaylist;
    m_player->setPlaylist(m_playlist);
    connect(m_playlist, SIGNAL(currentIndexChanged(int)),
        this, SLOT(onCurrentTrackChanged(int)));
    if (oldIndex >= 0 && oldIndex < newCount)
        m_playlist->setCurrentIndex(oldIndex);
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
    QAbstractItemModel* model = m_view->model();
    if (!model)
        return;
    int iCount = model->rowCount();
    QStringList fileList;
    QString sFileName;
    QModelIndex index;
    m_FilePathToRow.clear();
    for (int idx = 0; idx < iCount; idx++)
    {
        index = m_sortModel->index(idx, 2); // column 2 = Path
        sFileName = m_sortModel->data(index).toString() + "/";
        index = m_sortModel->index(idx, 0); // column 0 = file name
        sFileName.append(m_sortModel->data(index).toString());
        sFileName.append(".");
        index = m_sortModel->index(idx, 1); // column 1 = extension
        sFileName.append(m_sortModel->data(index).toString());
        //qDebug() << sFileName;
        fileList.append(sFileName);
        m_FilePathToRow.insert(sFileName, idx); // map filePath to row
    }
    // Run the tag loading in a separate thread
    m_tagWorker = new TagLoaderWorker();
    QFuture<void> future = QtConcurrent::run(m_tagWorker, &TagLoaderWorker::processFiles, fileList);
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(m_tagWorker, SIGNAL(finished()), this, SLOT(onTagLoadingFinished()));
    watcher->setFuture(future);
    connect(m_tagWorker, SIGNAL(tagLoaded(QString, AudioTagInfo)), this, SLOT(onTagLoaded(QString, AudioTagInfo)));
}

void PlaylistTable::onTagLoadingFinished()
{
    delete m_tagWorker;
    m_tagWorker = nullptr;
    ui->pushButton->setEnabled(true);
    qDebug() << "All tags loaded.";
    m_view->setSortingEnabled(true);
}

void PlaylistTable::onTagLoaded(const QString& filePath, const AudioTagInfo& info)
{
    if (!m_FilePathToRow.contains(filePath))
        return;
    int row = m_FilePathToRow.value(filePath);
    QAbstractItemModel* model = m_view->model();
    if (!model)
        return;
    model->setData(model->index(row, 3), info.iDuration);
    model->setData(model->index(row, 4), info.sArtist);
    model->setData(model->index(row, 5), info.sTitle);
    model->setData(model->index(row, 6), info.sAlbum);
    model->setData(model->index(row, 7), info.iTrackNum);
    model->setData(model->index(row, 8), info.iYear);
    model->setData(model->index(row, 9), info.sGenre);
    model->setData(model->index(row, 10), info.sComment);
    model->setData(model->index(row, 11), info.iBitrate);
    model->setData(model->index(row, 12), info.iSamplerate);
    model->setData(model->index(row, 13), info.iBits);
    model->setData(model->index(row, 14), info.iChannels);
    model->setData(model->index(row, 15), info.sFormat);
    model->setData(model->index(row, 16), info.sCoverSize);
    model->setData(model->index(row, 17), info.iFileSize);
    //qDebug() << "Tag for" << filePath << ":" << info.sTitle << info.sArtist << info.sAlbum;
}

void PlaylistTable::addTrackExt(const QString &filePath)
{
    QString fileName = extractFileName(filePath);
    QFileInfo info(filePath);
    // QMediaPlayer tempPlayer;
    // tempPlayer.setMedia(QUrl::fromLocalFile(filePath));
    // tempPlayer.play ();
    // tempPlayer.stop();
    // Load a default icon (could be a note or file-type icon)
    QIcon icon(":/img/img/icons8-music-48.png"); // resource path or file path
    AudioTag tag(filePath);
    int durationMs = tag.tagInfo().iDuration;  //0;//tempPlayer.duration(); // returns 0 if not yet loaded
    double durationSec = durationMs ;/// 1000.0;
    QList<QStandardItem *> rowItems;
    QStandardItem *fileItem = new QStandardItem(icon, fileName);
    QStandardItem *extensionItem = new QStandardItem(info.suffix());  // placeholder
    QStandardItem *pathItem = new QStandardItem(info.canonicalPath());
    QStandardItem *durationItem = new QStandardItem(QString::number(durationSec)); // placeholder
    QStandardItem *ArtistItem = new QStandardItem(tag.tagInfo().sArtist);
    rowItems << fileItem << extensionItem << pathItem << durationItem << ArtistItem;
    m_model->appendRow(rowItems);
    // m_model->insertRow (m_model->rowCount (),rowItems);
    m_playlist->addMedia(QUrl::fromLocalFile(filePath));
}

void PlaylistTable::playlistLoadFinished()
{
    on_pushButton_clicked();
    //on_pushButton_2_clicked ();
    //setSectionsResizeMode();
}

void PlaylistTable::addTrack(const QString &filePath)
{
    QString fileName = extractFileName(filePath);
    QFileInfo info(filePath);
    // QMediaPlayer tempPlayer;
    // tempPlayer.setMedia(QUrl::fromLocalFile(filePath));
    // tempPlayer.play ();
    // tempPlayer.stop();
    // Load a default icon (could be a note or file-type icon)
    QIcon icon(":/img/img/icons8-music-48.png"); // resource path or file path
    // AudioTag tag(filePath);
    // int durationMs = tag.tagInfo().iDuration;  //0;//tempPlayer.duration(); // returns 0 if not yet loaded
    // double durationSec = durationMs ;/// 1000.0;
    QList<QStandardItem *> rowItems;
    QStandardItem *fileItem = new QStandardItem(icon, fileName);
    QStandardItem *extensionItem = new QStandardItem(info.suffix());  // placeholder
    QStandardItem *pathItem = new QStandardItem(info.canonicalPath());
    QStandardItem *durationItem = new QStandardItem(QString::number(0)); // placeholder
    QStandardItem *ArtistItem = new QStandardItem("");
    rowItems << fileItem << extensionItem << pathItem << durationItem << ArtistItem;
    m_model->appendRow(rowItems);
    // m_model->insertRow (m_model->rowCount (),rowItems);
    m_playlist->addMedia(QUrl::fromLocalFile(filePath));
    // m_view->horizontalHeader()->setDefaultAlignment(Qt::AlignRight);
    // m_view->setTextElideMode(Qt::ElideRight);
    // for (int row = 0; row < m_model->rowCount(); ++row)
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
    m_model->setHorizontalHeaderLabels(QStringList() << "Filename" << "Ext" << "Path" << "Dur" << "Artist" << "Title" << "Album" << "Track" << "Year" << "Genre" << "Comment" << "Bitrate" << "Samplerate" << "Bits" << "Channels" << "Format" << "Cover size" << "File size");
    m_playlist->clear();
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
    qDebug() << __PRETTY_FUNCTION__;
    if (index < 0)
        return;
    // Reset icons
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        QStandardItem *item = m_model->item(i, 0); // Title column
        item->setIcon(QIcon(":/img/img/icons8-music-48.png"));      // resource path or file path
    }
    if (index >= 0 && index < m_model->rowCount())
    {
        QStandardItem *currentItem = m_model->item(index, 0);
        currentItem->setIcon(QIcon(":/img/img/icons8-play-48.png"));
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
