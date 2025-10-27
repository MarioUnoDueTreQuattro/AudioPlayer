#ifndef FADINGSLIDER_H
#define FADINGSLIDER_H

#include <QSlider>

class FadingSlider : public QSlider
{
    Q_OBJECT

public:
    explicit FadingSlider(QWidget *parent = nullptr);

public slots:
    void setFadeProgress(int percent); // 0–100
    void hideFadeProgress();
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int fadeProgress; // 0–100
};

#endif // FADINGSLIDER_H
