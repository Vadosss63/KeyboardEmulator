#include "DiodeItem.h"

#include <QAction>
#include <QCursor>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

DiodeItem::DiodeItem(qreal x, qreal y) : DiodeItem(LedDef{x, y}) {}

DiodeItem::DiodeItem(const LedDef& def, QGraphicsItem* parent)
    : ResizableRectItem(def.rect.x(), def.rect.y(), def.rect.width(), def.rect.height(), parent)
    , m_pin1(def.pin1)
    , m_pin2(def.pin2)
{
    setColor(QColor(def.color));
    setCircularShape(def.isCircular);

    setPin1(def.pin1);
    setPin2(def.pin2);
}

LedDef DiodeItem::getDefinition() const
{
    LedDef def;
    def.rect       = rectItem();
    def.color      = color().name();
    def.isCircular = isCircular();
    def.pin1       = m_pin1;
    def.pin2       = m_pin2;
    return def;
}

void DiodeItem::setPin1(uint8_t pin)
{
    m_pin1 = pin;
    updateTextInfo();
}

void DiodeItem::setPin2(uint8_t pin)
{
    m_pin2 = pin;
    updateTextInfo();
}

ResizableRectItem* DiodeItem::clone() const
{
    return new DiodeItem(getDefinition());
}

void DiodeItem::onStatusUpdate(uint8_t pin, bool isOn)
{
    if (pin != m_pin1)
    {
        return;
    }

    setActive(isOn);

    updateAppearance();
}

void DiodeItem::extendDerivedContextMenu(QMenu& menu)
{
    if (isModifyMod())
    {
        addConfigMenu(menu);
    }

    QAction* pinAct = menu.addAction(tr("Анод: %1").arg(m_pin1));
    pinAct->setEnabled(false);
    QAction* invAct = menu.addAction(tr("Катод: %1").arg(m_pin2));
    invAct->setEnabled(false);
}

void DiodeItem::setupDeleteItemAction(QAction* deleteAction)
{
    connect(deleteAction, &QAction::triggered, this, [this] { emit removeDiode(this); });
}

void DiodeItem::addConfigMenu(QMenu& menu)
{
    QMenu* pin1Menu = menu.addMenu(tr("Установить Анод"));
    QMenu* pin2Menu = menu.addMenu(tr("Установить Катод"));

    for (uint8_t i = 1; i <= 15; ++i)
    {
        QAction* pin1Act = pin1Menu->addAction(QString::number(i));
        connect(pin1Act, &QAction::triggered, this, [this, i]() { setPin1(i); });

        QAction* pin2Act = pin2Menu->addAction(QString::number(i));
        connect(pin2Act, &QAction::triggered, this, [this, i]() { setPin2(i); });
    }
    menu.addSeparator();
}

void DiodeItem::updateTextInfo()
{
    const QString info = QString("А:%1\nК:%2").arg(m_pin1).arg(m_pin2);
    setInfoText(info);
}
