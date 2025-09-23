#include "settings.h"
#include "ui_settings.h"
#include <QApplication>
//#include <QDateTime>
#include <QSettings>
//#include <QDir>
#include <QStyleFactory>
#include <QPalette>
#include <QDebug>
#include <QSettings>
#include <QColorDialog>

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    loadSettings ();
    //QApplication::setStyle(QStyleFactory::create("Windows"));
    // QApplication::setStyle(QStyleFactory::create("WindowsXP"));
    // QApplication::setStyle(QStyleFactory::create("WindowsVista"));
    // Detect available styles
    QStringList availableStyles = QStyleFactory::keys();
    qDebug() << "Available styles:" << availableStyles;
    ui->comboBoxTheme->addItems (availableStyles);
    ui->comboBoxColor->addItem ("Light");
    ui->comboBoxColor->addItem ("Dark");
    if (m_sTheme != "")
    {
        ui->comboBoxTheme->setCurrentText (m_sTheme);
    }
    else
    {
        // Prefer WindowsVista (default look on Windows 7 with Aero)
        if (availableStyles.contains("WindowsVista", Qt::CaseInsensitive))
        {
            // QApplication::setStyle(QStyleFactory::create("WindowsVista"));
            qDebug() << "Setting WindowsVista style";
            ui->comboBoxTheme->setCurrentText ("WindowsVista");
            m_sTheme = "WindowsVista";
        }
        else
        {
            // QApplication::setStyle(QStyleFactory::create("Windows")); // Fallback for Classic/Basic theme
            qDebug() << "Setting Windows style";
            ui->comboBoxTheme->setCurrentText ("Windows");
            m_sTheme = "Windows";
        }
    }
    ui->comboBoxColor->setCurrentText (m_sPalette);
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
    saveSettings ();
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
            QApplication::setPalette(darkPalette);
        }
        else
        {
            QApplication::setPalette(QApplication::style()->standardPalette());
        }
    }
    if (m_sTheme != "" )
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
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settings.setValue ("Theme", m_sTheme);
    settings.setValue ("ThemePalette", m_sPalette);
    settings.setValue("PlayedTextColor", m_playedTextColor.name());
    settings.sync();
}

void Settings::loadSettings()
{
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    m_sTheme = settings.value("Theme").toString();
    m_sPalette = settings.value("ThemePalette", "Light").toString();
    QString colorName = settings.value("PlayedTextColor", "#000080").toString();
    m_playedTextColor = QColor(colorName);
    QPalette palette = ui->pushButtonPlayedTextColor->palette();
    palette.setColor(QPalette::Button, m_playedTextColor);
    ui->pushButtonPlayedTextColor->setAutoFillBackground(true);
    ui->pushButtonPlayedTextColor->setPalette(palette);
}

void Settings::on_pushButtonPlayedTextColor_clicked()
{
    // Open color dialog, with white as default
    QColor chosenColor = QColorDialog::getColor(m_playedTextColor, this, "Select a color");
    if (chosenColor.isValid())
    {
        m_playedTextColor=chosenColor;
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

//                 case QDialogButtonBox::AcceptRole:
//                     qDebug() << "OK clicked -> apply + close";
//                     applySettings();
//                     accept();
//                     break;

//                 case QDialogButtonBox::RejectRole:
//                     qDebug() << "Cancel clicked -> discard + close";
//                     reject();
//                     break;

                 default:
                     break;
                 }
}
