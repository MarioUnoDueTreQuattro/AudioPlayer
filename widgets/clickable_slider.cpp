#include "clickable_slider.h"

ClickableSlider::ClickableSlider(QWidget *parent) : QSlider(parent)
{
}

void ClickableSlider::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // Calculate new value based on click position
        double ratio = event->pos().x() / static_cast<double>(width());
        int newValue = minimum() + ratio * (maximum() - minimum());
        setValue(newValue);
        emit sliderMoved(newValue);
        emit sliderReleased(); // optional, to trigger immediate seek
    }
    QSlider::mousePressEvent(event);
}
