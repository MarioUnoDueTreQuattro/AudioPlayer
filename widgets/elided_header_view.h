#ifndef ELIDEDHEADERVIEW_H
#define ELIDEDHEADERVIEW_H

#pragma once

#include <QHeaderView>
#include <QToolTip>

class ElidedHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit ElidedHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
     void leaveEvent(QEvent *event) override;
};
#endif // ELIDEDHEADERVIEW_H
