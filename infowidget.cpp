#include "infowidget.h"
#include "ui_infowidget.h"
#include <QMouseEvent>
#include <QTextEdit>
#include <QSize>
#include <QAbstractTextDocumentLayout>
#include <QTextDocument>
#include <QDebug>
//#include <QScreen>
#include <QSettings>
#include <QPalette>
#include <QIcon>

InfoWidget::InfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoWidget)
{
    ui->setupUi(this);
    setWindowIcon (QIcon(":/img/img/icons8-play-32.ico"));
    setWindowFlags(Qt::Tool | Qt::MSWindowsFixedSizeDialogHint);
    loadSettings();
    QPalette palette = ui->textEditInfo->palette();
    palette.setColor(QPalette::Base, palette.color(QPalette::Window));
    ui->textEditInfo->setPalette (palette);
    // Imposta la forma del frame a nessuna forma
    ui->textEditInfo->setFrameShape(QFrame::NoFrame);
    // Imposta l'ombra del frame a nessuna ombra
    ui->textEditInfo->setFrameShadow(QFrame::Plain);
    ui->textEditInfo->move(0, 0);
    ui->textEditInfo-> setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    // 1. Disabilita il ritorno a capo per far contare la larghezza
    ui->textEditInfo-> setLineWrapMode(QTextEdit::NoWrap);
    // 2. Disattiva le barre di scorrimento su entrambi gli assi
    ui->textEditInfo-> setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->textEditInfo-> setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QTextDocument *doc = ui->textEditInfo->document ();
    QAbstractTextDocumentLayout *docLayout = doc->documentLayout ();
    connect( docLayout, &QAbstractTextDocumentLayout::documentSizeChanged,
        this, updateSize);
}

InfoWidget::~InfoWidget()
{
    delete ui;
}

void InfoWidget::updateSize(const QSizeF &newSize)
{
    //       // Calcola l'altezza del contenuto e del widget
    //       // 1. Altezza del documento (incluso lo spazio extra se necessario)
    // int contentHeight = newSize.height();
    //       // 2. Calcola l'altezza aggiungendo margini, bordi, ecc.
    //       // I 'frameWidth()' sono essenziali per non tagliare il testo.
    // int totalHeight = contentHeight + 2 * ui->textEditInfo->frameWidth();
    //       // 3. Imposta l'altezza fissa del widget
    // ui->textEditInfo->setFixedHeight(totalHeight);
    // setFixedHeight(totalHeight);
    // Calcola l'altezza come prima
    int contentHeight = newSize.height();
    // Calcola la larghezza ideale (la riga più lunga)
    int contentWidth = ui->textEditInfo->document()->idealWidth();
    // Aggiungi margini, bordi e scroll bar (anche se disabilitati, a volte l'overhead resta)
    int totalHeight = contentHeight + 2 * ui->textEditInfo->frameWidth();;
    int totalWidth = contentWidth;// + 2 * ui->textEditInfo->frameWidth();
    // Imposta la dimensione fissa (sia altezza che larghezza)
    ui->textEditInfo->setFixedSize(totalWidth, totalHeight);
    setFixedSize(totalWidth, totalHeight);
}

void InfoWidget::setInfo(QString sInfo)
{
    ui->textEditInfo->setText (sInfo);
}

//void InfoWidget::on_pushButtonClose_clicked()
//{
// this->close ();
//}

void InfoWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // Chiamiamo la funzione close() per chiudere il widget di primo livello (la finestra).
        //qDebug("Finestra chiusa tramite click del mouse!");
        //close();
        //deleteLater();
    }
    // È buona norma chiamare l'implementazione della classe base
    // nel caso in cui ci siano altre elaborazioni da eseguire (anche se in questo caso non fa nulla di essenziale per la chiusura).
    QWidget::mousePressEvent(event);
}

void InfoWidget::moveEvent(QMoveEvent *event)
{
    // Chiamiamo la funzione di base per garantire che l'evento venga gestito normalmente
    QWidget::moveEvent(event);
    // 2. Chiama la funzione di salvataggio
    saveSettings();
    // (Opzionale) Puoi usare event->pos() per vedere la nuova posizione, ma
    // QWidget::pos() è più diretto dopo la chiamata alla base.
    //qDebug() << "Window moved to: " << pos();
}

void InfoWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::StyleChange || event->type() == QEvent::PaletteChange)
    {
        //qDebug() << "Stile del widget cambiato!";
        // Inserisci qui il tuo codice per reagire al cambio di stile
        //QPalette palette = ui->textEditInfo->palette();
        QPalette palette = this->palette();
        palette.setColor(QPalette::Base, palette.color(QPalette::Window));
        ui->textEditInfo->setPalette (palette);
    }
    // 2. Chiama l'implementazione della classe base
    QWidget::changeEvent(event);
}

void InfoWidget::closeEvent(QCloseEvent *event)
{
    emit windowClosed();
    event->accept();
    QWidget::closeEvent(event);
}

void InfoWidget::saveSettings()
{
    // QSettings richiede il nome dell'Organizzazione e dell'Applicazione
    // per creare un percorso di salvataggio univoco.
    QSettings settings;
    // Salva la posizione corrente del widget (coordinate x, y)
    settings.setValue("InfoWidgetPosition", pos());
    // Potresti anche salvare la dimensione (larghezza, altezza)
    settings.setValue("InfoWidgetSsize", size());
    // In Qt, le impostazioni vengono salvate automaticamente
}

void InfoWidget::loadSettings()
{
    QSettings settings;
    // 1. Carica la posizione
    QPoint savedPos = settings.value("InfoWidgetPosition", QPoint(100, 100)).toPoint();
    // 2. Carica la dimensione
    QSize savedSize = settings.value("InfoWidgetSsize", QSize(300, 100)).toSize();
    // 3. Applica le impostazioni
    resize(savedSize);
    // Controlla se la posizione salvata è visibile su qualsiasi schermo
    // (Utile se l'utente ha staccato un monitor).
    QScreen *screen = QGuiApplication::screenAt(savedPos);
    if (screen)
    {
        move(savedPos);
    }
    else
    {
        // Se la posizione non è valida, centra la finestra
        // La posizione di default (100, 100) verrà usata se la chiave non esiste
        move(QPoint(100, 100));
    }
}
