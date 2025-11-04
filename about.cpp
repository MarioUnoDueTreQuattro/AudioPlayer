#include "about.h"
#include "ui_about.h"
#include <QMessageBox>


about::about(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::about)
{
    ui->setupUi(this);
    setWindowTitle("About " + qApp->applicationName());
    setWindowFlags(Qt::Dialog|Qt::MSWindowsFixedSizeDialogHint);
    setupWidgets();
    this->adjustSize();
}

about::~about()
{
    delete ui;
}

void about::on_pushButtonOk_clicked()
{
    this->close();
}

void about::on_pushButtonAboutQt_clicked()
{
    QMessageBox::aboutQt(this);
}

void about::setupWidgets()
{
    ui->labelIcon->setPixmap(QPixmap(":/img/img/icons8-play-32.png"));
    QFont appFont = ui->labelAppName->font();
    appFont.setBold(true);
    appFont.setPixelSize(24);
    ui->labelAppName->setFont(appFont);
    ui->labelAppName->setText(qApp->applicationName());
    QString sIsDebug = "";
#ifdef QT_DEBUG
    sIsDebug = " Debug ";
#else
    sIsDebug = " Release ";
#endif
    QString sVersion = "<h3><p style=\"font-weight: bold; color: rgb(0, 0, 128);\">Version ";
    sVersion = sVersion + APP_VERSION;
    sVersion = sVersion + sIsDebug + "</p></h3>";
    sVersion=sVersion+    "<p>Build date and time: " + __DATE__ + " at " + __TIME__ + "</p>";
    ui->labelAppVersion->setText(sVersion);
    ui->labelAppCopyright->setText(    "<br><p>Copyright &copy; 2025 Andrea G.</p>"
    "<p>All rights reserved.</p>"
        /* "<p>Visit our website: <a href='https://www.example.com'>www.example.com</a></p>"*/);
        ui->textEditInfo->hide();
}
