#include "CustomScene.h"

#include <QMenu>

CustomScene::CustomScene(QObject* parent) : QGraphicsScene(parent)
{
    statusItem = addText("Status: Ready");
    statusItem->setDefaultTextColor(Qt::red);
    statusItem->setFont(QFont("Arial", 14));
}

void CustomScene::updateStatus(uint8_t pin1, uint8_t pin2, const QVector<uint8_t>& leds)
{
    // Update the status text item with the current pin and LED information
    QString statusText = QString("Status: Pins (%1, %2) LEDs: ").arg(pin1).arg(pin2);
    for (int i = 0; i < leds.size(); ++i)
    {
        statusText.append(QString::number(leds[i]));
        if (i < leds.size() - 1)
        {
            statusText.append(", ");
        }
    }
    statusItem->setPlainText(statusText);
}

void CustomScene::showStatus(bool on)
{
    statusItem->setVisible(on);
}

void CustomScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QGraphicsScene::contextMenuEvent(event);
    if (event->isAccepted())
    {
        return;
    }

    QMenu    menu;
    QAction* actButton = menu.addAction("Добавить кнопку");
    QAction* actDiode  = menu.addAction("Добавить диод");

    QAction* chosen = menu.exec(event->screenPos());
    if (!chosen)
    {
        return;
    }

    QPointF pos = event->scenePos();

    if (chosen == actButton)
    {
        auto* btn = new ButtonItem(pos.x(), pos.y());
        addItem(btn);
        buttonAdded(btn);
    }
    else if (chosen == actDiode)
    {
        auto* d = new DiodeItem(pos.x(), pos.y());
        addItem(d);
        diodeAdded(d);
    }

    event->accept();
}
