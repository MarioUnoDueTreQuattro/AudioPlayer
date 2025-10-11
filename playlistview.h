#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QMediaPlaylist>
#include <QMediaPlayer>

class PlaylistView : public QWidget
{
    Q_OBJECT

public:
    explicit PlaylistView(QMediaPlayer *player, QWidget *parent = 0);
    ~PlaylistView();

    void addTrack(const QString &filePath);
    void clear();

    QMediaPlaylist *mediaPlaylist() const { return m_playlist; }

signals:
    void trackActivated(int index);

public slots:
    void onDoubleClicked(const QModelIndex &index);
    void onCurrentTrackChanged(int index);

private:
    QString extractFileName(const QString &filePath);

    QMediaPlayer   *m_player;
    QMediaPlaylist *m_playlist;
    QStandardItemModel *m_model;
    QTableView *m_view;
};

#endif // PLAYLISTVIEW_H
