#include "playlistview.h"
#include <QFileInfo>
#include <QVBoxLayout>
#include <QDebug>
#include <QHeaderView>

PlaylistView::PlaylistView(QMediaPlayer *player, QWidget *parent)
    : QWidget(parent),
      m_player(player)
{
    // --- Create playlist ---
    m_playlist = new QMediaPlaylist(this);
    m_player->setPlaylist(m_playlist);

    // --- Create model ---
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels(QStringList() << "Title" << "Path");

    // --- Create table view ---
    m_view = new QTableView(this);
    m_view->setModel(m_model);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->horizontalHeader()->setStretchLastSection(true);
    m_view->verticalHeader()->hide();

    // --- Layout ---
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_view);
    setLayout(layout);

    // --- Connections ---
    connect(m_view, SIGNAL(doubleClicked(const QModelIndex &)),
            this, SLOT(onDoubleClicked(const QModelIndex &)));

    connect(m_playlist, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onCurrentTrackChanged(int)));
}

PlaylistView::~PlaylistView()
{
}

void PlaylistView::addTrack(const QString &filePath)
{
    QString fileName = extractFileName(filePath);

    QList<QStandardItem*> rowItems;
    rowItems << new QStandardItem(fileName)
             << new QStandardItem(filePath);

    m_model->appendRow(rowItems);
    m_playlist->addMedia(QUrl::fromLocalFile(filePath));
}

void PlaylistView::clear()
{
    m_model->clear();
    m_model->setHorizontalHeaderLabels(QStringList() << "Title" << "Path");
    m_playlist->clear();
}

QString PlaylistView::extractFileName(const QString &filePath)
{
    QFileInfo info(filePath);
    return info.completeBaseName();
}

void PlaylistView::onDoubleClicked(const QModelIndex &index)
{
    int row = index.row();
    m_playlist->setCurrentIndex(row);
    m_player->play();
    emit trackActivated(row);
}

void PlaylistView::onCurrentTrackChanged(int index)
{
    if (index < 0)
        return;
    m_view->selectRow(index);
}
