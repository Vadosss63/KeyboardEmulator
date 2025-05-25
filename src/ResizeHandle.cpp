#include "ResizeHandle.h"

#include <QPen>

#include "ResizableRectItem.h"

ResizeHandle::ResizeHandle(ResizableRectItem* parent, int idx)
    : QObject(), QGraphicsRectItem(-4, -4, 8, 8, parent), m_parent(parent), m_index(idx)
{
    setBrush(Qt::white);
    setPen(QPen(Qt::black));
    setFlags(ItemIsMovable | ItemSendsGeometryChanges | ItemIgnoresTransformations);
}

QVariant ResizeHandle::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionChange)
    {
        QPointF newPos = mapToScene(value.toPointF());
        emit    moved(m_index, newPos);
    }
    return QGraphicsRectItem::itemChange(change, value);
}