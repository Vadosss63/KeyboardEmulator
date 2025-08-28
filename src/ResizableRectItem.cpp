#include "ResizableRectItem.h"

#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QSignalBlocker>

#include "ResizeHandle.h"

ResizableRectItem::ResizableRectItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent)
    : QObject(), QGraphicsRectItem(x, y, w, h, parent)
{
    initHandles();
    infoItem = new QGraphicsTextItem(this);
    infoItem->setPlainText("Info:");
    infoItem->setDefaultTextColor(Qt::red);
    infoItem->setFont(QFont("Arial", 14));
    infoItem->setVisible(false);
}

void ResizableRectItem::setResizable(bool on)
{
    m_resizable = on;

    if (on)
    {
        setFlags(ItemIsMovable | ItemSendsGeometryChanges);
    }
    else
    {
        setFlags(ItemSendsGeometryChanges);
    }

    for (auto* h : m_handles)
    {
        h->setVisible(on);
    }

    infoItem->setVisible(on);
    updateHandles();
}

bool ResizableRectItem::isModifyMod() const
{
    return m_resizable;
}

QRectF ResizableRectItem::rectItem() const
{
    QRectF rScene = mapRectToScene(rect());
    return rScene;
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

void ResizableRectItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;

    if (isModifyMod())
    {
        addDeleteItemAction(menu);
    }

    extendDerivedContextMenu(menu);
    event->accept();

    menu.exec(event->screenPos());
}

void ResizableRectItem::addDeleteItemAction(QMenu& menu)
{
    QAction* deleteAction = menu.addAction("Удалить");
    setupDeleteItemAction(deleteAction);
}

void ResizableRectItem::setupDeleteItemAction(QAction* /*deleteAction*/) {}

void ResizableRectItem::extendDerivedContextMenu(QMenu& /*menu*/) {}

void ResizableRectItem::handleMoved(int handleIndex, const QPointF& scenePos)
{
    if (!m_resizable)
    {
        return;
    }

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
        // h->setParentItem(this);
        connect(h, &ResizeHandle::moved, this, [this](int idx, const QPointF& p) { handleMoved(idx, p); });
        m_handles[i] = h;
        h->setVisible(m_resizable);
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

    const QPointF infoPos = r.topRight() + QPointF{10, 10};
    infoItem->setPos(infoPos);
}