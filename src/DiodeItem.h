#pragma once

#include <QBrush>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

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

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
    {
        Q_UNUSED(widget);
        painter->setPen(pen());

        if (option->state & QStyle::State_Selected)
        {
            QPen newPen = pen();
            newPen.setStyle(Qt::DashLine);
            painter->setPen(newPen);
        }

        painter->setBrush(brush());
        painter->drawEllipse(rect());
    }

    QPainterPath shape() const override
    {
        QPainterPath path;
        path.addEllipse(rect());
        return path;
    }
};
