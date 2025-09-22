#include "settings.h"
#include "ui_settings.h"
#include <QApplication>
//#include <QDateTime>
#include <QSettings>
//#include <QDir>
#include <QStyleFactory>
#include <QPalette>
#include <QDebug>

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

    //QApplication::setStyle(QStyleFactory::create("Windows"));
    // QApplication::setStyle(QStyleFactory::create("WindowsXP"));
    // QApplication::setStyle(QStyleFactory::create("WindowsVista"));
    // Detect available styles
    QStringList availableStyles = QStyleFactory::keys();
    qDebug() << "Available styles:" << availableStyles;
    ui->comboBoxTheme->addItems (availableStyles);
    ui->comboBoxColor->addItem ("Light");
    ui->comboBoxColor->addItem ("Dark");
    // Prefer WindowsVista (default look on Windows 7 with Aero)
    if (availableStyles.contains("WindowsVista", Qt::CaseInsensitive))
    {
       // QApplication::setStyle(QStyleFactory::create("WindowsVista"));
        qDebug() << "Setting WindowsVista style";
        ui->comboBoxTheme->setCurrentText ("WindowsVista");
    }
    else
    {
       // QApplication::setStyle(QStyleFactory::create("Windows")); // Fallback for Classic/Basic theme
        qDebug() << "Setting Windows style";
        ui->comboBoxTheme->setCurrentText ("Windows");
    }


}

Settings::~Settings()
{
    delete ui;
}

void Settings::on_comboBoxTheme_activated(const QString &arg1)
{

    QApplication::setStyle(QStyleFactory::create(arg1));

    // Force widgets to re-polish
       foreach (QWidget *widget, QApplication::allWidgets())
       {
           widget->style()->unpolish(widget);
           widget->style()->polish(widget);
           widget->update();
       }

}

void Settings::on_comboBoxColor_activated(const QString &arg1)
{
   if (arg1=="Dark"){
//    QPalette darkPalette;
//    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
//    darkPalette.setColor(QPalette::WindowText, Qt::white);
//    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
//    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
//    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
//    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
//    darkPalette.setColor(QPalette::Text, Qt::white);
//    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
//    darkPalette.setColor(QPalette::ButtonText, Qt::white);
//    darkPalette.setColor(QPalette::BrightText, Qt::red);
//    darkPalette.setColor(QPalette::Highlight, QColor(142, 45, 197).lighter());
//    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
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

void Settings::on_Settings_accepted()
{
    qDebug() << __PRETTY_FUNCTION__ ;
}
