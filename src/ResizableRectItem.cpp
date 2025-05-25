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

    if (change == ItemSceneChange && scene() || change == ItemPositionChange || change == ItemTransformChange)
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

    QRectF r = rect();

    QPointF tlScene = mapToScene(r.topLeft());
    QPointF brScene = mapToScene(r.bottomRight());

    if (handleIndex == TopLeft)
    {
        tlScene = scenePos;
    }
    else if (handleIndex == BottomRight)
    {
        brScene = scenePos;
    }
    else if (handleIndex == TopRight)
    {
        tlScene.setY(brScene.y());
        brScene.setX(scenePos.x());
        tlScene.setX(scenePos.x() - r.width());
        brScene.setY(scenePos.y());
    }
    else if (handleIndex == BottomLeft)
    {
        tlScene.setX(scenePos.x());
        brScene.setY(scenePos.y());
    }

    prepareGeometryChange();
    QRectF newRect = QRectF(mapFromScene(tlScene), mapFromScene(brScene)).normalized();
    setRect(newRect);

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
    QRectF r = rect();

    static constexpr QPointF factors[HandleCount] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

    for (int i = 0; i < HandleCount; ++i)
    {
        QPointF localPt  = r.topLeft() + QPointF(r.width() * factors[i].x(), r.height() * factors[i].y());
        QPointF scenePt  = mapToScene(localPt);
        QPointF parentPt = mapFromScene(scenePt) - QPointF(4, 4);

        QSignalBlocker blocker(m_handles[i]);
        m_handles[i]->setPos(parentPt);
    }
}