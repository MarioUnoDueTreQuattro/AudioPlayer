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

    // Delay before showing the menu
    m_delayTimer->setSingleShot(true);
    m_delayTimer->setInterval(500);
    connect(m_delayTimer, SIGNAL(timeout()), this, SLOT(showMenuOnHover()));

    // Timer to check when to close the menu
    m_closeCheckTimer->setInterval(200);
    connect(m_closeCheckTimer, SIGNAL(timeout()), this, SLOT(checkMousePosition()));
    connect(this, SIGNAL(destroyed(QObject*)), m_closeCheckTimer, SLOT(stop()));
}

void HoverMenuToolButton::enterEvent(QEvent *event)
{
    m_delayTimer->start();
    QToolButton::enterEvent(event);
}

void HoverMenuToolButton::leaveEvent(QEvent *event)
{
    // Stop showing menu if still waiting
    m_delayTimer->stop();
    QToolButton::leaveEvent(event);

    // Start checking if the mouse left button+menu area
    if (menu() && menu()->isVisible())
        m_closeCheckTimer->start();
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

    QPoint globalPos = QCursor::pos();
    QRect buttonRect = QRect(mapToGlobal(QPoint(0, 0)), size());
    QRect menuRect(m_menuPointer->pos(), m_menuPointer->size());
    QRect totalRect = buttonRect.united(menuRect);

    if (!totalRect.contains(globalPos))
    {
        m_menuPointer->close();
        m_closeCheckTimer->stop();
        m_menuPointer.clear();
    }
}

void HoverMenuToolButton::showMenuOnHover()
{
    if (!menu())
        return;

    m_menuPointer = menu();
    showMenu();
    m_closeCheckTimer->start();
}
