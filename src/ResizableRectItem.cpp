#include "ResizableRectItem.h"

#include <QSignalBlocker>

#include "ResizeHandle.h"

ResizableRectItem::ResizableRectItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent)
    : QObject(), QGraphicsRectItem(x, y, w, h, parent)
{
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    initHandles();
}

QVariant ResizableRectItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    QVariant v = QGraphicsRectItem::itemChange(change, value);
    if (!scene())
    {
        return v;
    }

    if (change == ItemSceneChange || change == ItemPositionChange || change == ItemTransformChange)
    {
        updateHandles();
    }

    return v;
}

void ResizableRectItem::handleMoved(int handleIndex, const QPointF& scenePos)
{
    if (handleIndex < 0 || handleIndex >= HandleCount || !scene())
    {
        return;
    }

    const QRectF& r = rect();

    QPointF tlScene = r.topLeft();
    QPointF brScene = r.bottomRight();

    QRectF newRect = QRectF(tlScene, brScene);

    if (handleIndex == TopLeft)
    {
        newRect.setTopLeft(scenePos);
    }
    else if (handleIndex == BottomRight)
    {
        newRect.setBottomRight(scenePos);
    }
    else if (handleIndex == TopRight)
    {
        newRect.setTopRight(scenePos);
    }
    else if (handleIndex == BottomLeft)
    {
        newRect.setBottomLeft(scenePos);
    }

    prepareGeometryChange();

    setRect(newRect.normalized());

    updateHandles();
}

void ResizableRectItem::initHandles()
{
    for (int i = 0; i < HandleCount; i++)
    {
        auto* h = new ResizeHandle(this, i);
        h->setParentItem(this);
        connect(h, &ResizeHandle::moved, this, [this](int idx, const QPointF& p) { handleMoved(idx, p); });
        m_handles[i] = h;
    }
}

void ResizableRectItem::updateHandles()
{
    const QRectF& r = rect();

    static constexpr QPointF factors[HandleCount] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

    for (int i = 0; i < HandleCount; ++i)
    {
        QPointF newHandlerPos = r.topLeft() + QPointF(r.width() * factors[i].x(), r.height() * factors[i].y());

        QSignalBlocker blocker(m_handles[i]);
        m_handles[i]->setPos(newHandlerPos);
    }
}