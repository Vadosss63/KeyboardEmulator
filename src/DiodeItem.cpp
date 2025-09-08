#include "DiodeItem.h"

#include <QAction>
#include <QCursor>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

DiodeItem::DiodeItem(qreal x, qreal y) : DiodeItem(LedDef{x, y}) {}

DiodeItem::DiodeItem(const LedDef& def, QGraphicsItem* parent)
    : ResizableRectItem(def.rect.x(), def.rect.y(), def.rect.width(), def.rect.height(), parent)
    , m_pin(def.pin)
    , m_inverted(def.inverted)
{
    setColor(QColor(def.color));
    setCircularShape(def.isCircular);

    setPin(def.pin);
    setInverted(def.inverted);
}

LedDef DiodeItem::getDefinition() const
{
    LedDef def;
    def.rect       = rectItem();
    def.color      = color().name();
    def.isCircular = isCircular();
    def.pin        = m_pin;
    def.inverted   = m_inverted;
    return def;
}

void DiodeItem::setPin(uint8_t pin)
{
    m_pin = pin;
    updateTextInfo();
}

void DiodeItem::setInverted(bool inverted)
{
    m_inverted = inverted;
    updateTextInfo();
    onStatusUpdate(m_pin, false);
}

ResizableRectItem* DiodeItem::clone() const
{
    return new DiodeItem(getDefinition());
}

void DiodeItem::onStatusUpdate(uint8_t pin, bool isOn)
{
    if (pin != m_pin)
    {
        return;
    }

    bool actualState = m_inverted ? !isOn : isOn;

    setActive(actualState);

    updateAppearance();
}

void DiodeItem::extendDerivedContextMenu(QMenu& menu)
{
    if (isModifyMod())
    {
        addConfigMenu(menu);
    }

    QAction* pinAct = menu.addAction(tr("Пин: %1").arg(m_pin));
    pinAct->setEnabled(false);
    QAction* invAct = menu.addAction(tr("Инверсия: %1").arg(m_inverted));
    invAct->setEnabled(false);
}

void DiodeItem::setupDeleteItemAction(QAction* deleteAction)
{
    connect(deleteAction, &QAction::triggered, this, [this] { emit removeDiode(this); });
}

void DiodeItem::addConfigMenu(QMenu& menu)
{
    QMenu* pinMenu = menu.addMenu(tr("Установить пин"));
    QMenu* invMenu = menu.addMenu(tr("Инверсия"));

    for (uint8_t i = 1; i <= 15; ++i)
    {
        QAction* act = pinMenu->addAction(QString::number(i));
        connect(act, &QAction::triggered, this, [this, i]() { setPin(i); });
    }

    QAction* invOn  = invMenu->addAction(tr("None"));
    QAction* invOff = invMenu->addAction(tr("Inverted"));
    invOn->setData(QVariant(false));
    invOff->setData(QVariant(true));
    connect(invOn, &QAction::triggered, this, [this]() { setInverted(false); });
    connect(invOff, &QAction::triggered, this, [this]() { setInverted(true); });

    menu.addSeparator();
}

void DiodeItem::updateTextInfo()
{
    const QString info = QString("P:%1\nI:%2").arg(m_pin).arg(m_inverted);
    setInfoText(info);
}
