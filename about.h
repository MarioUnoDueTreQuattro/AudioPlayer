#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

namespace Ui {
class about;
}

class about : public QDialog
{
    Q_OBJECT

public:
    explicit about(QWidget *parent = nullptr);
    ~about();

    void setupWidgets();

private slots:
    void on_pushButtonOk_clicked();

    void on_pushButtonAboutQt_clicked();

private:
    Ui::about *ui;
};

#endif // ABOUT_H
