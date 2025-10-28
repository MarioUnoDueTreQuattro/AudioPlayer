#ifndef FADINGSLIDER_H
#define FADINGSLIDER_H

#include <QSlider>
#include <QPropertyAnimation>

class FadingSlider : public QSlider
{
    Q_OBJECT
    Q_PROPERTY(qreal indicatorOpacity READ indicatorOpacity WRITE setIndicatorOpacity)

public:
    explicit FadingSlider(QWidget *parent = nullptr);

    qreal indicatorOpacity() const;
    void setIndicatorOpacity(qreal value);

    bool animationIsEnabled() const;
    void setAnimationEnabled(bool bEnableAnimation);
void updateIndicatorSettings();
public slots:
    void hideFadeProgress();
    void startFadeIn();
    void startFadeOut();
    void setFadeProgress(int percent);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int fadeProgress; // 0–100
    qreal opacity;
    QPropertyAnimation *fadeAnim;
    bool m_bAnimationEnabled;
    bool m_bIndicator;
    int m_iRadius;
    int m_iTransparency;
    double m_dTransparency;
    QColor m_Color;
};

#endif // FADINGSLIDER_H
