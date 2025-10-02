#include "clickablelabel.h"
#include <QDebug>

// Constructor Implementation
ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent, f),
      sManagedTooltip("")
{
    // Set cursor to a pointer hand to visually indicate the label is clickable
    setCursor(Qt::PointingHandCursor);
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);  // 2 seconds
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onInactivityTimeout()));
}

QString ClickableLabel::getManagedTooltip() const
{
    return sManagedTooltip;
}

void ClickableLabel::setManagedTooltip(const QString &value)
{
    sManagedTooltip = value;
}

void ClickableLabel::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (!sManagedTooltip.isEmpty ())
    {
        // Manually show tooltip at the cursor position
        // QPoint globalPos = QCursor::pos();
        QPoint globalPos = mapToGlobal(QPoint(0, this->height ()));
        //QToolTip::showText(globalPos, this->toolTip (), this, QRect (50, 50, 100, 100), 2000);
        QToolTip::showText(globalPos, sManagedTooltip, this);
    }
    QLabel::enterEvent(event);
    // Start timer when mouse enters
    m_timer->start();
    // Make sure cursor is visible when entering
    //unsetCursor();
    setCursor(Qt::PointingHandCursor);
    m_cursorHidden = false;
    m_lastMousePos = mapFromGlobal(QCursor::pos());
}

void ClickableLabel::leaveEvent(QEvent *event)
{
    QToolTip::hideText();
    QLabel::leaveEvent(event);
    // Stop timer and show cursor when mouse leaves
    m_timer->stop();
    // unsetCursor();
    setCursor(Qt::PointingHandCursor);
    m_cursorHidden = false;
}

void ClickableLabel::mouseMoveEvent(QMouseEvent *event)
{
    QLabel::mouseMoveEvent(event);
    QPoint currentPos = event->pos();
    // Ignore small movements (<3 px in any direction)
    if (qAbs(currentPos.x() - m_lastMousePos.x()) < 4 &&
        qAbs(currentPos.y() - m_lastMousePos.y()) < 4)
    {
        return;
    }
    m_lastMousePos = currentPos;
    // Restart inactivity timer
    m_timer->start();
    // If cursor was hidden, show it again immediately
    if (m_cursorHidden)
    {
        // unsetCursor();
        setCursor(Qt::PointingHandCursor);
        m_cursorHidden = false;
    }
}

void ClickableLabel::onInactivityTimeout()
{
    // Hide cursor when inactive
    setCursor(Qt::BlankCursor);
    m_cursorHidden = true;
}

// Override implementation of the mouse press event
void ClickableLabel::mousePressEvent(QMouseEvent* event)
{
    // Check if the pressed button was the left mouse button
    if (event->button() == Qt::LeftButton)
    {
        // Emit the custom signal
        emit clicked();
        //qDebug() << "ClickableLabel: Left button detected and clicked() signal emitted.";
    }
    // IMPORTANT: Always call the base class implementation.
    // This allows the QLabel to handle other events it normally manages.
    QLabel::mousePressEvent(event);
}
