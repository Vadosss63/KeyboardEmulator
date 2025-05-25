#include "ButtonItem.h"

ButtonItem::ButtonItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent)
    : ResizableRectItem(x, y, w, h, parent), m_normalBrush(Qt::transparent), m_activeBrush(QColor(0, 0, 0, 80))
{
    QPen pen(Qt::blue, 2);
    pen.setStyle(Qt::DashLine);
    setPen(pen);
    setBrush(m_normalBrush);
}

void ButtonItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    m_active = !m_active;
    updateAppearance();

    ResizableRectItem::mousePressEvent(event);
}

void ButtonItem::updateAppearance()
{
    if (m_active)
    {
        setBrush(m_activeBrush);
    }
    else
    {
        setBrush(m_normalBrush);
    }
    update();
}