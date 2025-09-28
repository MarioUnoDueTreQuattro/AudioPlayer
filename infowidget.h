#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QWidget>
#include <fileref.h>
#include <tag.h>

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
private slots:
    //void on_pushButtonClose_clicked();
    void updateSize(const QSizeF &newSize);
private:
    Ui::InfoWidget *ui;
    void loadSettings();
    void saveSettings();
    QString formatTime(int totalSeconds);
    TagLib::FileRef *m_FileRef;
    QString m_Info;
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
signals:
    void windowClosed();

};

#endif // INFOWIDGET_H
