#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QWidget>
#include <fileref.h>
#include <tag.h>
#include <mpegfile.h>
#include <mpegproperties.h>
#include <mp4file.h>
#include <flacfile.h>
#include <opusfile.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <vorbisfile.h>
#include <speexfile.h>
#include <wavfile.h>
#include <wavproperties.h>

namespace Ui
{
class InfoWidget;
}

class InfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InfoWidget(QWidget *parent = nullptr);
    ~InfoWidget();
    void setInfo(QString);
    QString getInfo();
    void setFile(const QString &);
    QString getPictuePosition() const;
    void setPictuePosition(const QString &sPictuePosition);

private slots:
    //void on_pushButtonClose_clicked();
    void updateSize(const QSizeF &newSize);
    QString formatFileSize(qint64 bytes);
private:
    Ui::InfoWidget *ui;
    void loadSettings();
    void saveSettings();
    QString formatTime(int totalSeconds);
    TagLib::FileRef *m_FileRef;
    QString m_Info;
    QString m_sPictuePosition;
    // bool m_bHasPicture;
    QPixmap m_pix;
    QPixmap extractMP3Cover(TagLib::MPEG::File *mp3File);
    QPixmap extractMP4Cover(TagLib::MP4::File *file);
    QPixmap extractFLACCover(TagLib::FLAC::File *file);
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
signals:
    void windowClosed();
    void focusReceived();
};

#endif // INFOWIDGET_H
