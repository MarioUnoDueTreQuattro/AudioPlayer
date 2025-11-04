#include "hover_menu_toolbutton.h"
#include <QMenu>
#include <QCursor>
#include <QApplication>

HoverMenuToolButton::HoverMenuToolButton(QWidget *parent)
    : QToolButton(parent),
      m_delayTimer(new QTimer(this)),
      m_closeCheckTimer(new QTimer(this))
{
    setPopupMode(QToolButton::InstantPopup);
    //setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setAutoRaise(false); // Look like QPushButton (not flat)
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //setMinimumHeight(26); // Approximate QPushButton height on Windows
    // Configure delay timer (wait before showing menu)
    m_delayTimer->setSingleShot(true);
    m_delayTimer->setInterval(250);
    connect(m_delayTimer, SIGNAL(timeout()), this, SLOT(showMenuOnHover()));
    // Configure periodic timer (check if mouse left button/menu)
    m_closeCheckTimer->setInterval(200);
    connect(m_closeCheckTimer, SIGNAL(timeout()), this, SLOT(checkMousePosition()));
    // Stop timers on destruction
    connect(this, SIGNAL(destroyed(QObject*)), m_delayTimer, SLOT(stop()));
    connect(this, SIGNAL(destroyed(QObject*)), m_closeCheckTimer, SLOT(stop()));
}

void HoverMenuToolButton::enterEvent(QEvent *event)
{
    // Start timer to show menu after delay
    m_delayTimer->start();
    QToolButton::enterEvent(event);
}

void HoverMenuToolButton::leaveEvent(QEvent *event)
{
    // Stop show timer if still waiting
    m_delayTimer->stop();
    QToolButton::leaveEvent(event);
    // Start monitoring mouse position (to close the menu)
    if (menu() && menu()->isVisible())
        m_closeCheckTimer->start();
}

void HoverMenuToolButton::showMenuOnHover()
{
    if (!menu())
        return;
    this->setDown(true);
    m_menuPointer = menu();
     // Quando il menu si chiude, rilascia il pulsante
    connect(m_menuPointer, SIGNAL(aboutToHide()), this, SLOT(release()), Qt::UniqueConnection);

    showMenu();
    m_closeCheckTimer->start();
}

void HoverMenuToolButton::release()
{
    setDown(false);
}

void HoverMenuToolButton::checkMousePosition()
{
    if (!m_menuPointer)
    {
        m_closeCheckTimer->stop();
        return;
    }
    if (!m_menuPointer->isVisible())
    {
        m_closeCheckTimer->stop();
        m_menuPointer.clear();
        return;
    }
    const QPoint globalPos = QCursor::pos();
    const QRect buttonRect = QRect(mapToGlobal(QPoint(0, 0)), size());
    const QRect menuRect(m_menuPointer->pos(), m_menuPointer->size());
    const QRect totalRect = buttonRect.united(menuRect);
    if (!totalRect.contains(globalPos))
    {
        m_menuPointer->close();
        m_closeCheckTimer->stop();
        m_menuPointer.clear();
    }
}
