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
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            // Calculate new value based on click position
            double ratio = event->pos().x() / static_cast<double>(width());
            int newValue = minimum() + ratio * (maximum() - minimum());
            setValue(newValue);
            emit sliderMoved(newValue);
            emit sliderReleased(); // optional, to trigger immediate seek
        }
        QSlider::mousePressEvent(event);
    }
};

#endif // CLICKABLESLIDER_H
