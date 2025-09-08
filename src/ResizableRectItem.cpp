#include "ResizableRectItem.h"

#include <QBrush>
#include <QColorDialog>
#include <QFont>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainterPath>
#include <QPen>
#include <QSignalBlocker>
#include <QStyleOptionGraphicsItem>

#include "ResizeHandle.h"

ResizableRectItem::ResizableRectItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent)
    : QObject(), QGraphicsRectItem(x, y, w, h, parent)
{
    initHandles();
    infoItem = new QGraphicsTextItem(this);
    infoItem->setPlainText("");
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

bool ResizableRectItem::isCircular() const
{
    return m_circular;
}

void ResizableRectItem::setCircularShape(bool circular)
{
    m_circular = circular;
    updateAppearance();
}

bool ResizableRectItem::isActive() const
{
    return m_active;
}

void ResizableRectItem::setActive(bool active)
{
    m_active = active;
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

void ResizableRectItem::setInfoText(const QString& text)
{
    infoItem->setPlainText(text);
}

void ResizableRectItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);
    painter->setPen(pen());
    if (option->state & QStyle::State_Selected)
    {
        QPen selPen = pen();
        selPen.setStyle(Qt::DashLine);
        painter->setPen(selPen);
    }
    painter->setBrush(brush());

    if (isCircular())
    {
        painter->drawEllipse(rect());
        return;
    }

    painter->drawRect(rect());
}

QPainterPath ResizableRectItem::shape() const
{
    QPainterPath path;
    if (isCircular())
    {
        path.addEllipse(rect());
    }
    else
    {
        path.addRect(rect());
    }
    return path;
}

void ResizableRectItem::setColor(const QColor& color)
{
    m_color       = color;
    m_normalBrush = Qt::transparent;
    m_activeBrush = QColor(color.red(), color.green(), color.blue(), 100);

    QPen pen(color, 2);

    setPen(pen);
    setBrush(m_normalBrush);
    update();
}

QColor ResizableRectItem::color() const
{
    return m_color;
}

void ResizableRectItem::makeRectShape()
{
    const QRectF& r = rect();
    if (r.width() > r.height())
    {
        updateRect(QRectF(r.x(), r.y(), r.height(), r.height()));
        return;
    }

    updateRect(QRectF(r.x(), r.y(), r.width(), r.width()));
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

void ResizableRectItem::changeColor()
{
    QColor newColor = QColorDialog::getColor(color(), nullptr, tr("Выберите цвет"));
    if (!newColor.isValid())
    {
        return;
    }
    setColor(newColor);
}

void ResizableRectItem::addDeleteItemAction(QMenu& menu)
{
    QAction* deleteAction = menu.addAction("Удалить");
    setupDeleteItemAction(deleteAction);
    QAction* colorAction = menu.addAction("Цвет");
    connect(colorAction, &QAction::triggered, this, &ResizableRectItem::changeColor);
    QAction* shapeAction = menu.addAction(tr("Выровнять"));
    connect(shapeAction, &QAction::triggered, this, &ResizableRectItem::makeRectShape);

    QMenu*   shapeMenu  = menu.addMenu(tr("Форма"));
    QAction* rectAction = shapeMenu->addAction(tr("Квадрат"));
    connect(rectAction, &QAction::triggered, [this]() { setCircularShape(false); });
    QAction* ellipseAction = shapeMenu->addAction(tr("Круг"));
    connect(ellipseAction, &QAction::triggered, [this]() { setCircularShape(true); });

    QAction* copyAction = menu.addAction(tr("Копировать"));
    connect(copyAction, &QAction::triggered, [this]() { emit itemCopied(this); });
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

    updateRect(newRect);
}

void ResizableRectItem::initHandles()
{
    for (int i = 0; i < HandleCount; i++)
    {
        auto* h = new ResizeHandle(this, i);
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

void ResizableRectItem::updateRect(const QRectF& rect)
{
    prepareGeometryChange();
    setRect(rect);
    updateHandles();
}

void ResizableRectItem::updateAppearance()
{
    setBrush(isActive() ? m_activeBrush : m_normalBrush);
    update();
}