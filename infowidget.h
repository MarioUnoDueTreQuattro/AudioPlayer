#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QWidget>
#include <fileref.h>
#include <flacfile.h>
#include <mp4file.h>
#include <mpegfile.h>
#include <mpegproperties.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <opusfile.h>
#include <speexfile.h>
#include <tag.h>
#include <vorbisfile.h>
#include <wavfile.h>
#include <wavproperties.h>
//#include <mp4properties.h>
//#include <tpropertymap.h>
#include "settingsmanager.h"

namespace Ui
{
class InfoWidget;
}

class InfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InfoWidget(QWidget *parent = nullptr);
    ~InfoWidget() override;
    void setInfo(QString);
    QString getInfo();
    void setFile(const QString &);
    QString getPictuePosition() const;
    void setPictuePosition(const QString &sPictuePosition);
    bool getScalePixOriginalSize() const;
    void setScalePixOriginalSize(bool bScalePixOriginalSize);
    int getPixSize() const;
    void setPixSize(int iPixSize);
    void redrawBigPix();
    void setScalePixOriginalSizeMaxEnabled(bool bScalePixOriginalSizeMax);
    void setScalePixOriginalSizeMax(int iScalePixOriginalSizeMax);
private slots:
    //void on_pushButtonClose_clicked();
    void updateSize(const QSizeF &newSize);
   // QString formatFileSize(qint64 bytes);
    void pixClicked();

private:
    SettingsManager *settingsMgr;
    Ui::InfoWidget *ui;
    void loadSettings();
    void saveSettings();
   // QString formatTime(int totalSeconds);
    TagLib::FileRef *m_FileRef;
    QString m_Info;
    QString m_sPictuePosition;
    QString m_sPixSize;
    bool m_bScalePixOriginalSize;
    bool m_bScalePixOriginalSizeMax;
    int m_iScalePixOriginalSizeMax;
    int m_iPixSize;
    bool m_bPixIsBig;
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
