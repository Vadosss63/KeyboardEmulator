#include "ButtonItem.h"

#include <QAction>
#include <QDebug>
#include <QGraphicsSceneContextMenuEvent>

ButtonItem::ButtonItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent)
    : ResizableRectItem(x, y, w, h, parent), m_normalBrush(Qt::transparent), m_activeBrush(QColor(0, 0, 0, 80))
{
    QPen pen(Qt::blue, 2);
    pen.setStyle(Qt::DashLine);
    setPen(pen);
    setBrush(m_normalBrush);
}

void ButtonItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    m_active = !m_active;
    updateAppearance();
    ResizableRectItem::mousePressEvent(event);
}

void ButtonItem::extendContextMenu(QMenu& menu)
{
    QAction* cfg = menu.addAction(tr("Настроить выводы"));
    cfg->setData(QStringLiteral("pin_config"));
}

bool ButtonItem::handleDerivedContextMenuAction(QAction* action)
{
    if (!action)
    {
        return false;
    }

    if (action->data().toString() == QLatin1String("pin_config"))
    {
        showPinConfigMenu(QCursor::pos());
        return true;
    }
    return false;
}

void ButtonItem::showPinConfigMenu(const QPoint& screenPos)
{
    QMenu  menu;
    QMenu* pin1Menu = menu.addMenu(tr("Set Pin 1"));
    QMenu* pin2Menu = menu.addMenu(tr("Set Pin 2"));

    for (uint8_t i = 1; i <= 15; ++i)
    {
        QAction* act1 = pin1Menu->addAction(QString::number(i));
        connect(act1,
                &QAction::triggered,
                this,
                [this, i]()
                {
                    m_pin1 = i;
                    qDebug() << "ButtonItem: Pin1 set to" << i;
                    emit pinsAssigned(m_pin1, m_pin2);
                });
        QAction* act2 = pin2Menu->addAction(QString::number(i));
        connect(act2,
                &QAction::triggered,
                this,
                [this, i]()
                {
                    m_pin2 = i;
                    qDebug() << "ButtonItem: Pin2 set to" << i;
                    emit pinsAssigned(m_pin1, m_pin2);
                });
    }

    menu.addSeparator();
    QAction* titleAct = menu.addAction(tr("Current:"));
    titleAct->setEnabled(false);
    QAction* pin1Act = menu.addAction(tr("Pin1: %1").arg(m_pin1));
    pin1Act->setEnabled(false);
    QAction* pin2Act = menu.addAction(tr("Pin2: %1").arg(m_pin2));
    pin2Act->setEnabled(false);

    menu.exec(screenPos);
}

void ButtonItem::updateAppearance()
{
    setBrush(m_active ? m_activeBrush : m_normalBrush);
    update();
}
