#pragma once
#include <QBrush>
#include <QPen>

#include "ResizableRectItem.h"

class ButtonItem : public ResizableRectItem
{
public:
    ButtonItem(qreal x, qreal y, qreal w = 80, qreal h = 80, QGraphicsItem* parent = nullptr)
        : ResizableRectItem(x, y, w, h, parent)
    {
        setPen(QPen(Qt::blue, 2));
        setBrush(Qt::transparent);
    }
};
