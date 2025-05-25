#pragma once
#include <QBrush>
#include <QPen>

#include "ResizableRectItem.h"

class ButtonItem : public ResizableRectItem
{
public:
    ButtonItem(qreal x, qreal y, qreal w = 80, qreal h = 80, QGraphicsItem* parent = nullptr);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void updateAppearance();

    bool   m_active = false;
    QBrush m_normalBrush;
    QBrush m_activeBrush;
};
