#include "DiodeItem.h"

DiodeItem::DiodeItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent) : ResizableRectItem(x, y, w, h, parent)
{
    QColor offColor(Qt::gray);
    offColor.setAlphaF(0.3);
    m_offBrush = QBrush(offColor);

    QColor onColor(Qt::green);
    onColor.setAlphaF(0.3);
    m_onBrush = QBrush(onColor);

    QPen pen(m_offColor, 2);
    setPen(pen);

    setBrush(m_offBrush);
}

void DiodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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

QPainterPath DiodeItem::shape() const
{
    QPainterPath path;
    path.addEllipse(rect());
    return path;
}

void DiodeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    m_active = !m_active;
    updateAppearance();

    ResizableRectItem::mousePressEvent(event);
}

void DiodeItem::updateAppearance()
{
    setBrush(m_active ? m_onBrush : m_offBrush);
    setPen(QPen(m_active ? m_onColor : m_offColor, 2));
    update();
}