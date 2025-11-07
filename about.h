#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

namespace Ui
{
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();
    void setupWidgets();
    void adjustTextEditColor();
private slots:
    void on_pushButtonOk_clicked();
    void on_pushButtonAboutQt_clicked();
    void adjustTextEditHeight();
private:
    Ui::AboutDialog *ui;
    QString getSystemInformationString();
};

#endif // ABOUT_H
