#include "settings.h"
#include "ui_settings.h"
#include <QApplication>
//#include <QDateTime>
//#include <QSettings>
//#include <QDir>
#include <QStyleFactory>
#include <QPalette>
#include <QDebug>
//#include <QSettings>
#include <QColorDialog>
#include <QDesktopWidget>

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
    settingsMgr = SettingsManager::instance();
    loadSettings();
    //QApplication::setStyle(QStyleFactory::create("Windows"));
    // QApplication::setStyle(QStyleFactory::create("WindowsXP"));
    // QApplication::setStyle(QStyleFactory::create("WindowsVista"));
    // Detect available styles
    QStringList availableStyles = QStyleFactory::keys();
    qDebug() << "Available styles:" << availableStyles;
    ui->comboBoxTheme->addItems(availableStyles);
    ui->comboBoxColor->addItem("Light");
    ui->comboBoxColor->addItem("Dark");
    if (m_sTheme != "")
    {
        ui->comboBoxTheme->setCurrentText(m_sTheme);
    }
    else
    {
        // Prefer WindowsVista (default look on Windows 7 with Aero)
        if (availableStyles.contains("WindowsVista", Qt::CaseInsensitive))
        {
            // QApplication::setStyle(QStyleFactory::create("WindowsVista"));
            qDebug() << "Setting WindowsVista style";
            ui->comboBoxTheme->setCurrentText("WindowsVista");
            m_sTheme = "WindowsVista";
        }
        else
        {
            // QApplication::setStyle(QStyleFactory::create("Windows")); // Fallback for Classic/Basic theme
            qDebug() << "Setting Windows style";
            ui->comboBoxTheme->setCurrentText("Windows");
            m_sTheme = "Windows";
        }
    }
    ui->comboBoxColor->setCurrentText(m_sPalette);
    // Adjust the size of the dialog to fit its contents
    adjustSize();
}

Settings::~Settings()
{
    delete ui;
}

void Settings::on_comboBoxTheme_activated(const QString &arg1)
{
    m_sTheme = arg1;
}

void Settings::on_comboBoxColor_activated(const QString &arg1)
{
    m_sPalette = arg1;
}

void Settings::on_Settings_accepted()
{
    qDebug() << __PRETTY_FUNCTION__ ;
    saveSettings();
    if (m_sTheme != "")
    {
        QApplication::setStyle(QStyleFactory::create(m_sTheme));
    }
    if (m_sPalette != "")
    {
        if (m_sPalette == "Dark")
        {
            // QPalette darkPalette;
            // darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
            // darkPalette.setColor(QPalette::WindowText, Qt::white);
            // darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
            // darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
            // darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
            // darkPalette.setColor(QPalette::ToolTipText, Qt::white);
            // darkPalette.setColor(QPalette::Text, Qt::white);
            // darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
            // darkPalette.setColor(QPalette::ButtonText, Qt::white);
            // darkPalette.setColor(QPalette::BrightText, Qt::red);
            // darkPalette.setColor(QPalette::Highlight, QColor(142, 45, 197).lighter());
            // darkPalette.setColor(QPalette::HighlightedText, Qt::black);
            QPalette darkPalette;
            darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
            darkPalette.setColor(QPalette::WindowText, Qt::white);
            darkPalette.setColor(QPalette::Base, QColor(35, 35, 35));
            darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
            darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
            darkPalette.setColor(QPalette::ToolTipText, Qt::white);
            darkPalette.setColor(QPalette::Text, Qt::white);
            darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
            darkPalette.setColor(QPalette::ButtonText, Qt::white);
            darkPalette.setColor(QPalette::BrightText, Qt::red);
            darkPalette.setColor(QPalette::Highlight, QColor(173, 216, 230));
            darkPalette.setColor(QPalette::HighlightedText, Qt::black);
            QColor disabledWindowText(120, 120, 120);
            QColor disabledText(100, 100, 100);
            QColor disabledButtonText(130, 130, 130);
            darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, disabledWindowText);
            darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledText);
            darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledButtonText);
            // darkPalette.setColor(QPalette::Active, QPalette::WindowText, QColor(255,215,0));
            // darkPalette.setColor(QPalette::Inactive, QPalette::ButtonText,  QColor(255,215,0));
            //darkPalette.setColor(QPalette::Active, QPalette::ButtonText,  QColor(255,215,0));
            //darkPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(25, 25, 25)); // optional - darker input background
            // darkPalette.setColor(QPalette::Disabled, QPalette::Button, QColor(45, 45, 45)); // optional - duller buttons
            // QColor disabledColor(140, 140, 140);
            // darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, disabledColor);
            // darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
            // darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
            QApplication::setPalette(darkPalette);
        }
        else
        {
            QApplication::setPalette(QApplication::style()->standardPalette());
        }
    }
    if (m_sTheme != "")
    {
        // Force widgets to re-polish
        foreach (QWidget *widget, QApplication::allWidgets())
        {
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
            widget->update();
        }
    }
}

void Settings::saveSettings()
{
    //QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settingsMgr->setValue("Theme", m_sTheme);
    settingsMgr->setValue("ThemePalette", m_sPalette);
    settingsMgr->setValue("PlayedTextColor", m_playedTextColor.name());
    settingsMgr->setValue("AutoPlay", ui->checkBoxAutoPlay->isChecked());
    settingsMgr->setValue("ShowInfo", ui->checkBoxInfo->isChecked());
    QString sPosition;
    if (ui->radioButtonPixAbove->isChecked()) sPosition = "Above";
    else if (ui->radioButtonPixBelow->isChecked()) sPosition = "Below";
    else if (ui->radioButtonPixRight->isChecked()) sPosition = "Right";
    else if (ui->radioButtonPixLeft->isChecked()) sPosition = "Left";
    settingsMgr->setValue("PictuePositionInInfo", sPosition);
    settingsMgr->setValue("PictueScaleOriginalSize", ui->checkBoxScaleOriginalSize->isChecked());
    settingsMgr->setValue("PictueScaleSize", ui->spinBoxPixSize->value());
    settingsMgr->setValue("PictueScaleOriginalSizeMaxEnabled", ui->checkBoxScaleOriginalSizeMax->isChecked());
    settingsMgr->setValue("PictueScaleOriginalSizeMax", ui->spinBoxScaleOriginalSizeMax->value());
    settingsMgr->setValue("VolumeFade", ui->checkBoxFade->isChecked());
    settingsMgr->setValue("VolumeFadeTime", ui->spinBoxFade->value());
    settingsMgr->setValue("SettingsPosition", pos());
    settingsMgr->setValue("EnhancedPlaylist", ui->checkBoxEnhancedPlaylist->isChecked());
    settingsMgr->sync();
}

void Settings::loadSettings()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int pos_x = (screenGeometry.width() - this->width()) / 2;
    int pos_y = (screenGeometry.height() - this->height()) / 2;
    //QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    QPoint savedPos = settingsMgr->value("SettingsPosition", QPoint(pos_x, pos_y)).toPoint();
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
    m_sTheme = settingsMgr->value("Theme").toString();
    m_sPalette = settingsMgr->value("ThemePalette", "Light").toString();
    QString colorName = settingsMgr->value("PlayedTextColor", "#000080").toString();
    m_playedTextColor = QColor(colorName);
    QPalette palette = ui->pushButtonPlayedTextColor->palette();
    palette.setColor(QPalette::Button, m_playedTextColor);
    ui->pushButtonPlayedTextColor->setAutoFillBackground(true);
    ui->pushButtonPlayedTextColor->setPalette(palette);
    bool bAutoplay = settingsMgr->value("AutoPlay", true).toBool();
    ui->checkBoxAutoPlay->setChecked(bAutoplay);
    bool bShowInfo = settingsMgr->value("ShowInfo", true).toBool();
    ui->checkBoxInfo->setChecked(bShowInfo);
    //ui->groupBoxInfo->setEnabled (bShowInfo);
    m_sPictuePositionInInfo = settingsMgr->value("PictuePositionInInfo", "Right").toString();
    if (m_sPictuePositionInInfo == "Above") ui->radioButtonPixAbove->setChecked(true);
    else if (m_sPictuePositionInInfo == "Below") ui->radioButtonPixBelow->setChecked(true);
    else if (m_sPictuePositionInInfo == "Right") ui->radioButtonPixRight->setChecked(true);
    else if (m_sPictuePositionInInfo == "Left") ui->radioButtonPixLeft->setChecked(true);
    bool bScaleOriginalSizeMax = settingsMgr->value("PictueScaleOriginalSizeMaxEnabled", true).toBool();
    ui->checkBoxScaleOriginalSizeMax->setChecked(bScaleOriginalSizeMax);
    if (!bScaleOriginalSizeMax)
        ui->spinBoxScaleOriginalSizeMax->setEnabled(false);
    else
        ui->spinBoxScaleOriginalSizeMax->setEnabled(true);
    bool bScaleOriginalSize = settingsMgr->value("PictueScaleOriginalSize", true).toBool();
    ui->checkBoxScaleOriginalSize->setChecked(bScaleOriginalSize);
    if (!bScaleOriginalSize)
    {
        ui->spinBoxPixSize->setEnabled(true);
        ui->labelPixSize->setEnabled(true);
        if (!bScaleOriginalSizeMax) ui->spinBoxScaleOriginalSizeMax->setEnabled(false);
        else
            ui->spinBoxScaleOriginalSizeMax->setEnabled(false);
        ui->checkBoxScaleOriginalSizeMax->setEnabled(false);
    }
    else
    {
        ui->spinBoxPixSize->setEnabled(false);
        ui->labelPixSize->setEnabled(false);
        if (!bScaleOriginalSizeMax) ui->spinBoxScaleOriginalSizeMax->setEnabled(false);
        else
            ui->spinBoxScaleOriginalSizeMax->setEnabled(true);
        ui->checkBoxScaleOriginalSizeMax->setEnabled(true);
    }
    int iPixSize = settingsMgr->value("PictueScaleSize", 300).toInt();
    ui->spinBoxPixSize->setValue(iPixSize);
    int iScalePixOriginalSizeMax = settingsMgr->value("PictueScaleOriginalSizeMax", 600).toInt();
    ui->spinBoxScaleOriginalSizeMax->setValue(iScalePixOriginalSizeMax);
    bool bFade = settingsMgr->value("VolumeFade", true).toBool();
    ui->checkBoxFade->setChecked(bFade);
    if (!bFade)
        ui->spinBoxFade->setEnabled(false);
    else
        ui->spinBoxFade->setEnabled(true);
    int iFadeTime = settingsMgr->value("VolumeFadeTime", 1000).toInt();
    ui->spinBoxFade->setValue(iFadeTime);
    bool bEnhancedPlaylist = settingsMgr->value("EnhancedPlaylist", true).toBool();
    ui->checkBoxEnhancedPlaylist->setChecked(bEnhancedPlaylist);
}

void Settings::on_pushButtonPlayedTextColor_clicked()
{
    // Open color dialog, with white as default
    QColor chosenColor = QColorDialog::getColor(m_playedTextColor, this, "Select a color");
    if (chosenColor.isValid())
    {
        m_playedTextColor = chosenColor;
        QPalette palette = ui->pushButtonPlayedTextColor->palette();
        palette.setColor(QPalette::Button, m_playedTextColor);
        ui->pushButtonPlayedTextColor->setAutoFillBackground(true);
        ui->pushButtonPlayedTextColor->setPalette(palette);
        //ui->pushButtonPlayedTextColor->update ();
    }
}

void Settings::on_buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox *buttonBox = qobject_cast<QDialogButtonBox *>(sender());
    if (!buttonBox) return;
    QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(button);
    switch (role)
    {
        case QDialogButtonBox::ApplyRole:
            qDebug() << "Apply clicked -> apply changes without closing";
            //applySettings();
            on_Settings_accepted();
            emit applyClicked();   // <-- EMIT CUSTOM SIGNAL
            break;
        // case QDialogButtonBox::AcceptRole:
        // qDebug() << "OK clicked -> apply + close";
        // applySettings();
        // accept();
        // break;
        // case QDialogButtonBox::RejectRole:
        // qDebug() << "Cancel clicked -> discard + close";
        // reject();
        // break;
        default:
            break;
    }
}

void Settings::on_checkBoxScaleOriginalSize_stateChanged(int checkState)
{
    // ui->spinBoxPixSize->setStyleSheet("QSpinBox:disabled { background-color: #e0e0e0; color: #a0a0a0; }");
    // ui->labelPixSize->setStyleSheet("QLabel:disabled { color: #a0a0a0; }");
    if (checkState == 0)
    {
        ui->spinBoxPixSize->setEnabled(true);
        ui->labelPixSize->setEnabled(true);
        ui->spinBoxScaleOriginalSizeMax->setEnabled(false);
        ui->checkBoxScaleOriginalSizeMax->setEnabled(false);
    }
    else
    {
        ui->spinBoxPixSize->setEnabled(false);
        ui->labelPixSize->setEnabled(false);
        ui->checkBoxScaleOriginalSizeMax->setEnabled(true);
        if (ui->checkBoxScaleOriginalSizeMax->isChecked()) ui->spinBoxScaleOriginalSizeMax->setEnabled(true); else ui->spinBoxScaleOriginalSizeMax->setEnabled(false);
    }
}

void Settings::on_checkBoxScaleOriginalSizeMax_stateChanged(int checkState)
{
    if (checkState == 0)
    {
        ui->spinBoxScaleOriginalSizeMax->setEnabled(false);
    }
    else
    {
        ui->spinBoxScaleOriginalSizeMax->setEnabled(true);
    }
}

void Settings::on_checkBoxFade_stateChanged(int checkState)
{
    if (checkState == 0)
    {
        ui->spinBoxFade->setEnabled(false);
    }
    else
    {
        ui->spinBoxFade->setEnabled(true);
    }
}

void Settings::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    //QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settingsMgr->setValue("SettingsPosition", pos());
}

//void Settings::centerWindow()
//{
// QRect screenGeometry = QApplication::desktop()->screenGeometry();
// int x = (screenGeometry.width() - this->width()) / 2;
// int y = (screenGeometry.height() - this->height()) / 2;
// this->move(x, y);
//}
