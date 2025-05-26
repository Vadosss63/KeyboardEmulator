#pragma once

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>

class CustomScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit CustomScene(QObject* parent = nullptr);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
};