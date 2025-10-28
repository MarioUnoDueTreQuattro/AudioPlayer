#ifndef SETTINGS_H
#define SETTINGS_H

#include "settings_manager.h"
#include <QDialog>
#include <QDialogButtonBox>

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
    void on_pushButtonPlayedTextColor_clicked();
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_checkBoxScaleOriginalSize_stateChanged(int arg1);
    void on_checkBoxScaleOriginalSizeMax_stateChanged(int arg1);
    void on_checkBoxFade_stateChanged(int arg1);
    void on_pushButtonFadeColor_clicked();

    void on_pushButtonResetFade_clicked();

protected:
    void moveEvent(QMoveEvent *event) override;
private:
     SettingsManager *settingsMgr;
   Ui::Settings *ui;
    QString m_sTheme;
    QString m_sPalette;
    QColor m_playedTextColor;
    QColor m_FadeIndicatorColor;
    QString m_sPictuePositionInInfo;
    void saveSettings();
    void loadSettings();
signals:
    void applyClicked();  // custom signal
};

#endif // SETTINGS_H
