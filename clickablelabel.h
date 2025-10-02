#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QToolTip>
#include <QMouseEvent>
#include <QTimer>
#include <QCursor>
#include <Qt> // For Qt::PointingHandCursor
#include <QtMath>  // for qAbs()

/**
 * @brief ClickableLabel is a QLabel subclass that emits a signal when clicked.
 * * It overrides the mousePressEvent() to detect left-clicks and emit the
 * custom 'clicked()' signal, making it much easier to use in slots.
 */
class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    // Constructor definition, takes optional parent widget and window flags
    explicit ClickableLabel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    QString getManagedTooltip() const;
    void setManagedTooltip(const QString &value);

private:
    QString sManagedTooltip;
    QTimer *m_timer;
    bool m_cursorHidden = false;
    QPoint m_lastMousePos;
signals:
    // Custom signal emitted when the label is clicked (typically by the left button)
    void clicked();

protected:
    /**
     * @brief Overrides the standard QWidget mouse press event handler.
     * * This method intercepts mouse clicks on the label area.
     * @param event The QMouseEvent object containing click details.
     */
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private slots:
    void onInactivityTimeout();
};
#endif // CLICKABLELABEL_H
