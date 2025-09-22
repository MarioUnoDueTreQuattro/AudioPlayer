#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();

private slots:
    void on_comboBoxTheme_activated(const QString &arg1);
    void on_comboBoxColor_activated(const QString &arg1);

    void on_Settings_accepted();

private:
    Ui::Settings *ui;
};

#endif // SETTINGS_H
