#include "ButtonItem.h"

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

ButtonItem::ButtonItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent, uint8_t pin1, uint8_t pin2)
    : ResizableRectItem(x, y, w, h, parent), m_normalBrush(Qt::transparent), m_activeBrush(QColor(0, 0, 0, 80))
{
    QPen pen(Qt::blue, 2);
    pen.setStyle(Qt::DashLine);
    setPen(pen);
    setBrush(m_normalBrush);
    setPin1(pin1);
    setPin2(pin2);
}

ButtonItem::ButtonItem(const ButtonDef& def, QGraphicsItem* parent)
    : ButtonItem(def.rect.x(), def.rect.y(), def.rect.width(), def.rect.height(), parent, def.p1, def.p2)
{
}

ButtonDef ButtonItem::getDefinition() const
{
    ButtonDef def;
    def.rect = rectItem();
    def.p1   = m_pin1;
    def.p2   = m_pin2;
    return def;
}

void ButtonItem::setPin1(uint8_t pin)
{
    m_pin1 = pin;
    updateTextInfo();
}

void ButtonItem::setPin2(uint8_t pin)
{
    m_pin2 = pin;
    updateTextInfo();
}

void ButtonItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_clickable || m_active)
    {
        event->accept();
        return;
    }

    if (!isLeftButtonPressed(event))
    {
        ResizableRectItem::mousePressEvent(event);
        return;
    }

    m_active = true;
    updateAppearance();
    emit buttonPressed(m_pin1, m_pin2);

    if (isCtrlButtonPressed(event))
    {
        ResizableRectItem::mousePressEvent(event);
        return;
    }

    event->accept();
}

void ButtonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    ResizableRectItem::mouseReleaseEvent(event);

    if (!m_clickable || !isLeftButtonPressed(event))
    {
        return;
    }

    if (m_active)
    {
        m_active = false;
        updateAppearance();
        emit buttonReleased(m_pin1, m_pin2);
    }

    event->accept();
}

void ButtonItem::onStatusUpdate(uint8_t pin1, uint8_t pin2, bool isPressed)
{
    if (pin1 == 0 && pin2 == 0)
    {
        // Reset all buttons
        if (m_active)
        {
            m_active = false;
            updateAppearance();
        }
        return;
    }

    if (m_pin1 != pin1 || m_pin2 != pin2)
    {
        return;
    }

    m_active = isPressed;
    updateAppearance();
}

void ButtonItem::extendDerivedContextMenu(QMenu& menu)
{
    if (isModifyMod())
    {
        addPinConfigMenu(menu);
    }

    QAction* pin1Act = menu.addAction(tr("Pin1: %1").arg(m_pin1));
    pin1Act->setEnabled(false);
    QAction* pin2Act = menu.addAction(tr("Pin2: %1").arg(m_pin2));
    pin2Act->setEnabled(false);
}

void ButtonItem::setupDeleteItemAction(QAction* deleteAction)
{
    connect(deleteAction, &QAction::triggered, this, [this] { emit removeButton(this); });
}

void ButtonItem::addPinConfigMenu(QMenu& menu)
{
    QMenu* pin1Menu = menu.addMenu(tr("Set Pin 1"));
    QMenu* pin2Menu = menu.addMenu(tr("Set Pin 2"));

    for (uint8_t i = 1; i <= 15; ++i)
    {
        QAction* act1 = pin1Menu->addAction(QString::number(i));
        connect(act1, &QAction::triggered, this, [this, i]() { setPin1(i); });
        QAction* act2 = pin2Menu->addAction(QString::number(i));
        connect(act2, &QAction::triggered, this, [this, i]() { setPin2(i); });
    }

    menu.addSeparator();
}

void ButtonItem::updateTextInfo()
{
    const QString info = QString("P1:%1\nP2:%2").arg(m_pin1).arg(m_pin2);
    setInfoText(info);
}

void ButtonItem::updateAppearance()
{
    setBrush(m_active ? m_activeBrush : m_normalBrush);
    update();
}

void ButtonItem::setClickable(bool isClickable)
{
    m_clickable = isClickable;
}
