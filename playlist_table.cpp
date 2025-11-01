#include "playlist_table.h"
#include "database_manager.h"
#include "playlist_delegates.h"
#include "ui_playlist_table.h"
#include "utility.h"
#include <QDebug>
#include <QFileInfo>
#include <QHeaderView>
#include <QVBoxLayout>
//#include <QSettings>
#include <QApplication>
#include <QBrush>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QTimer>
//#include "widget.h"

//#include <QItemSelectionModel>
//#include <QDebug>

//PlaylistTable::PlaylistTable(QWidget *parent) :
// QWidget(parent),
// ui(new Ui::PlaylistTable),
// m_player(player)
//{
// ui->setupUi(this);
//}

PlaylistTable::PlaylistTable(QMediaPlayer *player, QMediaPlaylist *playlist, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PlaylistTable),
      m_player(player),
      m_playlist(playlist),
      m_CurrentItem(nullptr),
      m_CurrentPlaylistItem(nullptr),
      m_bHasBeenSorted(false),
      m_bSessionPlaylistIsShown(true)
      /*,
                              m_findCurrentIndex (-1)*/
{
    qRegisterMetaType<AudioTagInfo>("AudioTagInfo");
    ui->setupUi(this);
    setupToolButton();
    m_tagWorker = nullptr;
    m_FutureWatcher = nullptr;
    m_stagedPlaylist = new QMediaPlaylist(this);
    ui->comboBoxFind->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui->comboBoxFilter->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    setWindowFlags(Qt::Tool);
    setWindowTitle(qApp->applicationName() + " playlist");
    settingsMgr = SettingsManager::instance();
    // --- Create playlist ---
    //m_playlist = new QMediaPlaylist(this);
    //m_player->setPlaylist(m_playlist);
    // --- Create source model ---
    m_model = new QStandardItemModel(this);
    m_model->setColumnCount(ColumnIndex::ColumnCount);
    for (int i = 0; i < ColumnIndex::ColumnCount; ++i)
        m_model->setHeaderData(i, Qt::Horizontal, ColumnIndex::headerName(i));
    // m_model->setHorizontalHeaderLabels(QStringList()
    // << "Filename" << "Ext" << "Path" << "Duration" << "Artist"
    // << "Title" << "Album" << "Track" << "Year" << "Genre"
    // << "Comment" << "Bitrate" << "Samplerate" << "Bits" << "Channels"
    // << "Format" << "Cover size" << "File size");
    // --- Create sort proxy ---
    m_sortModel = new PlaylistSortModel(this);
    m_sortModel->setSourceModel(m_model);
    m_sortModel->setFilterColumns(QSet<int> {0, 2, 4, 5, 6});
    m_view = ui->tableView;
    m_HorizontalHeader = new ElidedHeaderView(Qt::Horizontal, m_view);
    m_view->setHorizontalHeader(m_HorizontalHeader);
    // m_view->horizontalHeader()->setHighlightSections(false);
    // m_view->horizontalHeader()->setTextElideMode(Qt::ElideRight);
    m_HorizontalHeader->setSectionsClickable(true);
    m_HorizontalHeader->setHighlightSections(false);
    m_HorizontalHeader->setTextElideMode(Qt::ElideRight);
    m_HorizontalHeader->setDefaultAlignment(Qt::AlignHCenter);
    m_view->setModel(m_sortModel);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setWordWrap(false);
    //m_view->setSortingEnabled(true);
    //m_view->horizontalHeader()->setSortIndicatorShown(true);
    m_view->setShowGrid(false);
    m_view->setAlternatingRowColors(true);
    // m_view->horizontalHeader()->setSectionsClickable(true);
    // --- Create and assign duration delegate ---
    //for (int col = 0; col < m_view->model()->columnCount(); ++col) {
    // m_view->setItemDelegateForColumn(col, new PlaylistDelegate(m_view));
    //}
    PlaylistDelegate *delegate = new PlaylistDelegate(m_view);
    for (int col = 0; col < m_view->model()->columnCount(); ++col)
    {
        m_view->setItemDelegateForColumn(col, delegate);
    }
    PlaylistDurationDelegate *durationDelegate = new PlaylistDurationDelegate(this);
    m_view->setItemDelegateForColumn(3, durationDelegate);
    PlaylistFileSizeDelegate *FileSizeDelegate = new PlaylistFileSizeDelegate(this);
    m_view->setItemDelegateForColumn(17, FileSizeDelegate);
    PlaylistRatingDelegate *ratingDelegate = new PlaylistRatingDelegate(this);
    m_view->setItemDelegateForColumn(ColumnIndex::Rating, ratingDelegate);
    // QHeaderView *header = m_view->horizontalHeader();
    // header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    // header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    // header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    // header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    // header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    // header->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_HorizontalHeader->setContextMenuPolicy(Qt::CustomContextMenu);
    loadSearchHistory();
    loadFilterHistory();
    setSignalsConnections();
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

void PlaylistTable::onTrackFinishedOrStopped(QMediaPlayer::State newState)
{
    LOG_MSG_SHORT(newState);
    return;
    // Esegui la sostituzione SOLO quando il player è nello stato STOP
    if (newState == QMediaPlayer::StoppedState && m_stagedPlaylist->mediaCount() > 0)
    {
        // Esegui la sostituzione:
        // 1. Disconnetti il vecchio segnale
        disconnect(m_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentTrackChanged(int)));
        // 2. Sostituisci la playlist corrente con la temporanea
        //QMediaPlaylist *oldPlaylist = m_playlist;
        m_playlist = m_stagedPlaylist;
        //m_stagedPlaylist = nullptr; // Rimuovi il riferimento temporaneo
        m_player->setPlaylist(m_playlist);
        //delete oldPlaylist;
        // 3. Riconnetti il nuovo segnale
        connect(m_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentTrackChanged(int)));
        emit playlistUpdated(m_playlist);
        // 4. Se la riproduzione è stata avviata dopo lo stop, fai ripartire la prima traccia
        // (O implementa la logica di riavvio desiderata, ma senza setPosition!)
        // Ad esempio: m_player->play();
    }
}

void PlaylistTable::setSignalsConnections()
{
    connect(m_player, SIGNAL(stateChanged(QMediaPlayer::State)),
        this, SLOT(onTrackFinishedOrStopped(QMediaPlayer::State)));
    connect(m_view, &QTableView::customContextMenuRequested, this, &PlaylistTable::showPlaylistContextMenu);
    connect(ui->pushButtonClearFind, &QPushButton::clicked, this, &PlaylistTable::clearSearchHighlight);
    connect(ui->pushButtonPrev, &QPushButton::clicked, this, &PlaylistTable::findPrevious);
    connect(ui->pushButtonNext, &QPushButton::clicked, this, &PlaylistTable::findNext);
    connect(ui->comboBoxFind, &QComboBox::currentTextChanged, this, &PlaylistTable::findInTable);
    connect(ui->comboBoxFilter, &QComboBox::currentTextChanged, m_sortModel, &PlaylistSortModel::setFilterText);
    connect(ui->comboBoxFind->lineEdit(), &QLineEdit::returnPressed, this, &PlaylistTable::handleNewSearchInput);
    connect(ui->comboBoxFilter->lineEdit(), &QLineEdit::returnPressed, this, &PlaylistTable::handleNewFilterInput);
    // connect(m_view->horizontalHeader(), &QHeaderView::sectionResized, this, &PlaylistTable::onColumnResized);
    // connect(m_view->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &PlaylistTable::onHeaderContextMenu);
    // connect(m_view->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(onHeaderSortChanged(int, Qt::SortOrder)));
    connect(m_HorizontalHeader, &QHeaderView::sectionResized, this, &PlaylistTable::onColumnResized);
    connect(m_HorizontalHeader, &QHeaderView::customContextMenuRequested, this, &PlaylistTable::onHeaderContextMenu);
    connect(m_HorizontalHeader, SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(onHeaderSortChanged(int, Qt::SortOrder)));
    connect(m_view, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onDoubleClicked(const QModelIndex &)));
    connect(m_view, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onClicked(const QModelIndex &)));
    connect(m_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentTrackChanged(int)));
}

PlaylistTable::~PlaylistTable()
{
    qDebug() << __PRETTY_FUNCTION__;
    delete ui;
}

void PlaylistTable::closeEvent(QCloseEvent *event)
{
    if (m_tagWorker)
    {
        m_tagWorker->stop(); // signal worker to abort
        if (m_FutureWatcher)
        {
            m_FutureWatcher->waitForFinished(); // wait for thread to finish
        }
    }
    // if (m_FutureWatcher != nullptr)
    // {
    // if (m_FutureWatcher->isRunning())
    // {
    //           // m_tagWorker->m_bStop = true;
    // QFuture<void> future = m_FutureWatcher->future();
    // future.cancel();
    // future.waitForFinished();
    // }
    // m_FutureWatcher->waitForFinished();
    // delete m_tagWorker;
    // m_tagWorker = nullptr;
    // }
    // if (m_tagWorker != nullptr) delete m_tagWorker;
    if (m_tagWorker)
    {
        disconnect(m_tagWorker, nullptr, this, nullptr);
        m_tagWorker->deleteLater();
        m_tagWorker = nullptr;
    }
    if (m_FutureWatcher)
    {
        m_FutureWatcher->deleteLater();
        m_FutureWatcher = nullptr;
    }
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
    //qDebug() << __PRETTY_FUNCTION__;
    QWidget::moveEvent(event);
    settingsMgr->setValue("PlaylistViewPosition", pos());
}

void PlaylistTable::resizeEvent(QResizeEvent *event)
{
    //qDebug() << __PRETTY_FUNCTION__;
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

//void PlaylistTable::onHeaderContextMenu(const QPoint &pos)
//{
// int column = m_view->horizontalHeader()->logicalIndexAt(pos);
// if (column < 0) return;
// QMenu menu;
// QAction *toggleAction = menu.addAction(m_view->isColumnHidden(column)
// ? tr("Show column")
// : tr("Hide column"));
// QMenu *columnMenu = menu.addMenu(tr("Columns"));
// for (int col = 0; col < m_model->columnCount(); ++col)
// {
// QString title = m_model->headerData(col, Qt::Horizontal).toString();
// QAction *action = columnMenu->addAction(title);
// action->setCheckable(true);
// action->setChecked(!m_view->isColumnHidden(col));
// action->setData(col); // store column index
// }
// QAction *selectedColumns = columnMenu->exec(m_view->horizontalHeader()->mapToGlobal(pos));
// if (selectedColumns && selectedColumns->data().isValid())
// {
// int col = selectedColumns->data().toInt();
// bool bVisible = !m_view->isColumnHidden(col);
// m_view->setColumnHidden(col, bVisible);
// settingsMgr->beginGroup("Table");
// settingsMgr->setValue(QString("Column_IsVisible_%1").arg(col), !bVisible);
// settingsMgr->endGroup();
// }
// QAction *selected = menu.exec(m_view->horizontalHeader()->mapToGlobal(pos));
// if (selected == toggleAction)
// {
// bool bVisible = !m_view->isColumnHidden(column);
// m_view->setColumnHidden(column, bVisible);
// settingsMgr->beginGroup("Table");
// settingsMgr->setValue(QString("Column_IsVisible_%1").arg(column), !bVisible);
// settingsMgr->endGroup();
// }
//}

void PlaylistTable::onHeaderContextMenu(const QPoint &pos)
{
    int column = m_view->horizontalHeader()->logicalIndexAt(pos);
    if (column < 0) return;
    QMenu menu;
    // Toggle current column visibility
    QAction *toggleAction = menu.addAction(m_view->isColumnHidden(column)
        ? tr("Show column")
        : tr("Hide column"));
    toggleAction->setData(column);
    // Submenu for all columns
    QMenu *columnMenu = new QMenu(tr("Columns"), &menu);
    QList<QAction *> columnActions;
    for (int col = 0; col < m_model->columnCount(); ++col)
    {
        QString title = m_model->headerData(col, Qt::Horizontal).toString();
        QAction *action = columnMenu->addAction(title);
        action->setCheckable(true);
        action->setChecked(!m_view->isColumnHidden(col));
        action->setData(col);
        columnActions.append(action);
    }
    menu.addMenu(columnMenu);
    QAction *selected = menu.exec(m_view->horizontalHeader()->mapToGlobal(pos));
    if (!selected) return;
    int col = selected->data().toInt();
    if (!selected->data().isValid()) return;
    bool bVisible = !m_view->isColumnHidden(col);
    m_view->setColumnHidden(col, bVisible);
    settingsMgr->beginGroup("Table");
    settingsMgr->setValue(QString("Column_IsVisible_%1").arg(col), !bVisible);
    settingsMgr->endGroup();
}

void PlaylistTable::restoreColumnVisibility()
{
    //settingsMgr->beginGroup("Table");
    for (int col = 0; col < m_view->model()->columnCount(); ++col)
    {
        bool bVisible = settingsMgr->value(QString("Table/Column_IsVisible_%1").arg(col), true).toBool();
        m_view->setColumnHidden(col, !bVisible);
    }
    //settingsMgr->endGroup();
}

void PlaylistTable::onColumnResized(int column, int oldSize, int newSize)
{
    // settingsMgr->beginGroup("Table");
    QString sSettingsGroup = settingsMgr->getSettings().group();
    if (sSettingsGroup != "")
    {
        //settingsMgr->endGroup();
        LOG_VAR(sSettingsGroup);
    }
    settingsMgr->setValue(QString("Table/Column_Width_%1").arg(column), newSize);
    // settingsMgr->endGroup();
}

void PlaylistTable::restoreColumnWidths()
{
    for (int col = 0; col < m_model->columnCount(); ++col)
    {
        // settingsMgr->beginGroup("Table");
        int width = settingsMgr->value(QString("Table/Column_Width_%1").arg(col), ColumnIndex::defaultWidth(col)).toInt();
        //int width = settingsMgr->value(QString("Table/Column_Width_%1").arg(col), -1).toInt();
        if (width > 0)
            m_view->setColumnWidth(col, width);
        // settingsMgr->endGroup();
    }
}

void PlaylistTable::loadsettings()
{
    QByteArray geometry = settingsMgr->value("PlaylistViewGeometry").toByteArray();
    if (!geometry.isEmpty())
        restoreGeometry(geometry);
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
}

void PlaylistTable::syncPlaylistOrder_(int sortColumn, Qt::SortOrder order)
{
    LOG_MSG_SHORT("");
    // 1. SALVA STATO, URL E POSIZIONE
    bool wasPlaying = (m_player->state() == QMediaPlayer::PlayingState);
    QUrl currentUrl;
    qint64 currentPos = 0;
    int oldCurrentIndex = -1;
    if (m_playlist && m_playlist->currentIndex() >= 0)
    {
        oldCurrentIndex = m_playlist->currentIndex();
        currentUrl = m_playlist->media(oldCurrentIndex).canonicalUrl();
        currentPos = m_player->position();
    }
    // 2. ESEGUI L'ORDINAMENTO SUL MODELLO PROXY
    m_view->setModel(m_sortModel);
    int sortCol = m_HorizontalHeader->sortIndicatorSection();
    Qt::SortOrder sortOrder = m_HorizontalHeader->sortIndicatorOrder();
    m_sortModel->sort(sortCol, sortOrder);
    qApp->processEvents();
    if (currentUrl.isEmpty() || !m_playlist)
    {
        return;
    }
    // 3. CREA LA NUOVA LISTA DI URL ORDINATI E TROVA IL NUOVO INDICE
    QAbstractItemModel *viewModel = m_view->model();
    int rowCount = viewModel->rowCount();
    QList<QUrl> orderedUrls;
    int newCurrentIndexInOrderedList = -1;
    for (int i = 0; i < rowCount; ++i)
    {
        // Logica di costruzione del percorso
        QModelIndex pathIndex = viewModel->index(i, 2);
        QString path = viewModel->data(pathIndex).toString() + "/";
        pathIndex = viewModel->index(i, 0);
        path.append(viewModel->data(pathIndex).toString());
        path.append(".");
        pathIndex = viewModel->index(i, 1);
        path.append(viewModel->data(pathIndex).toString());
        QUrl newUrl = QUrl::fromLocalFile(path);
        orderedUrls.append(newUrl);
        if (newUrl == currentUrl)
        {
            newCurrentIndexInOrderedList = i;
        }
    }
    if (newCurrentIndexInOrderedList == -1)
    {
        LOG_MSG_SHORT("Traccia corrente non trovata dopo l'ordinamento!");
        m_player->stop();
        return;
    }
    // 4. ISOLAMENTO DELLA TRACCIA CORRENTE E RIORDINO DELLA PLAYLIST
    // Disconnetti per evitare che gli eventi di manipolazione attivino l'UI prematuramente
    disconnect(m_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentTrackChanged(int)));
    // a. Sposta la traccia in riproduzione all'indice 0 della playlist.
    m_playlist->moveMedia(oldCurrentIndex, 0);
    int currentMediaIndex = 0;
    // b. Rimuovi TUTTI i media successivi alla traccia corrente (indice > 0)
    for (int i = m_playlist->mediaCount() - 1; i > currentMediaIndex; --i)
    {
        m_playlist->removeMedia(i);
    }
    // c. Aggiungi i nuovi media PRIMA della traccia corrente
    for (int i = 0; i < newCurrentIndexInOrderedList; ++i)
    {
        m_playlist->insertMedia(i, orderedUrls.at(i));
    }
    // d. Aggiungi i nuovi media DOPO la traccia corrente
    for (int i = newCurrentIndexInOrderedList + 1; i < rowCount; ++i)
    {
        m_playlist->addMedia(orderedUrls.at(i));
    }
    // Nota: Non chiamiamo setCurrentIndex qui, ma nel blocco asincrono (punto 6).
    // 5. AGGIORNAMENTI UI (Sincrono)
    m_view->setTextElideMode(Qt::ElideRight);
    for (int row = 0; row < m_model->rowCount(); ++row)
    {
        m_model->item(row, ColumnIndex::Duration)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_model->item(row, ColumnIndex::FileSize)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    emit playlistUpdated(m_playlist);
    // 6. RIPRISTINA LO STATO E LA POSIZIONE (ASINCRONO)
    // Aggiorna la selezione della riga nella vista (QTableView) - Sincrono
    QModelIndex proxyIndex = m_sortModel->index(newCurrentIndexInOrderedList, 0);
    m_view->scrollTo(proxyIndex, QAbstractItemView::EnsureVisible);
    m_view->selectRow(newCurrentIndexInOrderedList);
    // Ripristina lo stato di riproduzione in modo ASINCRONO
    if (wasPlaying)
    {
        QTimer::singleShot(0, [this, newCurrentIndexInOrderedList, currentPos]()
        {
            // Se si è interrotto:
            if (m_player->state() != QMediaPlayer::PlayingState)
            {
                // ** CORREZIONE CRITICA: Impostiamo l'indice qui **
                m_playlist->setCurrentIndex(newCurrentIndexInOrderedList);
                m_player->setPosition(currentPos);
                m_player->play();
            }
        });
    }
    else
    {
        // Se non era in riproduzione, assicurati che il player sia all'indice corretto
        m_playlist->setCurrentIndex(newCurrentIndexInOrderedList);
        m_player->stop();
    }
    // Riconnetti il segnale
    //connect(m_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentTrackChanged(int)));
}

void PlaylistTable::syncStagedPlaylistOrder(int sortColumn, Qt::SortOrder order)
{
    LOG_MSG_SHORT("");
    // 1. Esegui l'ordinamento sul modello proxy
    m_view->setModel(m_sortModel);
    int sortCol = m_HorizontalHeader->sortIndicatorSection();
    Qt::SortOrder sortOrder = m_HorizontalHeader->sortIndicatorOrder();
    m_sortModel->sort(sortCol, sortOrder);
    qApp->processEvents();
    // 2. CREA LA PLAYLIST ORDINATA TEMPORANEA
    m_stagedPlaylist->clear();
    QAbstractItemModel *viewModel = m_view->model();
    for (int i = 0; i < viewModel->rowCount(); ++i)
    {
        // ... (La tua logica per costruire il percorso URL) ...
        QModelIndex pathIndex = viewModel->index(i, 2);
        QString path = viewModel->data(pathIndex).toString() + "/";
        pathIndex = viewModel->index(i, 0);
        path.append(viewModel->data(pathIndex).toString());
        path.append(".");
        pathIndex = viewModel->index(i, 1);
        path.append(viewModel->data(pathIndex).toString());
        m_stagedPlaylist->addMedia(QUrl::fromLocalFile(path));
    }
    // 4. AGGIORNAMENTI UI (Solo la view, non il player)
    m_view->setTextElideMode(Qt::ElideRight);
    for (int row = 0; row < m_model->rowCount(); ++row)
    {
        m_model->item(row, ColumnIndex::Duration)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_model->item(row, ColumnIndex::FileSize)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    //emit playlistSorted (m_stagedPlaylist);
}

void PlaylistTable::syncPlaylistOrder(int sortColumn, Qt::SortOrder order)
{
    //qDebug() << "syncPlaylistOrder";
    LOG_MSG_SHORT("");
    m_view->setModel(m_sortModel);
    int sortCol = m_HorizontalHeader->sortIndicatorSection();
    Qt::SortOrder sortOrder = m_HorizontalHeader->sortIndicatorOrder();
    // Force re-sort now
    // m_view->horizontalHeader ()->setSortIndicator (sortCol,sortOrder);
    // m_view->horizontalHeader ()->setSortIndicatorShown (true);
    m_sortModel->sort(sortCol, sortOrder);
    // m_view->setModel (m_model);
    // m_model->sort (sortCol, sortOrder);
    // QEventLoop loop;
    // QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    // loop.exec(); // Blocks for 500 ms, but keeps UI responsive
    qApp->processEvents();
    // m_model=dynamic_cast<QStandardItemModel *>( m_sortModel);
    // m_sortModel= dynamic_cast<PlaylistSortModel *>( m_model);
    bool wasPlaying = (m_player->state() == QMediaPlayer::PlayingState);
    QUrl currentUrl;
    qint64 currentPos = 0;
    if (m_playlist && m_playlist->currentIndex() >= 0)
    {
        currentUrl = m_playlist->media(m_playlist->currentIndex()).canonicalUrl();
        currentPos = m_player->position();
    }
    QMediaPlaylist *newPlaylist = new QMediaPlaylist(this);
    newPlaylist->clear();
    // Keep old current media URL so we can restore the same track (if present)
    // TODO uncomment?
    /* QUrl oldCurrentUrl;
          int oldIndex = -1;
          if (m_playlist)
          {
              oldIndex = m_playlist->currentIndex();
              if (oldIndex >= 0 && oldIndex < m_playlist->mediaCount())
                  oldCurrentUrl = m_playlist->media(oldIndex).canonicalUrl();
          } */
    QAbstractItemModel *viewModel = m_view->model();
    for (int i = 0; i < viewModel->rowCount(); ++i)
    {
        QModelIndex pathIndex = viewModel->index(i, 2);
        QString path = viewModel->data(pathIndex).toString() + "/";
        pathIndex = viewModel->index(i, 0);
        path.append(viewModel->data(pathIndex).toString());
        path.append(".");
        pathIndex = viewModel->index(i, 1);
        path.append(viewModel->data(pathIndex).toString());
        newPlaylist->addMedia(QUrl::fromLocalFile(path));
    }
    // Iterate through the proxy model, which reflects sorted order
    // for (int i = 0; i < m_sortModel->rowCount(); ++i)
    // {
    // QModelIndex pathIndex = m_sortModel->index(i, 2); // column 2 = Path
    // QString path = m_sortModel->data(pathIndex).toString() + "/";
    // pathIndex = m_sortModel->index(i, 0);
    // path.append(m_sortModel->data(pathIndex).toString());
    // path.append(".");
    // pathIndex = m_sortModel->index(i, 1);
    // path.append(m_sortModel->data(pathIndex).toString());
    //        //path.append(".m4a");
    // qDebug() << path;
    // newPlaylist->addMedia(QUrl::fromLocalFile(path));
    // }
    // Replace playlist
    // disconnect old playlist signals if necessary
    if (m_playlist)
    {
        if (wasPlaying)
        {
            m_player->pause();
        }
        disconnect(m_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentTrackChanged(int)));
        delete m_playlist;
        m_playlist = nullptr;
    }
    m_playlist = newPlaylist;
    m_player->setPlaylist(m_playlist);
    // int oldIndex = m_playlist->currentIndex();
    int newCount = newPlaylist->mediaCount();
    // delete m_playlist;
    // m_playlist = newPlaylist;
    // m_player->setPlaylist(m_playlist);
    connect(m_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentTrackChanged(int)));
    // TODO uncomment?
    /*
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
    */
    // align
    //m_view->setColumnWidth(3, 80);
    //m_HorizontalHeader->setDefaultAlignment(Qt::AlignHCenter);
    m_view->setTextElideMode(Qt::ElideRight);
    for (int row = 0; row < m_model->rowCount(); ++row)
    {
        // m_model->item(row, 3)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        // m_model->item(row, 17)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_model->item(row, ColumnIndex::Duration)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_model->item(row, ColumnIndex::FileSize)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    //m_view->setModel (m_model);
    emit playlistUpdated(m_playlist);
    int matchIndex = -1;
    if (!currentUrl.isEmpty())
    {
        for (int i = 0; i < m_playlist->mediaCount(); ++i)
        {
            QUrl url = m_playlist->media(i).canonicalUrl();
            if (url == currentUrl)
            {
                matchIndex = i;
                break;
            }
        }
    }
    if (matchIndex >= 0)
    {
        m_playlist->setCurrentIndex(matchIndex);
        m_player->setPosition(currentPos);
        if (wasPlaying)
            m_player->play();
    }
    else
    {
        // No matching track found; stop playback
        m_player->stop();
    }
}

void PlaylistTable::delayedFindInTable()
{
    findInTable(ui->comboBoxFind->currentText());
}

void PlaylistTable::onHeaderSortChanged(int logicalIndex, Qt::SortOrder order)
{
    Q_UNUSED(logicalIndex);
    Q_UNUSED(order);
    m_iSortCol = logicalIndex;
    m_SortOrder = order;
    //if (m_bHasBeenSorted==false) return;
    //m_view->blockSignals(true);
    emit isSorting(true);
    //qDebug() << "onHeaderSortChanged logicalIndex" << logicalIndex;
    if (logicalIndex != -1)
    {
        settingsMgr->setValue("PlaylistViewSortColumn", logicalIndex);
        settingsMgr->setValue("PlaylistViewSortColumnOrder", order);
    }
    // m_view->horizontalHeader ()->setSortIndicator (logicalIndex,order);
    // m_view->horizontalHeader ()->setSortIndicatorShown (true);
    // After sorting the model, rebuild the QMediaPlaylist order.
    syncPlaylistOrder(logicalIndex, order);
    //syncStagedPlaylistOrder (logicalIndex, order);
    qDebug() << "Playlist sorted and reordered to match table view.";
    emit isSorting(false);
    //m_view->blockSignals(false);
    if (!ui->comboBoxFind->currentText().isEmpty() && ui->comboBoxFind->currentText().length() < MIN_SEARCH_CHARS == false) QTimer::singleShot(100, this, &PlaylistTable::delayedFindInTable);
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

//void PlaylistTable::on_pushButton_clicked()
//{
// ui->pushButton->setEnabled(false);
// m_view->setSortingEnabled(false);
// if (!m_sortModel)
// return;
// int rowCount = m_sortModel->rowCount();
// QStringList fileList;
//    // Iterate through the proxy model (sorted/displayed order)
// for (int idx = 0; idx < rowCount; ++idx)
// {
//        // Column 0 = Filename column; UserRole+1 stores the full path
// QModelIndex index = m_sortModel->index(idx, 0);
// QString fullPath = m_sortModel->data(index, Qt::UserRole + 1).toString();
// if (!fullPath.isEmpty())
// fileList.append(fullPath);
// }
//    // Run the tag loading in a separate thread
// m_tagWorker = new TagLoaderWorker();
// QFuture<void> future = QtConcurrent::run(m_tagWorker, &TagLoaderWorker::processFiles, fileList);
// m_FutureWatcher = new QFutureWatcher<void>(this);
// m_FutureWatcher->setFuture(future);
// connect(m_tagWorker, SIGNAL(finished()), this, SLOT(onTagLoadingFinished()));
// connect(m_tagWorker, SIGNAL(tagLoaded(QString, AudioTagInfo)),
// this, SLOT(onTagLoaded(QString, AudioTagInfo)));
//}

//void PlaylistTable::on_pushButton_clicked()
//{
// ui->pushButton->setEnabled(false);
// m_view->setSortingEnabled(false);
// QAbstractItemModel* model = m_view->model();
// if (!model)
// return;
// int iCount = model->rowCount();
// QStringList fileList;
// QString sFileName;
// QModelIndex index;
// m_FilePathToRow.clear();
// for (int idx = 0; idx < iCount; idx++)
// {
// index = m_sortModel->index(idx, 2); // column 2 = Path
// sFileName = m_sortModel->data(index).toString() + "/";
// index = m_sortModel->index(idx, 0); // column 0 = file name
// sFileName.append(m_sortModel->data(index).toString());
// sFileName.append(".");
// index = m_sortModel->index(idx, 1); // column 1 = extension
// sFileName.append(m_sortModel->data(index).toString());
// qDebug() << sFileName;
// fileList.append(sFileName);
// m_FilePathToRow.insert(sFileName, idx); // map filePath to row
// }
//    // Run the tag loading in a separate thread
// TagLoaderWorker* worker = new TagLoaderWorker();
// QFuture<void> future = QtConcurrent::run(worker, &TagLoaderWorker::processFiles, fileList);
// QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
// connect(watcher, SIGNAL(finished()), this, SLOT(onTagLoadingFinished()));
// watcher->setFuture(future);
// connect(worker, SIGNAL(tagLoaded(QString, AudioTagInfo)), this, SLOT(onTagLoaded(QString, AudioTagInfo)));
//}

//void PlaylistTable::on_pushButton_clicked()
//{
// ui->pushButton->setEnabled(false);
// m_view->setSortingEnabled(false);
//    // m_view->setModel(m_model);
// if (!m_sortModel)
// return;
// int rowCount = m_sortModel->rowCount();
// QStringList fileList;
// m_FilePathToRow.clear();
//    // Iterate sorted/displayed rows
// for (int idx = 0; idx < rowCount; ++idx)
// {
// QModelIndex index = m_sortModel->index(idx, 0); // column 0 = filename
// QString fullPath = m_sortModel->data(index, Qt::UserRole + 1).toString();
// if (!fullPath.isEmpty())
// {
// QModelIndex proxyIndex = m_sortModel->index(idx, 0);
// QString fullPath = m_sortModel->data(proxyIndex, Qt::UserRole + 1).toString();
// QModelIndex sourceIndex = m_sortModel->mapToSource(proxyIndex);
// int sourceRow = sourceIndex.row();
// m_FilePathToRow.insert(fullPath, sourceRow);
// fileList.append(fullPath);
// }
// }
//    // Run the tag loader in a separate thread
// m_tagWorker = new TagLoaderWorker();
// QFuture<void> future = QtConcurrent::run(m_tagWorker, &TagLoaderWorker::processFiles, fileList);
// m_FutureWatcher = new QFutureWatcher<void>(this);
// m_FutureWatcher->setFuture(future);
// connect(m_tagWorker, SIGNAL(finished()), this, SLOT(onTagLoadingFinished()));
// connect(m_tagWorker, SIGNAL(tagLoaded(QString, AudioTagInfo)),
// this, SLOT(onTagLoaded(QString, AudioTagInfo)));
//}
void PlaylistTable::readTags()
{
    ui->pushButton->setEnabled(false);
    m_view->setSortingEnabled(false);
    if (!m_sortModel)
        return;
    int proxyRowCount = m_sortModel->rowCount();
    QStringList fileList;
    m_FilePathToRow.clear();
    // Iterate through PROXY rows (displayed order)
    for (int proxyRow = 0; proxyRow < proxyRowCount; ++proxyRow)
    {
        // Get proxy index for column 0 (filename)
        QModelIndex proxyIndex = m_sortModel->index(proxyRow, 0);
        // Map proxy index to source index
        QModelIndex sourceIndex = m_sortModel->mapToSource(proxyIndex);
        if (!sourceIndex.isValid())
            continue;
        int sourceRow = sourceIndex.row();
        // Read data from SOURCE model
        QStandardItem *item = m_model->item(sourceRow, 0);
        if (!item)
            continue;
        QString fullPath = item->data(Qt::UserRole + 1).toString();
        bool bIsTagged = item->data(Qt::UserRole + 2).toBool();
        //if (bIsTagged==false) qDebug()<<"Not tagged"; else qDebug()<<"TAGGED";
        if (bIsTagged == false)
        {
            if (!fullPath.isEmpty())
            {
                // Map filePath to SOURCE row (not proxy row!)
                m_FilePathToRow.insert(fullPath, sourceRow);
                fileList.append(fullPath);
            }
        }
    }
    if (m_FilePathToRow.count() == 0)
    {
        ui->pushButton->setEnabled(true);
        m_bHasBeenSorted = true;
        m_view->setSortingEnabled(true);
    }
    else
    {
        // Start tag loader worker thread
        m_tagWorker = new TagLoaderWorker();
        QFuture<void> future = QtConcurrent::run(m_tagWorker,
                &TagLoaderWorker::processFiles,
                fileList);
        m_FutureWatcher = new QFutureWatcher<void>(this);
        m_FutureWatcher->setFuture(future);
        connect(m_tagWorker, SIGNAL(finished()),
            this, SLOT(onTagLoadingFinished()));
        connect(m_tagWorker, SIGNAL(tagLoaded(QString, AudioTagInfo)),
            this, SLOT(onTagLoaded(QString, AudioTagInfo)));
    }
}

void PlaylistTable::on_pushButton_clicked()
{
    readPlaylistTags();
    //readTags ();
}

void PlaylistTable::saveSessionPlaylist()
{
    //DatabaseManager::instance(). clearSessionPlaylist();
    QList<QString> fullFilePaths;
    int proxyRowCount = m_sortModel->rowCount();
    for (int proxyRow = 0; proxyRow < proxyRowCount; ++proxyRow)
    {
        // Get proxy index for column 0 (filename)
        QModelIndex proxyIndex = m_sortModel->index(proxyRow, 0);
        // Map proxy index to source index
        QModelIndex sourceIndex = m_sortModel->mapToSource(proxyIndex);
        if (!sourceIndex.isValid())
            continue;
        int sourceRow = sourceIndex.row();
        // Read data from SOURCE model
        QStandardItem *item = m_model->item(sourceRow, 0);
        if (!item)
            continue;
        QString fullPath = item->data(Qt::UserRole + 1).toString();
        //qDebug() << proxyRow << fullPath;
        if (!fullPath.isEmpty())
        {
            // Map filePath to SOURCE row (not proxy row!)
            fullFilePaths.append(fullPath);
        }
    }
    DatabaseManager::instance().saveSessionPlaylist(fullFilePaths);
}

void PlaylistTable::readPlaylistTags()
{
    ui->pushButton->setEnabled(false);
    m_view->setSortingEnabled(false);
    if (!m_sortModel)
        return;
    int proxyRowCount = m_sortModel->rowCount();
    QStringList fileList;
    m_FilePathToRow.clear();
    // Iterate through PROXY rows (displayed order)
    for (int proxyRow = 0; proxyRow < proxyRowCount; ++proxyRow)
    {
        // Get proxy index for column 0 (filename)
        QModelIndex proxyIndex = m_sortModel->index(proxyRow, 0);
        // Map proxy index to source index
        QModelIndex sourceIndex = m_sortModel->mapToSource(proxyIndex);
        if (!sourceIndex.isValid())
            continue;
        int sourceRow = sourceIndex.row();
        // Read data from SOURCE model
        QStandardItem *item = m_model->item(sourceRow, 0);
        if (!item)
            continue;
        QString fullPath = item->data(Qt::UserRole + 1).toString();
        if (!fullPath.isEmpty())
        {
            // Map filePath to SOURCE row (not proxy row!)
            m_FilePathToRow.insert(fullPath, sourceRow);
            fileList.append(fullPath);
        }
    }
    if (m_FilePathToRow.count() == 0)
    {
        ui->pushButton->setEnabled(true);
        m_bHasBeenSorted = true;
        m_view->setSortingEnabled(true);
    }
    else
    {
        // Start tag loader worker thread
        m_tagWorker = new TagLoaderWorker();
        QFuture<void> future = QtConcurrent::run(m_tagWorker,
                &TagLoaderWorker::processFiles,
                fileList);
        m_FutureWatcher = new QFutureWatcher<void>(this);
        m_FutureWatcher->setFuture(future);
        connect(m_tagWorker, SIGNAL(finished()),
            this, SLOT(onTagLoadingFinished()));
        connect(m_tagWorker, SIGNAL(tagLoaded(QString, AudioTagInfo)),
            this, SLOT(onTagLoaded(QString, AudioTagInfo)));
    }
}

void PlaylistTable::onTagLoadingFinished()
{
    LOG_MSG_SHORT("Read" << m_FilePathToRow.count() << "tags");
    ui->pushButton->setEnabled(true);
    //qDebug() << "All tags loaded.";
    // disconnect(m_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentTrackChanged(int)));
    m_bHasBeenSorted = true;
    m_view->setSortingEnabled(true);
    if (m_FilePathToRow.count() > 0)
    {
        delete m_tagWorker;
        m_tagWorker = nullptr;
        delete m_FutureWatcher;
        m_FutureWatcher = nullptr;
    }
    m_FilePathToRow.clear();
    // m_view->horizontalHeader()->setSortIndicator(m_iSortCol, m_SortOrder);
    // m_view->horizontalHeader()->setSortIndicatorShown(true);
    // connect(m_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentTrackChanged(int)));
    // After sorting the model, rebuild the QMediaPlaylist order.
    // QModelIndex proxyIndex = m_sortModel->index(mapSourceRowToProxy(m_model, m_sortModel, m_CurrentItem->row()), 0);
    // m_view->scrollTo(proxyIndex, QAbstractItemView::PositionAtCenter);//EnsureVisible);
}

//void PlaylistTable::onTagLoaded(const QString& filePath, const AudioTagInfo& info)
//{
////    static int icount;
////    icount++;
////    qDebug() icount << filePath;
// if (!m_model)
// return;
//    // Find the source row by matching the stored full path in UserRole + 1
// int matchRow = -1;
// for (int r = 0; r < m_model->rowCount(); ++r)
// {
// QStandardItem* item = m_model->item(r, 0); // filename column
// if (!item) continue;
// QString storedPath = m_model->item(r, 0)->data(Qt::UserRole + 1).toString();
// if (QFileInfo(storedPath).canonicalFilePath() == QFileInfo(filePath).canonicalFilePath())
// {
// matchRow = r;
// break;
// }
// }
// if (matchRow < 0)
// return; // not found
//    // Update the other columns
// m_model->setData(m_model->index(matchRow, 3), info.iDuration);
// m_model->setData(m_model->index(matchRow, 4), info.sArtist);
// m_model->setData(m_model->index(matchRow, 5), info.sTitle);
// m_model->setData(m_model->index(matchRow, 6), info.sAlbum);
// m_model->setData(m_model->index(matchRow, 7), info.iTrackNum);
// m_model->setData(m_model->index(matchRow, 8), info.iYear);
// m_model->setData(m_model->index(matchRow, 9), info.sGenre);
// m_model->setData(m_model->index(matchRow, 10), info.sComment);
// m_model->setData(m_model->index(matchRow, 11), info.iBitrate);
// m_model->setData(m_model->index(matchRow, 12), info.iSamplerate);
// m_model->setData(m_model->index(matchRow, 13), info.iBits);
// m_model->setData(m_model->index(matchRow, 14), info.iChannels);
// m_model->setData(m_model->index(matchRow, 15), info.sFormat);
// m_model->setData(m_model->index(matchRow, 16), info.sCoverSize);
// m_model->setData(m_model->index(matchRow, 17), info.iFileSize);
//}
//void PlaylistTable::onTagLoaded(const QString& filePath, const AudioTagInfo& info)
//{
// static int icount;
// icount++;
// qDebug() << icount << filePath;
// if (!m_FilePathToRow.contains(filePath))
// return;
// int row = m_FilePathToRow.value(filePath);
// QAbstractItemModel* model = m_view->model();
// if (!model)
// return;
// model->setData(model->index(row, 3), info.iDuration);
// model->setData(model->index(row, 4), info.sArtist);
// model->setData(model->index(row, 5), info.sTitle);
// model->setData(model->index(row, 6), info.sAlbum);
// model->setData(model->index(row, 7), info.iTrackNum);
// model->setData(model->index(row, 8), info.iYear);
// model->setData(model->index(row, 9), info.sGenre);
// model->setData(model->index(row, 10), info.sComment);
// model->setData(model->index(row, 11), info.iBitrate);
// model->setData(model->index(row, 12), info.iSamplerate);
// model->setData(model->index(row, 13), info.iBits);
// model->setData(model->index(row, 14), info.iChannels);
// model->setData(model->index(row, 15), info.sFormat);
// model->setData(model->index(row, 16), info.sCoverSize);
// model->setData(model->index(row, 17), info.iFileSize);
//    //qDebug() << "Tag for" << filePath << ":" << info.sTitle << info.sArtist << info.sAlbum;
//}
//void PlaylistTable::onTagLoaded(const QString& filePath, const AudioTagInfo& info)
//{
// if (!m_FilePathToRow.contains(filePath))
// return;
// int row = m_FilePathToRow.value(filePath);
//    // int proxyRow = m_FilePathToRow.value(filePath);
// QAbstractItemModel* model = m_view->model();
// if (!model)
// return;
// model->setData(model->index(row, 3), info.iDuration);
// model->setData(model->index(row, 4), info.sArtist);
// model->setData(model->index(row, 5), info.sTitle);
// model->setData(model->index(row, 6), info.sAlbum);
// model->setData(model->index(row, 7), info.iTrackNum);
// model->setData(model->index(row, 8), info.iYear);
// model->setData(model->index(row, 9), info.sGenre);
// model->setData(model->index(row, 10), info.sComment);
// model->setData(model->index(row, 11), info.iBitrate);
// model->setData(model->index(row, 12), info.iSamplerate);
// model->setData(model->index(row, 13), info.iBits);
// model->setData(model->index(row, 14), info.iChannels);
// model->setData(model->index(row, 15), info.sFormat);
// model->setData(model->index(row, 16), info.sCoverSize);
// model->setData(model->index(row, 17), info.iFileSize);
//    // // Update the proxy model directly
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 3), info.iDuration);
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 4), info.sArtist);
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 5), info.sTitle);
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 6), info.sAlbum);
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 7), info.iTrackNum);
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 8), info.iYear);
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 9), info.sGenre);
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 10), info.sComment);
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 11), info.iBitrate);
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 12), info.iSamplerate);
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 13), info.iBits);
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 14), info.iChannels);
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 15), info.sFormat);
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 16), info.sCoverSize);
//    // m_sortModel->setData(m_sortModel->index(proxyRow, 17), info.iFileSize);
//    // Optional: emit dataChanged for UI update
// QModelIndex topLeft = m_model->index(row, 3);
// QModelIndex bottomRight = m_model->index(row, 17);
// emit m_model->dataChanged(topLeft, bottomRight);
//}
void PlaylistTable::onTagLoaded(const QString &filePath, const AudioTagInfo &info)
{
    // Check if we have mapping for this file
    if (!m_FilePathToRow.contains(filePath))
        return;
    // Get SOURCE row index
    int sourceRow = m_FilePathToRow.value(filePath);
    // Validate source row
    if (sourceRow < 0 || sourceRow >= m_model->rowCount())
        return;
    // Update data in SOURCE model (not proxy!)
    // The proxy will automatically reflect these changes
    QStandardItem *item = m_model->item(sourceRow, 0);
    item->setData(true, Qt::UserRole + 2);
    // m_model->setData(m_model->index(sourceRow,  Qt::UserRole + 2), true);
    m_model->setData(m_model->index(sourceRow, 3), info.iDuration);
    m_model->setData(m_model->index(sourceRow, 4), info.sArtist);
    m_model->setData(m_model->index(sourceRow, 5), info.sTitle);
    m_model->setData(m_model->index(sourceRow, 6), info.sAlbum);
    m_model->setData(m_model->index(sourceRow, 7), info.iTrackNum);
    m_model->setData(m_model->index(sourceRow, 8), info.iYear);
    m_model->setData(m_model->index(sourceRow, 9), info.sGenre);
    m_model->setData(m_model->index(sourceRow, 10), info.sComment);
    m_model->setData(m_model->index(sourceRow, 11), info.iBitrate);
    m_model->setData(m_model->index(sourceRow, 12), info.iSamplerate);
    m_model->setData(m_model->index(sourceRow, 13), info.iBits);
    m_model->setData(m_model->index(sourceRow, 14), info.iChannels);
    m_model->setData(m_model->index(sourceRow, 15), info.sFormat);
    m_model->setData(m_model->index(sourceRow, 16), info.sCoverSize);
    m_model->setData(m_model->index(sourceRow, 17), info.iFileSize);
    m_model->setData(m_model->index(sourceRow, ColumnIndex::LastModified), info.sLastModified);
    m_model->setData(m_model->index(sourceRow, ColumnIndex::Rating), info.iRating);
    m_model->setData(m_model->index(sourceRow, ColumnIndex::PlayCount), info.iPlayCount);
    DatabaseManager &db = DatabaseManager::instance();
    AudioTagInfo updateInfo = info;
    // bool bUpdate = db.updateTrack(filePath, updateInfo);
    bool bUpdate = db.loadOrUpdateTrack(filePath, updateInfo);
    if (sourceRow % 100 == 0) qApp->processEvents();
    // Note: setData() automatically emits dataChanged signal
    // No need to manually emit dataChanged
}

void PlaylistTable::playlistLoadFinished()
{
    m_bSessionPlaylistIsShown = true;
    restoreColumnWidths();
    restoreColumnVisibility();
    // on_pushButton_clicked();
    readTags();
    //on_pushButton_2_clicked ();
    //setSectionsResizeMode();
    int iSortCol = settingsMgr->value("PlaylistViewSortColumn", 0).toInt();
    Qt::SortOrder order = static_cast<Qt::SortOrder>(settingsMgr->value("PlaylistViewSortColumnOrder", 0).toInt());
    //m_sortModel->sort(iSortCol, order);
    //if (m_view->horizontalHeader()->isSortIndicatorShown())
    // onHeaderSortChanged(iSortCol, order);
    // m_HorizontalHeader->setSortIndicator(iSortCol, order);
    // m_HorizontalHeader->setSortIndicatorShown(true);
    m_sortModel->sort(iSortCol, order);
    m_view->setSortingEnabled(true);
    m_view->horizontalHeader()->setSortIndicator(iSortCol, order);
    m_HorizontalHeader->setSortIndicatorShown(true);
}

void PlaylistTable::addFilesFinished()
{
    readTags();
    // int iSortCol = settingsMgr->value("PlaylistViewSortColumn", 0).toInt();
    // Qt::SortOrder order = static_cast<Qt::SortOrder>(settingsMgr->value("PlaylistViewSortColumnOrder", 0).toInt());
    //    //m_sortModel->sort(iSortCol, order);
    // onHeaderSortChanged(iSortCol, order);
}

void PlaylistTable::alignColumns(int iLastRow)
{
    m_model->item(iLastRow, ColumnIndex::Duration)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_model->item(iLastRow, ColumnIndex::FileSize)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_model->item(iLastRow, ColumnIndex::Bitrate)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_model->item(iLastRow, ColumnIndex::Bits)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_model->item(iLastRow, ColumnIndex::Channels)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_model->item(iLastRow, ColumnIndex::Samplerate)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_model->item(iLastRow, ColumnIndex::Track)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_model->item(iLastRow, ColumnIndex::Year)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_model->item(iLastRow, ColumnIndex::PlayCount)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

void PlaylistTable::addTrack(const QString &filePath)
{
    QString fileName = extractFileName(filePath);
    QFileInfo info(filePath);
    // --- Default icon ---
    QIcon icon(":/img/img/icons8-music-48.png");
    DatabaseManager &db = DatabaseManager::instance();
    AudioTagInfo tagInfo;
    //bool bUpdate = db.updateTrack(filePath, updateInfo);
    // bool bUpdate= db.loadOrUpdateTrack (filePath,updateInfo);
    bool b_DB_HasTag = db.loadTrack(filePath, tagInfo);
    if (b_DB_HasTag)
    {
        /*
               // --- Create standard items for each column ---
        QStandardItem* fileItem = new QStandardItem(icon, fileName);
        fileItem->setData(info.canonicalFilePath(), Qt::UserRole + 1); // store full path
        fileItem->setData(true, Qt::UserRole + 2);
        QStandardItem* extensionItem = new QStandardItem(tagInfo.sExtension);
        QStandardItem* pathItem = new QStandardItem(tagInfo.sPath);
        QStandardItem* durationItem = new QStandardItem(tagInfo.iDuration);
        QStandardItem* artistItem = new QStandardItem(tagInfo.sArtist);
        QStandardItem* titleItem = new QStandardItem(tagInfo.sTitle);
        QStandardItem* albumItem = new QStandardItem(tagInfo.sAlbum);
        QStandardItem* trackItem = new QStandardItem(tagInfo.iTrackNum);
        QStandardItem* yearItem = new QStandardItem(tagInfo.iYear);
        QStandardItem* genreItem = new QStandardItem(tagInfo.sGenre);
        QStandardItem* commentItem = new QStandardItem(tagInfo.sComment);
        QStandardItem* bitrateItem = new QStandardItem(tagInfo.iBitrate);
        QStandardItem* samplerateItem = new QStandardItem(tagInfo.iSamplerate);
        QStandardItem* bitsItem = new QStandardItem(tagInfo.iBits);
        QStandardItem* channelsItem = new QStandardItem(tagInfo.iChannels);
        QStandardItem* formatItem = new QStandardItem(tagInfo.sFormat);
        QStandardItem* coverSizeItem = new QStandardItem(tagInfo.sCoverSize);
        QStandardItem* fileSizeItem = new QStandardItem(tagInfo.iFileSize);
        // Build row
        QList<QStandardItem *> rowItems;
        rowItems << fileItem << extensionItem << pathItem << durationItem
           << artistItem << titleItem << albumItem << trackItem
           << yearItem << genreItem << commentItem << bitrateItem
           << samplerateItem << bitsItem << channelsItem << formatItem
           << coverSizeItem << fileSizeItem;

        */
        QList<QStandardItem *> rowItems = tagInfo.toStandardItems();
        m_model->appendRow(rowItems);
    }
    else
    {
        // --- Create standard items for each column ---
        QStandardItem *fileItem = new QStandardItem(icon, fileName);
        fileItem->setData(info.canonicalFilePath(), Qt::UserRole + 1); // store full path
        fileItem->setData(false, Qt::UserRole + 2);
        QStandardItem *extensionItem = new QStandardItem(info.suffix());
        QStandardItem *pathItem = new QStandardItem(QDir::toNativeSeparators(info.canonicalPath()));
        QStandardItem *durationItem = new QStandardItem(QString::number(0));
        QStandardItem *artistItem = new QStandardItem("");
        QStandardItem *titleItem = new QStandardItem("");
        QStandardItem *albumItem = new QStandardItem("");
        QStandardItem *trackItem = new QStandardItem("");
        QStandardItem *yearItem = new QStandardItem("");
        QStandardItem *genreItem = new QStandardItem("");
        QStandardItem *commentItem = new QStandardItem("");
        QStandardItem *bitrateItem = new QStandardItem("");
        QStandardItem *samplerateItem = new QStandardItem("");
        QStandardItem *bitsItem = new QStandardItem("");
        QStandardItem *channelsItem = new QStandardItem("");
        QStandardItem *formatItem = new QStandardItem("");
        QStandardItem *coverSizeItem = new QStandardItem("");
        QStandardItem *fileSizeItem = new QStandardItem("");
        QStandardItem *lastModifiedItem = new QStandardItem("");
        QStandardItem *ratingItem = new QStandardItem("");
        QStandardItem *playCountItem = new QStandardItem("");
        // Build row
        QList<QStandardItem *> rowItems;
        rowItems << fileItem << extensionItem << pathItem << durationItem
            << artistItem << titleItem << albumItem << trackItem
            << yearItem << genreItem << commentItem << bitrateItem
            << samplerateItem << bitsItem << channelsItem << formatItem
            << coverSizeItem << fileSizeItem << lastModifiedItem << ratingItem << playCountItem;
        m_model->appendRow(rowItems);
    }
    // --- Add to playlist ---
    //m_playlist->addMedia(QUrl::fromLocalFile(filePath));
    int iLastRow = m_model->rowCount() - 1;
    alignColumns(iLastRow);
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
    m_model->setColumnCount(ColumnIndex::ColumnCount);
    for (int i = 0; i < ColumnIndex::ColumnCount; ++i)
        m_model->setHeaderData(i, Qt::Horizontal, ColumnIndex::headerName(i));
    // m_model->setHorizontalHeaderLabels(QStringList()
    // << "Filename" << "Ext" << "Path" << "Duration" << "Artist"
    // << "Title" << "Album" << "Track" << "Year" << "Genre"
    // << "Comment" << "Bitrate" << "Samplerate" << "Bits" << "Channels"
    // << "Format" << "Cover size" << "File size"); m_playlist->clear();
    /*
      // Prima imposta il resize mode
    m_view->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    // Poi imposta i limiti
    m_view->verticalHeader()->setDefaultSectionSize(16);
    m_view->verticalHeader()->setMaximumSectionSize(32);

    // Opzionale: imposta anche minimum
    m_view->verticalHeader()->setMinimumSectionSize(16);
    */
    m_view->verticalHeader()->setDefaultSectionSize(16);
    m_view->verticalHeader()->setMaximumSectionSize(32);
    restoreColumnWidths();
    restoreColumnVisibility();
    //on_pushButton_clicked();
    //on_pushButton_2_clicked ();
    //setSectionsResizeMode();
    int iSortCol = settingsMgr->value("PlaylistViewSortColumn", 0).toInt();
    Qt::SortOrder order = static_cast<Qt::SortOrder>(settingsMgr->value("PlaylistViewSortColumnOrder", 0).toInt());
    // m_sortModel->sort(iSortCol, order);
    if (m_model->rowCount() > 0) onHeaderSortChanged(iSortCol, order);
    //if (m_view->horizontalHeader()->isSortIndicatorShown()) onHeaderSortChanged(iSortCol, order);
    // setSectionsResizeMode();
}

QString PlaylistTable::extractFileName(const QString &filePath)
{
    QFileInfo info(filePath);
    return info.completeBaseName();
}

void PlaylistTable::onDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
    {
        LOG_MSG_SHORT("No valid index:" << index.row());
        return;
    }
    int proxyRow = index.row();
    // Map proxy row to source row
    QModelIndex sourceIndex = m_sortModel->mapToSource(index);
    int sourceRow = sourceIndex.row();
    // Controlla se il brano selezionato è già in riproduzione
    if (m_playlist->currentIndex() == proxyRow)
    {
        //qDebug() << "new row";
        // Aggiorna playlist e playcount solo se è un nuovo brano
        m_playlist->setCurrentIndex(proxyRow);
        emit trackActivated(proxyRow);
        // Incrementa playcount nel modello
        QStandardItem *playCountItem = m_model->item(sourceRow, ColumnIndex::PlayCount);
        if (playCountItem)
        {
            int playCount = playCountItem->text().toInt();
            playCount++;
            playCountItem->setText(QString::number(playCount));
            playCountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            emit m_model->dataChanged(playCountItem->index(), playCountItem->index(), {Qt::DisplayRole});
        }
        // Incrementa playcount nel database
        QStandardItem *fileItem = m_model->item(sourceRow, ColumnIndex::Filename);
        if (fileItem)
        {
            QString path = fileItem->data(Qt::UserRole + 1).toString();
            if (!path.isEmpty())
            {
                DatabaseManager::instance().incrementPlayCount(path);
                DatabaseManager::instance().addToHistory(path, 0);
                fileItem->setIcon(QIcon(":/img/img/icons8-play-48.png"));
            }
        }
    }
    else
    {
        qDebug() << "same row";
        QModelIndex proxyindex = m_sortModel->mapToSource(index);
        QStandardItem *stdItem = m_model->item(proxyindex.row(), 0);
        m_playlist->setCurrentIndex(proxyRow);
        emit trackActivated(proxyRow);
        stdItem->setIcon(QIcon(":/img/img/icons8-play-48.png"));
    }
    /*
          // qDebug() << __PRETTY_FUNCTION__ << "index.row()" << index.row();
    int row = index.row();
    m_playlist->setCurrentIndex(row);
    //m_player->play();
    //onCurrentTrackChanged(row);
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

    */
}

//void PlaylistTable::onDoubleClicked(const QModelIndex &index)
//{
// if (!index.isValid())
// return;
// int proxyRow = index.row();
// QModelIndex sourceIndex = m_sortModel->mapToSource(index);
// int sourceRow = sourceIndex.row();
//    // Se è un nuovo brano, aggiorna indice e incrementa play count
// if (m_playlist->currentIndex() != proxyRow) {
// m_playlist->setCurrentIndex(proxyRow);
// emit trackActivated(proxyRow);
// incrementPlayCount(sourceRow);
// } else {
//        // Brano già in riproduzione
// m_playlist->setCurrentIndex(proxyRow);
// emit trackActivated(proxyRow);
// }
//}
void PlaylistTable::onClicked(const QModelIndex &index)
{
    int row = mapProxyRowToSource(m_sortModel, index.row());
    //LOG_MSG_SHORT("index.row()= proxy:" << index.row() << " - Model:" << row);
    //m_view->setModel (m_model);
    //m_view->verticalHeader ()->setModel (m_sortModel);
}

void PlaylistTable::setCurrentItemIcon(bool bPlaying)
{
    //if (!m_bSessionPlaylistIsShown) return;
    if (m_player->state() != QMediaPlayer::PlayingState || !bPlaying) // !bPlaying)
    {
        QIcon defaultIcon(":/img/img/icons8-music-48.png");
        // QIcon defaultIcon(":/img/img/icons8-stop-48.png");
        if (m_CurrentItem) m_CurrentItem->setIcon(defaultIcon);
    }
    else
    {
        QIcon playingIcon(":/img/img/icons8-play-48.png");
        if (m_CurrentItem) m_CurrentItem->setIcon(playingIcon);
    }
}

//void PlaylistTable::onCurrentTrackChanged(int index)
//{
// qDebug() << __PRETTY_FUNCTION__ << "index:" << index;
// if (index < 0 || !m_playlist)
// return;
//    // 1) Get current playing file path, normalized
// QMediaContent media = m_playlist->media(index);
// if (media.isNull())
// return;
// QString currentPath = media.canonicalUrl().toLocalFile();
// if (currentPath.isEmpty())
// return;
// QString currentCanon = QFileInfo(currentPath).canonicalFilePath();
// if (currentCanon.isEmpty())
// currentCanon = QDir::fromNativeSeparators(currentPath);
//    // 2) Reset icons on SOURCE model (keep icons tied to source rows)
// QIcon defaultIcon(":/img/img/icons8-music-48.png");
// QIcon playingIcon(":/img/img/icons8-play-48.png");
// for (int i = 0; i < m_model->rowCount(); ++i)
// {
// QStandardItem *item = m_model->item(i, 0); // filename column
// if (item)
// item->setIcon(defaultIcon);
// }
//    // 3) Find matching source row by reading UserRole+1 stored full path
// int matchSourceRow = -1;
// for (int r = 0; r < m_model->rowCount(); ++r)
// {
// QStandardItem *it = m_model->item(r, 0);
// if (!it) continue;
// QString stored = it->data(Qt::UserRole + 1).toString();
// if (stored.isEmpty()) continue;
// QString storedCanon = QFileInfo(stored).canonicalFilePath();
// if (storedCanon.isEmpty())
// storedCanon = QDir::fromNativeSeparators(stored);
// if (QString::compare(storedCanon, currentCanon, Qt::CaseInsensitive) == 0)
// {
// matchSourceRow = r;
// break;
// }
// }
// if (matchSourceRow < 0)
// {
// qDebug() << "onCurrentTrackChanged: no matching source row for" << currentPath;
//        // if you want, clear selection:
// if (m_view->selectionModel())
// m_view->selectionModel()->clearSelection();
// return;
// }
//    // 4) Set playing icon on the source item
// QStandardItem *currentItem = m_model->item(matchSourceRow, 0);
// if (currentItem)// && m_player->state()== QMediaPlayer::PlayingState)
// {
// currentItem->setIcon(playingIcon);
// m_CurrentItem = currentItem;
// }
//    // 5) Map source -> proxy and select/scroll the proxy index
// QModelIndex sourceIndex = m_model->index(matchSourceRow, 0);
// QModelIndex proxyIndex = m_sortModel->mapFromSource(sourceIndex);
// if (proxyIndex.isValid())
// {
//        // Scroll to and select the entire row in the view
// m_view->scrollTo(proxyIndex, QAbstractItemView::EnsureVisible);
// QItemSelectionModel *sel = m_view->selectionModel();
// if (sel)
// {
// sel->clearSelection();
// sel->select(proxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
// m_view->setCurrentIndex(proxyIndex);
// }
// else
// {
// m_view->selectRow(proxyIndex.row());
// }
// }
// else
// {
//        // The row might be filtered out by the proxy
// qDebug() << "proxyIndex invalid — row might be filtered out";
// if (m_view->selectionModel())
// m_view->selectionModel()->clearSelection();
// }
//}
void PlaylistTable::incrementPlayCount(int sourceRow)
{
    if (sourceRow < 0 || sourceRow >= m_model->rowCount())
        return;
    // Increment in model
    QStandardItem *playCountItem = m_model->item(sourceRow, ColumnIndex::PlayCount);
    if (playCountItem)
    {
        int playCount = playCountItem->text().toInt();
        playCount++;
        playCountItem->setText(QString::number(playCount));
        playCountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        emit m_model->dataChanged(playCountItem->index(),
            playCountItem->index(),
        {Qt::DisplayRole});
    }
    // Increment in database
    QStandardItem *fileItem = m_model->item(sourceRow, ColumnIndex::Filename);
    if (fileItem)
    {
        QString path = fileItem->data(Qt::UserRole + 1).toString();
        if (!path.isEmpty())
        {
            DatabaseManager::instance().incrementPlayCount(path);
        }
    }
}

void PlaylistTable::onCurrentTrackChanged(int playlistIndex)
{
    qDebug() << __PRETTY_FUNCTION__ << "playlistIndex:" << playlistIndex;
    if (playlistIndex < 0 || !m_playlist)
        return;
    QModelIndex sourceIndexInc = m_sortModel->mapToSource(m_sortModel->index(playlistIndex, 0));
    int sourceRow = sourceIndexInc.row();
    // Get current playing file path from playlist
    QMediaContent media = m_playlist->media(playlistIndex);
    if (media.isNull())
        return;
    QString currentPath = media.canonicalUrl().toLocalFile();
    if (currentPath.isEmpty())
        return;
    // Normalize path for comparison
    QString currentCanon = QFileInfo(currentPath).canonicalFilePath();
    if (currentCanon.isEmpty())
        currentCanon = QDir::fromNativeSeparators(currentPath);
    LOG_MSG_SHORT("Looking for track: " + currentCanon);
    // Define icons
    QIcon defaultIcon(":/img/img/icons8-music-48.png");
    QIcon playingIcon(":/img/img/icons8-play-48.png");
    // Reset ALL icons in SOURCE model first
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        QStandardItem *item = m_model->item(i, 0);
        if (item)
            item->setIcon(defaultIcon);
    }
    // Find matching SOURCE row by comparing stored file paths
    int matchSourceRow = -1;
    for (int r = 0; r < m_model->rowCount(); ++r)
    {
        QStandardItem *item = m_model->item(r, 0);
        if (!item)
            continue;
        QString stored = item->data(Qt::UserRole + 1).toString();
        if (stored.isEmpty())
            continue;
        // Normalize stored path
        QString storedCanon = QFileInfo(stored).canonicalFilePath();
        if (storedCanon.isEmpty())
            storedCanon = QDir::fromNativeSeparators(stored);
        // Compare paths
        if (QString::compare(storedCanon, currentCanon, Qt::CaseInsensitive) == 0)
        {
            matchSourceRow = r;
            LOG_MSG_SHORT("Found match at source row: " + QString::number(matchSourceRow));
            break;
        }
    }
    // If no match found, clear selection and exit
    if (matchSourceRow < 0)
    {
        qDebug() << "WARNING: No matching source row found for" << currentPath;
        if (m_view->selectionModel())
            m_view->selectionModel()->clearSelection();
        m_CurrentItem = nullptr;
        m_CurrentPlaylistItem = nullptr;
        return;
    }
    // Set playing icon on the matched SOURCE item
    QStandardItem *currentItem = m_model->item(matchSourceRow, 0);
    if (currentItem)
    {
        currentItem->setIcon(playingIcon);
        m_CurrentItem = currentItem;
        if (m_bSessionPlaylistIsShown)
        {
            m_CurrentPlaylistItem = currentItem;
            m_iLastPlaylistFile = m_CurrentPlaylistItem->row();
        }
    }
    incrementPlayCount(sourceRow); // Map SOURCE row to PROXY row for view operations
    DatabaseManager::instance().addToHistory(currentCanon, 0);
    QModelIndex sourceIndex = m_model->index(matchSourceRow, 0);
    QModelIndex proxyIndex = m_sortModel->mapFromSource(sourceIndex);
    if (proxyIndex.isValid())
    {
        LOG_MSG_SHORT("Mapped to proxy row: " + QString::number(proxyIndex.row()));
        // Select and scroll to the row in the view
        m_view->scrollTo(proxyIndex, QAbstractItemView::EnsureVisible);
        QItemSelectionModel *selModel = m_view->selectionModel();
        if (selModel)
        {
            selModel->clearSelection();
            // Select the entire row
            QModelIndex leftIndex = m_sortModel->index(proxyIndex.row(), 0);
            QModelIndex rightIndex = m_sortModel->index(proxyIndex.row(),
                m_sortModel->columnCount() - 1);
            QItemSelection selection(leftIndex, rightIndex);
            selModel->select(selection, QItemSelectionModel::Select);
            m_view->setCurrentIndex(proxyIndex);
        }
    }
    else
    {
        // Row is filtered out by proxy (not visible)
        qDebug() << "Row is filtered out - source row" << matchSourceRow
            << "has no valid proxy index";
        if (m_view->selectionModel())
            m_view->selectionModel()->clearSelection();
    }
}

void PlaylistTable::on_pushButton_2_clicked()
{
    QHeaderView *header = m_view->horizontalHeader();
    header->resizeSections(QHeaderView::ResizeToContents);
}

int PlaylistTable::mapSourceRowToProxy(QAbstractItemModel *sourceModel, QSortFilterProxyModel *proxyModel, int sourceRow)
{
    QModelIndex sourceIndex = sourceModel->index(sourceRow, 0);
    return proxyModel->mapFromSource(sourceIndex).row();
}

int PlaylistTable::mapProxyRowToSource(QSortFilterProxyModel *proxyModel, int proxyRow)
{
    if (!proxyModel)
        return -1;
    QModelIndex proxyIndex = proxyModel->index(proxyRow, 0); // column 0 is arbitrary, just needs a valid index
    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    return sourceIndex.isValid() ? sourceIndex.row() : -1;
}

int PlaylistTable::mapRowToProxy( int sourceRow)
{
    QModelIndex sourceIndex = m_model->index(sourceRow, 0);
    return m_sortModel->mapFromSource(sourceIndex).row();
}

int PlaylistTable::mapRowToSource( int proxyRow)
{
    if (!m_sortModel)
        return -1;
    QModelIndex proxyIndex = m_sortModel->index(proxyRow, 0); // column 0 is arbitrary, just needs a valid index
    QModelIndex sourceIndex = m_sortModel->mapToSource(proxyIndex);
    return sourceIndex.isValid() ? sourceIndex.row() : -1;
}

void PlaylistTable::updateSearchCount(int currentRow)
{
    QPoint globalPos = ui->pushButtonClearFind->mapToGlobal(QPoint(ui->pushButtonClearFind->width() / 2, ui->pushButtonClearFind->height() / 2));
    // Costruisci una lista delle righe uniche con match
    QSet<int> uniqueRows;
    for (const QModelIndex &idx : m_findMatches)
        uniqueRows.insert(idx.row());
    QList<int> rowsList = uniqueRows.toList();
    std::sort(rowsList.begin(), rowsList.end()); // Ordina le righe
    if (rowsList.isEmpty())
    {
        // ui->labelSearchStatus->setText("0 of 0");
        QToolTip::showText(globalPos, "No results", ui->pushButtonClearFind);
        return;
    }
    // Trova l’indice della riga corrente
    int currentIndex = rowsList.indexOf(currentRow);
    if (currentIndex == -1)
        currentIndex = 0; // fallback
    // ui->labelSearchStatus->setText(QString("%1 of %2")
    // .arg(currentIndex + 1)
    // .arg(rowsList.size()));
    QString text = QString("%1 of %2").arg(currentIndex + 1).arg(rowsList.size());
    QToolTip::showText(globalPos, text, ui->pushButtonClearFind);
}

void PlaylistTable::handleNewSearchInput()
{
    QString newText = ui->comboBoxFind->currentText().trimmed();
    if (newText.isEmpty() || newText.length() < MIN_SEARCH_CHARS)
    {
        return;
    }
    int existingIndex = -1;
    for (int i = 0; i < ui->comboBoxFind->count(); ++i)
    {
        if (ui->comboBoxFind->itemText(i).compare(newText, Qt::CaseInsensitive) == 0)
        {
            existingIndex = i;
            break;
        }
    }
    if (existingIndex != -1)
    {
        ui->comboBoxFind->removeItem(existingIndex);
    }
    ui->comboBoxFind->insertItem(0, newText);
    ui->comboBoxFind->setEditText(newText);
    while (ui->comboBoxFind->count() > MAX_SEARCH_HISTORY_SIZE)
    {
        ui->comboBoxFind->removeItem(ui->comboBoxFind->count() - 1); // Rimuove l'ultimo elemento
    }
    // ui->comboBoxFind->setCurrentText(newText);
    QStringList updatedHistory;
    for (int i = 0; i < ui->comboBoxFind->count(); ++i)
    {
        updatedHistory.append(ui->comboBoxFind->itemText(i));
    }
    saveSearchHistory(updatedHistory);
}

void PlaylistTable::handleNewFilterInput()
{
    QString newText = ui->comboBoxFilter->currentText().trimmed();
    if (newText.isEmpty() || newText.length() < MIN_SEARCH_CHARS)
    {
        return;
    }
    int existingIndex = -1;
    for (int i = 0; i < ui->comboBoxFilter->count(); ++i)
    {
        if (ui->comboBoxFilter->itemText(i).compare(newText, Qt::CaseInsensitive) == 0)
        {
            existingIndex = i;
            break;
        }
    }
    if (existingIndex != -1)
    {
        ui->comboBoxFilter->removeItem(existingIndex);
    }
    ui->comboBoxFilter->insertItem(0, newText);
    ui->comboBoxFilter->setEditText(newText);
    while (ui->comboBoxFilter->count() > MAX_SEARCH_HISTORY_SIZE)
    {
        ui->comboBoxFilter->removeItem(ui->comboBoxFilter->count() - 1); // Rimuove l'ultimo elemento
    }
    // ui->comboBoxFilter->setCurrentText(newText);
    QStringList updatedHistory;
    for (int i = 0; i < ui->comboBoxFilter->count(); ++i)
    {
        updatedHistory.append(ui->comboBoxFilter->itemText(i));
    }
    saveFilterHistory(updatedHistory);
}

/*
  void PlaylistTable::handleNewSearchInput()
{
    QString newText = ui->comboBoxFind->currentText().trimmed();
    if (newText.length() < 4) return;
    if (newText.isEmpty())
    {
        return;
    }
    QStringList currentHistory;
    for (int i = 0; i < ui->comboBoxFind->count(); ++i)
    {
        currentHistory.append(ui->comboBoxFind->itemText(i));
    }
    // currentHistory.removeAll(newText);
    int existingIndex = -1;
    QString itemText;
    for (int i = 0; i < ui->comboBoxFind->count(); ++i)
    {
    itemText=ui->comboBoxFind->itemText(i);
        if (itemText.compare(newText, Qt::CaseInsensitive) == 0)
        {
            existingIndex = i;
            break; // Trovato, esci dal loop
        }
    }
    if (existingIndex != -1)
    {
        // Rimuovi la vecchia versione (duplicata) trovata
        ui->comboBoxFind->removeItem(existingIndex);
        currentHistory.removeAt (existingIndex);
    }
    currentHistory.prepend(newText);
    while (currentHistory.size() > MAX_SEARCH_HISTORY_SIZE)
    {
        currentHistory.removeLast();
    }
    ui->comboBoxFind->clear();
    ui->comboBoxFind->addItems(currentHistory);
    ui->comboBoxFind->setCurrentText(newText);
    saveSearchHistory(currentHistory);
}

*/
void PlaylistTable::loadSearchHistory()
{
    settingsMgr->beginGroup("Table");
    QStringList history = settingsMgr->value("SearchHistory").toStringList();
    settingsMgr->endGroup();
    ui->comboBoxFind->clear();
    if (!history.isEmpty())
    {
        ui->comboBoxFind->addItems(history);
        //ui->comboBoxFind->setCurrentText(history.first());
        //ui->comboBoxFind->setEditText(history.first());
    }
    ui->comboBoxFind->setEditText(QString());
}

void PlaylistTable::loadFilterHistory()
{
    settingsMgr->beginGroup("Table");
    QStringList history = settingsMgr->value("FilterHistory").toStringList();
    settingsMgr->endGroup();
    ui->comboBoxFilter->clear();
    if (!history.isEmpty())
    {
        ui->comboBoxFilter->addItems(history);
        //ui->comboBoxFilter->setCurrentText(history.first());
        //ui->comboBoxFilter->setEditText(history.first());
    }
    ui->comboBoxFilter->setEditText(QString());
}

void PlaylistTable::saveSearchHistory(const QStringList &history)
{
    settingsMgr->beginGroup("Table");
    settingsMgr->setValue("SearchHistory", history);
    settingsMgr->endGroup();
}

void PlaylistTable::saveFilterHistory(const QStringList &history)
{
    settingsMgr->beginGroup("Table");
    settingsMgr->setValue("FilterHistory", history);
    settingsMgr->endGroup();
}

//void PlaylistTable::updateSearchCount(int currentMatchIndex)
//{
// int totalMatches = m_findMatches.size();
// QPoint globalPos = ui->label_2-> mapToGlobal(QPoint(ui->label_2->width() / 2, ui->label_2->height() / 2));
// if (totalMatches == 0)
// {
//        //ui->labelSearchStatus->setText("0 of 0");
// QToolTip::showText(globalPos, "No results", ui->label_2);
// return;
// }
//    // currentMatchIndex deve essere tra 0 e totalMatches-1
// currentMatchIndex = qBound(0, currentMatchIndex, totalMatches - 1);
// QString text = QString("%1 of %2").arg(currentMatchIndex + 1).arg(totalMatches);
// QToolTip::showText(globalPos, text, ui->label_2);
////ui->labelSearchStatus->setText(text);
//}
void PlaylistTable::findNext()
{
    if (m_findMatches.isEmpty() && !m_lastSearchText.isEmpty())
        findInTable(m_lastSearchText);
    if (m_findMatches.isEmpty())
        return;
    int currentRow = m_view->currentIndex().isValid()
        ? m_view->currentIndex().row()
        : -1;
    // Trova la prossima riga contenente un match
    int nextRow = -1;
    int nextIndexInMatches = -1;
    for (int i = 0; i < m_findMatches.size(); ++i)
    {
        if (m_findMatches[i].row() > currentRow)
        {
            nextRow = m_findMatches[i].row();
            nextIndexInMatches = i;
            break;
        }
    }
    if (nextRow == -1) // ricomincia dall'inizio
        nextRow = m_findMatches.first().row();
    QModelIndex nextIndex = m_sortModel->index(nextRow, 0);
    m_view->setCurrentIndex(nextIndex);
    m_view->scrollTo(nextIndex, QAbstractItemView::EnsureVisible);
    updateSearchCount(nextRow);
}

void PlaylistTable::findPrevious()
{
    if (m_findMatches.isEmpty() && !m_lastSearchText.isEmpty())
        findInTable(m_lastSearchText);
    if (m_findMatches.isEmpty())
        return;
    int currentRow = m_view->currentIndex().isValid()
        ? m_view->currentIndex().row()
        : m_sortModel->rowCount();
    // Trova la riga precedente contenente un match
    int prevRow = -1;
    int nextIndexInMatches = -1;
    for (int i = m_findMatches.size() - 1; i >= 0; --i)
    {
        if (m_findMatches[i].row() < currentRow)
        {
            prevRow = m_findMatches[i].row();
            nextIndexInMatches = i - 1;
            break;
        }
    }
    if (prevRow == -1) // torna all’ultima
        prevRow = m_findMatches.last().row();
    QModelIndex nextIndex = m_sortModel->index(prevRow, 0);
    m_view->setCurrentIndex(nextIndex);
    m_view->scrollTo(nextIndex, QAbstractItemView::EnsureVisible);
    updateSearchCount(prevRow);
}

void PlaylistTable::clearSearchHighlight()
{
    for (int r = 0; r < m_sortModel->rowCount(); ++r)
        for (int c = 0; c < m_sortModel->columnCount(); ++c)
            m_sortModel->setData(m_sortModel->index(r, c), QVariant(), Qt::UserRole + 10);
    m_findMatches.clear();
    m_view->viewport()->update();
    // ui->pushButtonNext->setEnabled (true);
    // ui->pushButtonPrev->setEnabled (true);
}

void PlaylistTable::findInTable(const QString &searchText)
{
    if (searchText.length() < MIN_SEARCH_CHARS) return;
    if (!m_sortModel || searchText.isEmpty())
        return;
    m_lastSearchText = searchText;
    m_findMatches.clear();
    // Clear previous highlights
    for (int r = 0; r < m_sortModel->rowCount(); ++r)
        for (int c = 0; c < m_sortModel->columnCount(); ++c)
            m_sortModel->setData(m_sortModel->index(r, c), QVariant(), Qt::UserRole + 10);
    //    // Rimuovi evidenziazione precedente
    // for (int r = 0; r < m_sortModel->rowCount(); ++r)
    // for (int c = 0; c < m_sortModel->columnCount(); ++c)
    // m_sortModel->setData(m_sortModel->index(r, c), QVariant(), Qt::BackgroundRole);
    // Cerca e colora
    for (int r = 0; r < m_sortModel->rowCount(); ++r)
    {
        for (int c = 0; c < m_sortModel->columnCount(); ++c)
        {
            if (m_view->isColumnHidden(c)) // skip hidden columns
                continue;
            QModelIndex idx = m_sortModel->index(r, c);
            QString text = m_sortModel->data(idx).toString();
            if (text.contains(searchText, Qt::CaseInsensitive))
            {
                // m_sortModel->setData(idx, QColor(Qt::yellow), Qt::BackgroundRole);
                // m_sortModel->setData(idx, QColor(Qt::red), Qt::ForegroundRole);
                m_sortModel->setData(idx, true, Qt::UserRole + 10);
                //qDebug() << text;
                m_findMatches.append(idx);
            }
        }
    }
    if (m_findMatches.isEmpty())
    {
        updateSearchCount(0);
        return;
    }
    QModelIndex first = m_findMatches.first();
    m_view->setCurrentIndex(first);
    m_view->scrollTo(first, QAbstractItemView::EnsureVisible);
    ui->pushButtonNext->setEnabled(true);
    ui->pushButtonPrev->setEnabled(true);
    updateSearchCount(1);
    // Se nessuna selezione, vai al primo risultato
    // QModelIndex current = m_view->currentIndex();
    // if (!current.isValid())
    // {
    // QModelIndex first = m_findMatches.first();
    // m_view->scrollTo(first, QAbstractItemView::EnsureVisible);
    // m_view->setCurrentIndex(first);
    // m_view->selectionModel()->select(first, QItemSelectionModel::ClearAndSelect);
    // }
    // else
    //{
    // QModelIndex first = m_findMatches.first();
    // m_view->scrollTo(first, QAbstractItemView::EnsureVisible);
    // m_view->setCurrentIndex(first);
    // m_view->selectionModel()->select(first, QItemSelectionModel::ClearAndSelect);
    //}
}

//void PlaylistTable::clearSearchHighlight()
//{
// for (int r = 0; r < m_sortModel->rowCount(); ++r)
// for (int c = 0; c < m_sortModel->columnCount(); ++c)
// m_sortModel->setData(m_sortModel->index(r, c), QVariant(), Qt::BackgroundRole);
// m_findMatches.clear();
// m_lastSearchText.clear();
//}
//void PlaylistTable::findInTable(const QString &searchText)
//{
// if (searchText.isEmpty())
// return;
// QAbstractItemModel *model = m_sortModel; //m_view->model();
// QList<QModelIndex> matches;
// for (int row = 0; row < model->rowCount(); ++row)
// {
// for (int col = 0; col < model->columnCount(); ++col)
// {
// QModelIndex index = model->index(row, col);
// QString cellText = model->data(index).toString();
// if (cellText.contains(searchText, Qt::CaseInsensitive))
// matches.append(index);
// }
// }
// if (!matches.isEmpty())
// {
// m_view->scrollTo(matches.first(), QAbstractItemView::PositionAtCenter);
// m_view->setCurrentIndex(matches.first());
// m_view->selectionModel()->select(matches.first(), QItemSelectionModel::Select);
// }
//}
//void PlaylistTable::setRating(const QModelIndex &index, int newRating)
//{
//    // Dobbiamo aggiornare il modello.
//    // Il rating si trova nella COLONNA RATING (supponiamo sia la colonna 2)
//    // Crea l'indice per la colonna del rating sulla riga selezionata
// QModelIndex ratingIndex = index.sibling(index.row(), ColumnIndex::Rating);
//    // Nel tuo delegato stai leggendo il DisplayRole, quindi impostiamo quello.
//    // L'uso di QString::number(newRating) è cruciale per la logica attuale.
// QString ratingValue = QString::number(newRating);
//    // Se la nuova valutazione è 0, puoi scegliere di usare -1 (Rating non assegnato)
//    // per coerenza con la tua precedente logica, se necessario.
// if (newRating == 0)
// {
//        // Se vuoi usare -1 per "Clear Rating", altrimenti usa "0".
//        // ratingValue = "-1";
// }
//    // Supponendo che tu stia usando QStandardItemModel o simile:
// m_view->model()->setData(ratingIndex, ratingValue, Qt::DisplayRole);
//    // Nota: Se la tua classe ha un modello personalizzato, assicurati che
//    // setData() sia implementato correttamente.
// QStandardItem *fileItem = m_model->item(index.row(), ColumnIndex::Filename);
// if (fileItem)
// {
// QString path = fileItem->data(Qt::UserRole + 1).toString();
// if (!path.isEmpty())
// DatabaseManager::instance().setRating(path, newRating);
// LOG_MSG_SHORT("path" << path << "row" << index.row());
// }
//}
void PlaylistTable::setRating(const QModelIndex &index, int newRating)
{
    if (!index.isValid() || newRating < -1)
        return;
    // Map from proxy to source if sorting/filtering is active
    QModelIndex sourceIndex = m_sortModel->mapToSource(index);
    int sourceRow = sourceIndex.row();
    if (sourceRow < 0 || sourceRow >= m_model->rowCount())
        return;
    // Update the model's rating column
    QModelIndex ratingIndex = m_model->index(sourceRow, ColumnIndex::Rating);
    QString ratingValue = QString::number(newRating);
    m_model->setData(ratingIndex, ratingValue, Qt::DisplayRole);
    // Emit dataChanged manually if needed
    emit m_model->dataChanged(ratingIndex, ratingIndex, {Qt::DisplayRole});
    // Update database
    QStandardItem *fileItem = m_model->item(sourceRow, ColumnIndex::Filename);
    if (fileItem)
    {
        QString path = fileItem->data(Qt::UserRole + 1).toString();
        if (!path.isEmpty())
        {
            if (!DatabaseManager::instance().setRating(path, newRating))
                qWarning() << "Failed to update rating for" << path;
        }
    }
    LOG_MSG_SHORT("Updated rating:" << newRating << "row:" << sourceRow);
}

void PlaylistTable::showPlaylistContextMenu(const QPoint &pos)
{
    QModelIndex index = m_view->indexAt(pos);
    if (!index.isValid()) return;
    QModelIndex sourceIndex = m_sortModel->mapToSource(index);
    int sourceRow = sourceIndex.row();
    QStandardItem *item = m_model->item(sourceRow, 0);
    QString fullPath = item->data(Qt::UserRole + 1).toString();
    int trackId = DatabaseManager::instance().getTrackId(fullPath);
    bool inFavorites = DatabaseManager::instance().isTrackInFavorites(trackId);
    QMenu contextMenu(this);
    QAction *scrollToCurrentAction = contextMenu.addAction(tr("Scroll to current"));
    scrollToCurrentAction->setIcon(QIcon(":/img/img/icons8-search-in-list-48.png"));
    QAction *favoriteAction;
    if (inFavorites)
    {
        favoriteAction = contextMenu.addAction(tr("Remove from favorites"));
        favoriteAction->setIcon(QIcon(":/img/img/icons8-love-not-48.png"));
    }
    else
    {
        favoriteAction = contextMenu.addAction(tr("Add to favorites"));
        favoriteAction->setIcon(QIcon(":/img/img/icons8-love-48.png"));
    }
    QMenu *ratingMenu = contextMenu.addMenu(tr("Set Rating"));
    // Aggiungi un'icona alla voce del sottomenu (opzionale, ma consigliato)
    ratingMenu->setIcon(QIcon(":/img/img/icons8-star-48.png"));
    // Creazione delle azioni di Rating (da 0 a 5)
    QAction *ratingRemoveAction = new QAction("Remove rating", this);
    ratingMenu->addAction(ratingRemoveAction);
    for (int rating = 0; rating <= 5; ++rating)
    {
        // 2a. Crea il testo dell'azione
        QString actionText = QString::number(rating) + " stars";
        // 2b. Crea l'azione e impostala (il nome dell'oggetto QAction è RatingAction_N)
        QAction *ratingAction = new QAction(actionText, this);
        ratingAction->setObjectName(QString("RatingAction_%1").arg(rating));
        // 2c. Associa i dati: memorizza il valore del rating nell'azione stessa
        // Questo ci servirà per sapere quale valore applicare in setRatingFromMenu
        ratingAction->setData(rating);
        // 2d. Aggiungi l'azione al sottomenu
        ratingMenu->addAction(ratingAction);
    }
    // QAction *forgetPlayedAction = contextMenu.addAction(tr("Forget played"));
    // forgetPlayedAction->setIcon(QIcon(":/img/img/icons8-reload-48.png"));
    // QString sText = "Forget played (" + QString::number(m_playedList.count()) + " of " + QString::number(m_model->rowCount()) + ")";
    // forgetPlayedAction->setText(sText);
    // QAction *filterAction = contextMenu.addAction(tr("Filter"));
    // filterAction->setIcon(QIcon(":/img/img/icons8-filter-48.png"));
    // contextMenu.addSeparator();
    // QAction *removeSelectedAction = contextMenu.addAction(tr("Remove selected"));
    // QAction *clearExceptSelectedAction = contextMenu.addAction(tr("Clear all except selected"));
    // QAction *clearAction = contextMenu.addAction(tr("Clear playlist"));
    // QAction *playlistTableAction = nullptr;
    // if (m_bTablePlaylist)
    // {
    // playlistTableAction = contextMenu.addAction(tr("Show playlist table"));
    // playlistTableAction->setIcon(QIcon(":/img/img/icons8-playlist_trim-48.png"));
    // }
    // contextMenu.addSeparator();
    // QAction *searchAction = contextMenu.addAction(tr("Search on Google"));
    // searchAction->setIcon(QIcon(":/img/img/icons8-google-48.png"));
    // QAction *selectInExplorerAction = contextMenu.addAction(tr("Select in file manager"));
    // selectInExplorerAction->setIcon(QIcon(":/img/img/Folder_audio.png"));
    // QAction *copyNameAction = contextMenu.addAction(tr("Copy file name"));
    // copyNameAction->setIcon(QIcon(":/img/img/icons8-copy-to-clipboard_file-48.png"));
    // QAction *copyFullPathAction = contextMenu.addAction(tr("Copy full pathname"));
    // copyFullPathAction->setIcon(QIcon(":/img/img/icons8-copy-to-clipboard_path-48.png"));
    // contextMenu.addSeparator();
    // QAction *loadAction = contextMenu.addAction(tr("Load playlist"));
    // QAction *saveAction = contextMenu.addAction(tr("Save playlist"));
    // saveAction->setIcon(QIcon(":/img/img/icons8-folder-save-48.png"));
    // loadAction->setIcon(QIcon(":/img/img/icons8-folder-load-48.png"));
    // clearAction->setIcon(QIcon(":/img/img/icons8-clear-48.png"));
    // clearExceptSelectedAction->setIcon(QIcon(":/img/img/icons8-list_keep1-48.png"));
    // removeSelectedAction->setIcon(QIcon(":/img/img/icons8-list_delete1-48.png"));
    QAction *selectedAction = contextMenu.exec(m_view->viewport()->mapToGlobal(pos));
    if (!selectedAction) return;
    if (selectedAction == scrollToCurrentAction)
    {
        if (m_CurrentItem)
        {
            //QModelIndex sourceIndex = m_model->index(m_CurrentItem->row(), 0);
            QModelIndex proxyIndex = m_sortModel->index(mapSourceRowToProxy(m_model, m_sortModel, m_CurrentItem->row()), 0);
            m_view->scrollTo(proxyIndex, QAbstractItemView::PositionAtCenter); //EnsureVisible);
        }
    }
    else if (selectedAction == ratingRemoveAction)
    {
        setRating(index, -1);
    }
    else if (selectedAction->objectName().startsWith("RatingAction_"))
    {
        int newRating = selectedAction->data().toInt();
        setRating(index, newRating);
    }
    else if (selectedAction == favoriteAction)
    {
        //QModelIndex sourceIndex = m_sortModel->mapToSource(index);
        //int sourceRow = sourceIndex.row();
        //QStandardItem *item = m_model->item(sourceRow, 0);
        //QString fullPath = item->data(Qt::UserRole + 1).toString();
        if (inFavorites)
        {
            DatabaseManager::instance().removeFromFavorites(fullPath);
        }
        else
        {
            DatabaseManager::instance().addToFavorites(fullPath);
        }
    }
    // if (selectedAction == saveAction) handleSavePlaylist();
    // else if (selectedAction == forgetPlayedAction) forgetPlayed();
    // else if (selectedAction == playlistTableAction) showPlaylistTable();
    // else if (selectedAction == filterAction) {
    // ui->labelFilter->setVisible(true);
    // ui->lineEditFilter->setVisible(true);
    // ui->pushButtonResetFilter->setVisible(true);
    // ui->lineEditFilter->setText("");
    // ui->lineEditFilter->setFocus();
    // }
    // else if (selectedAction == copyNameAction) copyCurrentName();
    // else if (selectedAction == copyFullPathAction) copyCurrentFullPath();
    // else if (selectedAction == loadAction) handleLoadPlaylist();
    // else if (selectedAction == removeSelectedAction) handleRemoveSelected();
    // else if (selectedAction == clearExceptSelectedAction) clearExceptSelected();
    // else if (selectedAction == clearAction) clearPlaylist(false);
    // else if (selectedAction == scrollToCurrentAction) scrollToCurrentTrack();
    // else if (selectedAction == searchAction) {
    // QString filename = m_view->model()->data(m_view->model()->index(index.row(), 0)).toString();
    // QFileInfo info(filename);
    // openGoogleSearch(info.completeBaseName());
    // }
    // else if (selectedAction == selectInExplorerAction) {
    // QUrl mediaUrl;
    // int selectedRow = index.row();
    // QModelIndex sourceIndex = m_sortModel->mapToSource(m_sortModel->index(selectedRow, 0));
    // if (selectedRow < 0 || selectedRow >= m_playlist->mediaCount()) return;
    // mediaUrl = m_playlist->media(selectedRow).canonicalUrl();
    // QString localFile = mediaUrl.toLocalFile();
    // openFolderAndSelectFileEx(localFile);
    // }
}

/*
  void PlaylistTable::setKeyboardTargetWidget(Widget *target)
{
    m_target = target;
}

void PlaylistTable::keyPressEvent(QKeyEvent *event)
{
    if (!m_target)
    {
        QWidget::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Space)
    {
        QAction *action = m_target->getActionPlay();
        if (action)
            action->trigger();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

*/
void PlaylistTable::on_pushButtonFav_clicked()
{
    // m_iLastPlaylistFile = m_CurrentPlaylistItem->row();
    // m_lastPlaylistFile = m_CurrentItem->data(Qt::UserRole + 1).toString();
    if (m_bSessionPlaylistIsShown) saveSessionPlaylist();
    m_bSessionPlaylistIsShown = false;
    m_model->clear();
    m_model->setColumnCount(ColumnIndex::ColumnCount);
    for (int i = 0; i < ColumnIndex::ColumnCount; ++i)
        m_model->setHeaderData(i, Qt::Horizontal, ColumnIndex::headerName(i));
    // m_model->setHorizontalHeaderLabels(QStringList()
    // << "Filename" << "Ext" << "Path" << "Duration" << "Artist"
    // << "Title" << "Album" << "Track" << "Year" << "Genre"
    // << "Comment" << "Bitrate" << "Samplerate" << "Bits" << "Channels"
    // << "Format" << "Cover size" << "File size"); m_playlist->clear();
    /*
      // Prima imposta il resize mode
    m_view->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    // Poi imposta i limiti
    m_view->verticalHeader()->setDefaultSectionSize(16);
    m_view->verticalHeader()->setMaximumSectionSize(32);

    // Opzionale: imposta anche minimum
    m_view->verticalHeader()->setMinimumSectionSize(16);
    */
    m_view->verticalHeader()->setDefaultSectionSize(16);
    m_view->verticalHeader()->setMaximumSectionSize(32);
    restoreColumnWidths();
    restoreColumnVisibility();
    m_HorizontalHeader->setSortIndicator(-1, Qt::AscendingOrder);
    m_HorizontalHeader->setSortIndicatorShown(false);
    //on_pushButton_clicked();
    //on_pushButton_2_clicked ();
    //setSectionsResizeMode();
    int iSortCol = settingsMgr->value("PlaylistViewSortColumn", 0).toInt();
    Qt::SortOrder order = static_cast<Qt::SortOrder>(settingsMgr->value("PlaylistViewSortColumnOrder", 0).toInt());
    // m_sortModel->sort(iSortCol, order);
    //if (m_model->rowCount() > 0) onHeaderSortChanged(iSortCol, order);
    //if (m_view->horizontalHeader()->isSortIndicatorShown()) onHeaderSortChanged(iSortCol, order);
    // setSectionsResizeMode();
    // m_playlist->clear();
    QList<AudioTagInfo> favList = DatabaseManager::instance().favoriteTracks();
    for (AudioTagInfo tagInfo : favList)
    {
        QList<QStandardItem *> rowItems = tagInfo.toStandardItems();
        m_model->appendRow(rowItems);
        //m_playlist->addMedia(QUrl::fromLocalFile(tagInfo.sPath + "/" + tagInfo.sBaseFileName + "." + tagInfo.sExtension));
    }
    if (!favList.isEmpty())
    {
        QModelIndex topIndex = m_model->index(0, 0);
        m_view->scrollTo(topIndex, QAbstractItemView::PositionAtTop);
        m_view->selectRow(0);
    }
    m_playlist->clear();
    m_view->setModel(m_sortModel);
    int proxyRowCount = m_sortModel->rowCount();
    for (int proxyRow = 0; proxyRow < proxyRowCount; ++proxyRow)
    {
        // Get proxy index for column 0 (filename)
        QModelIndex proxyIndex = m_sortModel->index(proxyRow, 0);
        // Map proxy index to source index
        QModelIndex sourceIndex = m_sortModel->mapToSource(proxyIndex);
        if (!proxyIndex.isValid())
            continue;
        int sourceRow = sourceIndex.row();
        // Read data from SOURCE model
        QStandardItem *item = m_model->item(sourceRow, 0);
        if (!item)
            continue;
        QString fullPath = item->data(Qt::UserRole + 1).toString();
        //qDebug()<<"fullPath="<< fullPath;
        m_playlist->addMedia(QUrl::fromLocalFile(fullPath));
        alignColumns(proxyRow);
    }
    emit playlistUpdated(m_playlist);
    m_view->setSortingEnabled(true);
    m_HorizontalHeader->setSortIndicatorShown(true);
}

void PlaylistTable::on_pushButtonHistory_clicked()
{
    //m_iLastPlaylistFile = m_CurrentPlaylistItem->row();
    // m_lastPlaylistFile = m_CurrentItem->data(Qt::UserRole + 1).toString();
    if (m_bSessionPlaylistIsShown) saveSessionPlaylist();
    m_bSessionPlaylistIsShown = false;
    m_model->clear();
    m_model->setColumnCount(ColumnIndex::ColumnCount);
    for (int i = 0; i < ColumnIndex::ColumnCount; ++i)
        m_model->setHeaderData(i, Qt::Horizontal, ColumnIndex::headerName(i));
    // m_model->setHorizontalHeaderLabels(QStringList()
    // << "Filename" << "Ext" << "Path" << "Duration" << "Artist"
    // << "Title" << "Album" << "Track" << "Year" << "Genre"
    // << "Comment" << "Bitrate" << "Samplerate" << "Bits" << "Channels"
    // << "Format" << "Cover size" << "File size"); m_playlist->clear();
    /*
      // Prima imposta il resize mode
    m_view->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    // Poi imposta i limiti
    m_view->verticalHeader()->setDefaultSectionSize(16);
    m_view->verticalHeader()->setMaximumSectionSize(32);

    // Opzionale: imposta anche minimum
    m_view->verticalHeader()->setMinimumSectionSize(16);
    */
    m_view->verticalHeader()->setDefaultSectionSize(16);
    m_view->verticalHeader()->setMaximumSectionSize(32);
    restoreColumnWidths();
    restoreColumnVisibility();
    m_HorizontalHeader->setSortIndicator(-1, Qt::AscendingOrder);
    m_HorizontalHeader->setSortIndicatorShown(false);
    //on_pushButton_clicked();
    //on_pushButton_2_clicked ();
    //setSectionsResizeMode();
    int iSortCol = settingsMgr->value("PlaylistViewSortColumn", 0).toInt();
    Qt::SortOrder order = static_cast<Qt::SortOrder>(settingsMgr->value("PlaylistViewSortColumnOrder", 0).toInt());
    // m_sortModel->sort(iSortCol, order);
    //if (m_model->rowCount() > 0) onHeaderSortChanged(iSortCol, order);
    //if (m_view->horizontalHeader()->isSortIndicatorShown()) onHeaderSortChanged(iSortCol, order);
    // setSectionsResizeMode();
    // m_playlist->clear();
    QList<AudioTagInfo> historyList = DatabaseManager::instance().historyTracks();
    for (AudioTagInfo tagInfo : historyList)
    {
        QList<QStandardItem *> rowItems = tagInfo.toStandardItems();
        m_model->appendRow(rowItems);
        //m_playlist->addMedia(QUrl::fromLocalFile(tagInfo.sPath + "/" + tagInfo.sBaseFileName + "." + tagInfo.sExtension));
    }
    if (!historyList.isEmpty())
    {
        QModelIndex topIndex = m_model->index(0, 0);
        m_view->scrollTo(topIndex, QAbstractItemView::PositionAtTop);
        m_view->selectRow(0);
    }
    // if (m_model->rowCount() == 0 || m_model->columnCount() == 0)
    // return;
    // QModelIndex topLeft = m_model->index(0, 0);
    // QModelIndex bottomRight = m_model->index(m_model->rowCount() - 1, m_model->columnCount() - 1);
    // emit m_model->dataChanged(topLeft, bottomRight);
    m_playlist->clear();
    m_view->setModel(m_sortModel);
    int proxyRowCount = m_sortModel->rowCount();
    for (int proxyRow = 0; proxyRow < proxyRowCount; ++proxyRow)
    {
        // Get proxy index for column 0 (filename)
        QModelIndex proxyIndex = m_sortModel->index(proxyRow, 0);
        // Map proxy index to source index
        QModelIndex sourceIndex = m_sortModel->mapToSource(proxyIndex);
        if (!proxyIndex.isValid())
            continue;
        int sourceRow = sourceIndex.row();
        // Read data from SOURCE model
        QStandardItem *item = m_model->item(sourceRow, 0);
        if (!item)
            continue;
        QString fullPath = item->data(Qt::UserRole + 1).toString();
        //qDebug()<<"fullPath="<< fullPath;
        m_playlist->addMedia(QUrl::fromLocalFile(fullPath));
        alignColumns(proxyRow);
    }
    emit playlistUpdated(m_playlist);
    m_view->setSortingEnabled(true);
    m_HorizontalHeader->setSortIndicatorShown(true);
}

void PlaylistTable::on_pushButtonPlaylist_clicked()
{
    //saveSessionPlaylist();
    m_bSessionPlaylistIsShown = true;
    m_model->clear();
    m_model->setColumnCount(ColumnIndex::ColumnCount);
    for (int i = 0; i < ColumnIndex::ColumnCount; ++i)
        m_model->setHeaderData(i, Qt::Horizontal, ColumnIndex::headerName(i));
    // m_model->setHorizontalHeaderLabels(QStringList()
    // << "Filename" << "Ext" << "Path" << "Duration" << "Artist"
    // << "Title" << "Album" << "Track" << "Year" << "Genre"
    // << "Comment" << "Bitrate" << "Samplerate" << "Bits" << "Channels"
    // << "Format" << "Cover size" << "File size"); m_playlist->clear();
    /*
      // Prima imposta il resize mode
    m_view->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    // Poi imposta i limiti
    m_view->verticalHeader()->setDefaultSectionSize(16);
    m_view->verticalHeader()->setMaximumSectionSize(32);

    // Opzionale: imposta anche minimum
    m_view->verticalHeader()->setMinimumSectionSize(16);
    */
    m_view->verticalHeader()->setDefaultSectionSize(16);
    m_view->verticalHeader()->setMaximumSectionSize(32);
    restoreColumnWidths();
    restoreColumnVisibility();
    m_HorizontalHeader->setSortIndicator(-1, Qt::AscendingOrder);
    m_HorizontalHeader->setSortIndicatorShown(false);
    //on_pushButton_clicked();
    //on_pushButton_2_clicked ();
    //setSectionsResizeMode();
    //if (m_model->rowCount() > 0) onHeaderSortChanged(iSortCol, order);
    //if (m_view->horizontalHeader()->isSortIndicatorShown()) onHeaderSortChanged(iSortCol, order);
    // setSectionsResizeMode();
    // m_playlist->clear();
    QList<AudioTagInfo> favList = DatabaseManager::instance().loadSessionPlaylist();
    for (AudioTagInfo tagInfo : favList)
    {
        QList<QStandardItem *> rowItems = tagInfo.toStandardItems();
        m_model->appendRow(rowItems);
        //m_playlist->addMedia(QUrl::fromLocalFile(tagInfo.sPath + "/" + tagInfo.sBaseFileName + "." + tagInfo.sExtension));
    }
    if (!favList.isEmpty())
    {
        QModelIndex topIndex = m_model->index(0, 0);
        m_view->scrollTo(topIndex, QAbstractItemView::PositionAtTop);
        m_view->selectRow(0);
    }
    m_playlist->clear();
    m_view->setModel(m_sortModel);
    int proxyRowCount = m_sortModel->rowCount();
    for (int proxyRow = 0; proxyRow < proxyRowCount; ++proxyRow)
    {
        // Get proxy index for column 0 (filename)
        QModelIndex proxyIndex = m_sortModel->index(proxyRow, 0);
        // Map proxy index to source index
        QModelIndex sourceIndex = m_sortModel->mapToSource(proxyIndex);
        if (!proxyIndex.isValid())
            continue;
        int sourceRow = sourceIndex.row();
        // Read data from SOURCE model
        QStandardItem *item = m_model->item(sourceRow, 0);
        if (!item)
            continue;
        QString fullPath = item->data(Qt::UserRole + 1).toString();
        //qDebug()<<"fullPath="<< fullPath;
        m_playlist->addMedia(QUrl::fromLocalFile(fullPath));
        alignColumns(proxyRow);
    }
    //emit playlistUpdated(m_playlist);
    int iSortCol = settingsMgr->value("PlaylistViewSortColumn", 0).toInt();
    Qt::SortOrder order = static_cast<Qt::SortOrder>(settingsMgr->value("PlaylistViewSortColumnOrder", 0).toInt());
    m_sortModel->sort(iSortCol, order);
    m_view->setSortingEnabled(true);
    m_view->horizontalHeader()->setSortIndicator(iSortCol, order);
    m_HorizontalHeader->setSortIndicatorShown(true);
    //onCurrentTrackChanged(m_iLastPlaylistFile);
    // if (m_CurrentItem)
    // {
    //            //QModelIndex sourceIndex = m_model->index(m_CurrentItem->row(), 0);
    // QModelIndex proxyIndex = m_sortModel->index(mapSourceRowToProxy(m_model, m_sortModel, m_CurrentItem->row()), 0);
    // m_view->scrollTo(proxyIndex, QAbstractItemView::PositionAtCenter);//EnsureVisible);
    // }
}

void PlaylistTable::setupToolButton()
{
    toolButtonMenu = new QMenu(this);
    resetAllPlayCountsAction = new QAction("Reset all play counts", this);
    deleteInexistentFilesAction = new QAction("Delete inexistent files", this);
    toolButtonMenu->addAction(resetAllPlayCountsAction);
    toolButtonMenu->addAction(deleteInexistentFilesAction);
    ui->toolButton->setMenu(toolButtonMenu);
    // 4. Imposta la popupMode su InstantPopup
    // Questo è il passaggio chiave per mostrare il menu al clic!
    // ui->toolButton->setPopupMode(QToolButton::InstantPopup);
    connect(resetAllPlayCountsAction, &QAction::triggered, this, &PlaylistTable::resetAllPlayCounts);
    //connect(deleteInexistentFilesAction, &QAction::triggered, this, &PlaylistTable::deleteInexistentFiles);
    connect(deleteInexistentFilesAction, &QAction::triggered, [this]()
    {
        DatabaseManager::instance().deleteInexistentFiles();
    });
}

void PlaylistTable::resetAllPlayCounts()
{
    DatabaseManager &db = DatabaseManager::instance();
    db.resetAllPlayCounts();
    int iRows = m_view->model()->rowCount();
    for (int iIdx = 0; iIdx < iRows; iIdx++)
    {
        QStandardItem *playCountItem = m_model->item(iIdx, ColumnIndex::PlayCount);
        if (playCountItem)
        {
            playCountItem->setText(QString::number(0));
            playCountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            emit m_model->dataChanged(playCountItem->index(), playCountItem->index(),
            {Qt::DisplayRole});
        }
    }
}
