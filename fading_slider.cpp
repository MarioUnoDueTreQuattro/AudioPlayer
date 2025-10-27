#include "fading_slider.h"
#include <QPainter>
#include <QStyleOptionSlider>
#include <QDebug>

FadingSlider::FadingSlider(QWidget *parent)
    : QSlider(Qt::Horizontal, parent), fadeProgress(-1), opacity(0.0), m_bAnimationEnabled(true)
{
    setMinimum(0);
    setMaximum(100);
    fadeAnim = new QPropertyAnimation(this, "indicatorOpacity");
    fadeAnim->setDuration(400); // 400ms fade
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
}

void FadingSlider::setFadeProgress(int percent)
{
    fadeProgress = qBound(0, percent, 100);
    update();
}

qreal FadingSlider::indicatorOpacity() const
{
    return opacity;
}

void FadingSlider::setIndicatorOpacity(qreal value)
{
    opacity = value;
    update();
}

void FadingSlider::startFadeIn()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (m_bAnimationEnabled)
    {
        fadeAnim->stop();
        fadeAnim->setDirection(QAbstractAnimation::Forward);
        fadeAnim->start();
    }
}

void FadingSlider::startFadeOut()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (m_bAnimationEnabled)
    {
        fadeAnim->stop();
        fadeAnim->setDirection(QAbstractAnimation::Backward);
        fadeAnim->start();
    }
}

void FadingSlider::hideFadeProgress()
{
    fadeProgress = -1;
    update();
}
//void FadingSlider::paintEvent(QPaintEvent *event)
//{
// QSlider::paintEvent(event);

// if (fadeProgress < 0)
// return; // indicatore nascosto

// QPainter painter(this);
// painter.setRenderHint(QPainter::Antialiasing);

// QStyleOptionSlider opt;
// initStyleOption(&opt);

// QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
// int x = grooveRect.left() + (grooveRect.width() * fadeProgress) / 100;

// QRect indicatorRect(x - 2, grooveRect.top(), 4, grooveRect.height());
// painter.setBrush(Qt::red);
// painter.setPen(Qt::NoPen);
// painter.drawRect(indicatorRect);
//}
//void FadingSlider::paintEvent(QPaintEvent *event)
//{
// QSlider::paintEvent(event);

// if (fadeProgress < 0)
// return; // indicator hidden

// QPainter painter(this);
// painter.setRenderHint(QPainter::Antialiasing);

// QStyleOptionSlider opt;
// initStyleOption(&opt);

// QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);

//    // Compute pixel position of fade progress within groove
// int grooveLeft = grooveRect.left();
// int grooveWidth = grooveRect.width();
// int x = grooveLeft + (grooveWidth * fadeProgress) / 100;

//    // Draw vertical indicator at fade position
// QRect indicatorRect(x - 2, grooveRect.top(), 4, grooveRect.height());
// painter.setBrush(Qt::red);
// painter.setPen(Qt::NoPen);
// painter.drawRect(indicatorRect);
//}
//void FadingSlider::paintEvent(QPaintEvent *event)
//{
// QSlider::paintEvent(event);

// if (fadeProgress < 0)
// return; // indicator hidden

// QPainter painter(this);
// painter.setRenderHint(QPainter::Antialiasing);

// QStyleOptionSlider opt;
// initStyleOption(&opt);

// QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
// int grooveLeft = grooveRect.left();
// int grooveWidth = grooveRect.width();
// int x = grooveLeft + (grooveWidth * fadeProgress) / 100;

//    // Smaller, semitransparent, rounded indicator
// int indicatorWidth = 4;
// int indicatorHeight = grooveRect.height() / 2;
// QRectF indicatorRect(x - indicatorWidth / 2.0,
// grooveRect.center().y() - indicatorHeight / 2.0,
// indicatorWidth,
// indicatorHeight);

// QColor fadeColor(0, 0, 255);
// fadeColor.setAlpha(128);    // semitransparent

// painter.setBrush(fadeColor);
// painter.setPen(Qt::NoPen);
// painter.drawRoundedRect(indicatorRect, 2, 2);
//}
//void FadingSlider::paintEvent(QPaintEvent *event)
//{
// QSlider::paintEvent(event);

// if (fadeProgress < 0)
// return; // indicator hidden

// QPainter painter(this);
// painter.setRenderHint(QPainter::Antialiasing);

// QStyleOptionSlider opt;
// initStyleOption(&opt);

//    // Simula posizione del handle per il valore di fade
// QStyleOptionSlider fadeOpt = opt;
// int sliderMin = minimum();
// int sliderMax = maximum();
// int valueForFade = sliderMin + (fadeProgress * (sliderMax - sliderMin)) / 100;
// fadeOpt.sliderValue = valueForFade;

// QRect fadeHandleRect = style()->subControlRect(QStyle::CC_Slider, &fadeOpt, QStyle::SC_SliderHandle, this);
// int centerX = fadeHandleRect.center().x();

//    // Indicatore estetico: blu, semitrasparente, arrotondato
// int indicatorWidth = 4;
// int indicatorHeight = fadeHandleRect.height() / 2;
// QRectF indicatorRect(centerX - indicatorWidth / 2.0,
// fadeHandleRect.center().y() - indicatorHeight / 2.0,
// indicatorWidth,
// indicatorHeight);

// QColor fadeColor(0, 120, 255); // blu
// fadeColor.setAlpha(100);       // semitrasparente

// painter.setBrush(fadeColor);
// painter.setPen(Qt::NoPen);
// painter.drawRoundedRect(indicatorRect, 2, 2);
//}
//void FadingSlider::paintEvent(QPaintEvent *event)
//{
// QSlider::paintEvent(event);

// if (fadeProgress < 0)
// return; // indicator hidden

// QPainter painter(this);
// painter.setRenderHint(QPainter::Antialiasing);

// QStyleOptionSlider opt;
// initStyleOption(&opt);

// QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);

//    // Get style metrics
// int sliderMin = minimum();
// int sliderMax = maximum();
// int sliderRange = sliderMax - sliderMin;

//    // Compute pixel position of fadeProgress manually
// double ratio = static_cast<double>(fadeProgress) / 100.0;
// int x = grooveRect.left() + static_cast<int>(ratio * grooveRect.width());

//    // Adjust for style margins if needed
// int indicatorWidth = 4;
// int indicatorHeight = grooveRect.height() / 2;
// QRectF indicatorRect(x - indicatorWidth / 2.0,
// grooveRect.center().y() - indicatorHeight / 2.0,
// indicatorWidth,
// indicatorHeight);

// QColor fadeColor(0, 120, 255); // blue
// fadeColor.setAlpha(100);       // semitransparent

// painter.setBrush(fadeColor);
// painter.setPen(Qt::NoPen);
// painter.drawRoundedRect(indicatorRect, 2, 2);
//}
//void FadingSlider::paintEvent(QPaintEvent *event)
//{
// QSlider::paintEvent(event);

// if (fadeProgress < 0)
// return; // indicator hidden

// QPainter painter(this);
// painter.setRenderHint(QPainter::Antialiasing);

// QStyleOptionSlider opt;
// initStyleOption(&opt);

//    // Simulate a virtual handle at fadeProgress
// QStyleOptionSlider fadeOpt = opt;
// int sliderMin = minimum();
// int sliderMax = maximum();
// int valueForFade = sliderMin + (fadeProgress * (sliderMax - sliderMin)) / 100;
// fadeOpt.sliderValue = valueForFade;
// fadeOpt.sliderPosition = valueForFade;

// QRect fadeHandleRect = style()->subControlRect(QStyle::CC_Slider, &fadeOpt, QStyle::SC_SliderHandle, this);
// int centerX = fadeHandleRect.center().x();

//    // Draw indicator centered on virtual handle
// int indicatorWidth = 4;
// int indicatorHeight = fadeHandleRect.height() / 2;
// QRectF indicatorRect(centerX - indicatorWidth / 2.0,
// fadeHandleRect.center().y() - indicatorHeight / 2.0,
// indicatorWidth,
// indicatorHeight);

// QColor fadeColor(0, 120, 255); // blue
// fadeColor.setAlpha(255);       // semitransparent

// painter.setBrush(fadeColor);
// painter.setPen(Qt::NoPen);
// painter.drawRoundedRect(indicatorRect, 2, 2);
//}
//void FadingSlider::paintEvent(QPaintEvent *event)
//{
// QSlider::paintEvent(event);

// if (fadeProgress < 0)
// return; // indicator hidden

// QPainter painter(this);
// painter.setRenderHint(QPainter::Antialiasing);

// QStyleOptionSlider opt;
// initStyleOption(&opt);

//    // Simula posizione del handle virtuale per fadeProgress
// QStyleOptionSlider fadeOpt = opt;
// int sliderMin = minimum();
// int sliderMax = maximum();
// int valueForFade = sliderMin + (fadeProgress * (sliderMax - sliderMin)) / 100;
// fadeOpt.sliderValue = valueForFade;
// fadeOpt.sliderPosition = valueForFade;

// QRect fadeHandleRect = style()->subControlRect(QStyle::CC_Slider, &fadeOpt, QStyle::SC_SliderHandle, this);
// QPoint center = fadeHandleRect.center();

//    // Cerchio semitrasparente blu
// int radius = 5;
// QRectF circleRect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);

// QColor fadeColor(0, 120, 255); // blu
// fadeColor.setAlpha(128);       // semitrasparente

// painter.setBrush(fadeColor);
// painter.setPen(Qt::NoPen);
// painter.drawEllipse(circleRect);
//}
void FadingSlider::paintEvent(QPaintEvent *event)
{
    QSlider::paintEvent(event);
    if (fadeProgress < 0 || opacity <= 0.01)
        return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QStyleOptionSlider fadeOpt = opt;
    int sliderMin = minimum();
    int sliderMax = maximum();
    int valueForFade = sliderMin + (fadeProgress * (sliderMax - sliderMin)) / 100;
    fadeOpt.sliderValue = valueForFade;
    fadeOpt.sliderPosition = valueForFade;
    QRect fadeHandleRect = style()->subControlRect(QStyle::CC_Slider, &fadeOpt, QStyle::SC_SliderHandle, this);
    QPoint center = fadeHandleRect.center();
    int radius = 5;
    QRectF circleRect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);
    QColor fadeColor(0, 120, 255);
    fadeColor.setAlphaF(opacity); // animatable
    painter.setBrush(fadeColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(circleRect);
}

bool FadingSlider::animationIsEnabled() const
{
    return m_bAnimationEnabled;
}

void FadingSlider::setAnimationEnabled(bool bEnableAnimation)
{
    m_bAnimationEnabled = bEnableAnimation;
}
