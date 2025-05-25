#pragma once
#include <QBrush>
#include <QPen>

#include "ResizableRectItem.h"

class DiodeItem : public ResizableRectItem
{
public:
    DiodeItem(qreal x, qreal y, qreal w = 80, qreal h = 80, QGraphicsItem* parent = nullptr)
        : ResizableRectItem(x, y, w, h, parent)
    {
        setPen(QPen(Qt::red, 2));
        setBrush(QBrush(Qt::yellow, Qt::SolidPattern));
    }
};
