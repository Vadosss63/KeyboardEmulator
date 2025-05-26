#include "CustomScene.h"

#include <QMenu>

#include "ButtonItem.h"
#include "DiodeItem.h"

CustomScene::CustomScene(QObject* parent) : QGraphicsScene(parent) {}

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
    }
    else if (chosen == actDiode)
    {
        auto* d = new DiodeItem(pos.x(), pos.y());
        addItem(d);
    }

    event->accept();
}
