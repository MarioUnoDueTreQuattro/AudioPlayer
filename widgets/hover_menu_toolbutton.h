#ifndef HOVERMENUTOOLBUTTON_H
#define HOVERMENUTOOLBUTTON_H

#include <QToolButton>
#include <QTimer>
#include <QEvent>
#include <QPointer>

/**
 * @brief A QToolButton that automatically shows its menu when hovered
 *        and hides it when the cursor leaves both the button and the menu.
 */
class HoverMenuToolButton : public QToolButton
{
    Q_OBJECT

public:
    explicit HoverMenuToolButton(QWidget *parent = nullptr);

protected:
    // Called when mouse enters the button
    void enterEvent(QEvent *event) override;

    // Called when mouse leaves the button
    void leaveEvent(QEvent *event) override;

private slots:
    // Show the menu after a delay
    void showMenuOnHover();

    // Periodically check cursor position to close the menu
    void checkMousePosition();
 void release();
private:
    QTimer *m_delayTimer;          // Delay before showing menu
    QTimer *m_closeCheckTimer;     // Periodic check for mouse position
    QPointer<QMenu> m_menuPointer; // Safe pointer to the active menu
};

#endif // HOVERMENUTOOLBUTTON_H
