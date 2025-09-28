#include "DiodeItem.h"

#include <QAction>
#include <QCursor>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

DiodeItem::DiodeItem(const ItemDef& def, QGraphicsItem* parent) : AbstractItem(def, parent)
{
    updateTextInfo();
}

DiodeItem::DiodeItem(qreal x, qreal y) : DiodeItem(LedDef{x, y}, nullptr) {}

ResizableRectItem* DiodeItem::clone() const
{
    return new DiodeItem(getDefinition());
}

void DiodeItem::extendDerivedContextMenu(QMenu& menu)
{
    if (isShowExtendedMenu())
    {
        addConfigMenu(menu);
    }

    QAction* pinAct = menu.addAction(tr("Анод: %1").arg(getPin1()));
    pinAct->setEnabled(false);
    QAction* invAct = menu.addAction(tr("Катод: %1").arg(getPin2()));
    invAct->setEnabled(false);
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
    const QString info = QString("А:%1\nК:%2").arg(getPin1()).arg(getPin2());
    setInfoText(info);
}
