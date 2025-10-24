#include "esc_aware_lineedit.h"

EscAwareLineEdit::EscAwareLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    // Il costruttore non fa nulla di speciale, chiama solo il costruttore della base
    setPlaceholderText("Press ESC to close");
}

void EscAwareLineEdit::keyPressEvent(QKeyEvent *event)
{
    // **1. Controllo del Tasto:**
    // Verifichiamo se l'evento tastiera corrisponde al tasto "Escape"
    if (event->key() == Qt::Key_Escape)
    {
        // Tasto "Esc" rilevato! 🎉
        qDebug() << "Tasto ESC premuto! Intercetto l'evento.";
        // **2. Azione desiderata:**
        // Ad esempio, potremmo voler svuotare il campo di testo.
        this->clear();
        emit escapePressed();
        // Se vogliamo che l'evento si fermi qui e non venga
        // propagato al genitore o ad altre implementazioni,
        // chiamiamo event->accept() e usciamo.
        event->accept();
    }
    else
    {
        // **3. Gestione di Altri Tasti:**
        // Se non è il tasto "Esc", dobbiamo chiamare l'implementazione
        // della classe base (QLineEdit) per permettere la normale
        // digitazione del testo.
        QLineEdit::keyPressEvent(event);
    }
}
