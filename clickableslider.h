#ifndef CLICKABLESLIDER_H
#define CLICKABLESLIDER_H

#include <QSlider>
#include <QMouseEvent>

class ClickableSlider : public QSlider
{
    Q_OBJECT
public:
    explicit ClickableSlider(QWidget *parent = nullptr); /*: QSlider(parent) {}*/

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // CLICKABLESLIDER_H
