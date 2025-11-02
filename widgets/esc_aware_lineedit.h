#ifndef ESCAWARELINEEDIT_H
#define ESCAWARELINEEDIT_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QDebug> // Utile per la stampa di debug

// Dichiarazione della nostra classe personalizzata
class EscAwareLineEdit : public QLineEdit
{
    // Macro necessaria per ogni classe QObject che definisce segnali, slot, ecc.
    Q_OBJECT

public:
    // Costruttore che accetta un widget padre opzionale
    explicit EscAwareLineEdit(QWidget *parent = nullptr);

protected:
    // La funzione chiave da sovrascrivere per gestire gli eventi della tastiera
    void keyPressEvent(QKeyEvent *event) override;
signals:
    // Dichiariamo un nuovo segnale.
    // Il nome suggerisce cosa è successo (tasto "Esc" premuto).
    void escapePressed();};

#endif // ESCAWARELINEEDIT_H
