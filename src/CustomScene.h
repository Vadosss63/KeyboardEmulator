#pragma once

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>

#include "ButtonItem.h"
#include "DiodeItem.h"

class CustomScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit CustomScene(QObject* parent = nullptr) : QGraphicsScene(parent) {}

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override
    {
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
};