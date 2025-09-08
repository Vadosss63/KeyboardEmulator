#include "CustomScene.h"

#include <QMenu>

#include "TextDefinitions.h"

CustomScene::CustomScene(QObject* parent) : QGraphicsScene(parent) {}

void CustomScene::clear()
{
    QGraphicsScene::clear();
}

void CustomScene::setPasteEnabled(bool isEnabled)
{
    isPasteEnabled = isEnabled;
}

void CustomScene::setModifiable(bool isMod)
{
    isModifiable = isMod;
}

void CustomScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QGraphicsScene::contextMenuEvent(event);

    if (!isModifiable)
    {
        return;
    }

    if (event->isAccepted())
    {
        return;
    }

    QMenu    menu;
    QAction* actButton = menu.addAction("Добавить кнопку");
    QAction* actDiode  = menu.addAction("Добавить диод");
    QAction* actPaste  = menu.addAction("Вставить");
    actPaste->setEnabled(isPasteEnabled);

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
    else if (chosen == actPaste)
    {
        emit pasteItem(pos);
    }

    event->accept();
}
