#include "AbstractItem.h"

#include <QAction>
#include <QDebug>
#include <QGraphicsSceneContextMenuEvent>

namespace
{

bool isLeftButtonPressed(QGraphicsSceneMouseEvent* event)
{
    if (!event)
    {
        return false;
    }

    return event->button() == Qt::LeftButton;
}

bool isCtrlButtonPressed(QGraphicsSceneMouseEvent* event)
{
    if (!event)
    {
        return false;
    }

    return event->modifiers() & Qt::ControlModifier;
}

}

AbstractItem::AbstractItem(const ItemDef& def, QGraphicsItem* parent)
    : ResizableRectItem(def.rect.x(), def.rect.y(), def.rect.width(), def.rect.height(), parent)
    , m_pin1(def.p1)
    , m_pin2(def.p2)
{
    setColor(QColor(def.color));
    setCircularShape(def.isCircular);
}

ItemDef AbstractItem::getDefinition() const
{
    ItemDef def{};
    def.rect       = rectItem();
    def.color      = color().name();
    def.isCircular = isCircular();
    def.p1         = m_pin1;
    def.p2         = m_pin2;
    return def;
}

void AbstractItem::setPin1(uint8_t pin)
{
    if (m_pin1 == pin)
    {
        return;
    }
    m_pin1 = pin;
    updateTextInfo();
    emit pinsChanged(this);
}

void AbstractItem::setPin2(uint8_t pin)
{
    if (m_pin2 == pin)
    {
        return;
    }

    m_pin2 = pin;
    updateTextInfo();
    emit pinsChanged(this);
}

uint8_t AbstractItem::getPin1() const
{
    return m_pin1;
}

uint8_t AbstractItem::getPin2() const
{
    return m_pin2;
}
void AbstractItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_clickable || isActive())
    {
        event->accept();
        return;
    }

    if (!isLeftButtonPressed(event))
    {
        ResizableRectItem::mousePressEvent(event);
        return;
    }

    setActive(true);
    updateAppearance();
    emit buttonPressed({m_pin1, m_pin2});

    if (isCtrlButtonPressed(event))
    {
        ResizableRectItem::mousePressEvent(event);
        return;
    }

    event->accept();
}

void AbstractItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    ResizableRectItem::mouseReleaseEvent(event);

    if (!m_clickable || !isLeftButtonPressed(event))
    {
        return;
    }

    if (isActive())
    {
        setActive(false);
        updateAppearance();
        emit buttonReleased({m_pin1, m_pin2});
    }

    event->accept();
}

void AbstractItem::setupDeleteItemAction(QAction* deleteAction)
{
    connect(deleteAction, &QAction::triggered, this, [this] { emit removeItem(this); });
}

void AbstractItem::setClickable(bool isClickable)
{
    m_clickable = isClickable;
}