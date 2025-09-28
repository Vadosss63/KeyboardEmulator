#include "ButtonItem.h"

#include <QAction>
#include <QGraphicsSceneContextMenuEvent>

ButtonItem::ButtonItem(const ItemDef& def, QGraphicsItem* parent) : AbstractItem(def, parent)
{
    updateTextInfo();
}

ButtonItem::ButtonItem(qreal x, qreal y) : ButtonItem(ButtonDef{x, y}, nullptr) {}

ResizableRectItem* ButtonItem::clone() const
{
    return new ButtonItem(getDefinition());
}

void ButtonItem::extendDerivedContextMenu(QMenu& menu)
{
    if (isShowExtendedMenu())
    {
        addPinConfigMenu(menu);
    }

    QAction* pin1Act = menu.addAction(tr("Пин1: %1").arg(getPin1()));
    pin1Act->setEnabled(false);
    QAction* pin2Act = menu.addAction(tr("Пин2: %1").arg(getPin2()));
    pin2Act->setEnabled(false);
}

void ButtonItem::addPinConfigMenu(QMenu& menu)
{
    QMenu* pin1Menu = menu.addMenu(tr("Установить пин 1"));
    QMenu* pin2Menu = menu.addMenu(tr("Установить пин 2"));

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
    const QString info = QString("P1:%1\nP2:%2").arg(getPin1()).arg(getPin2());
    setInfoText(info);
}
