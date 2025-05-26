#pragma once

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>

#include "ButtonItem.h"
#include "DiodeItem.h"

class CustomScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit CustomScene(QObject* parent = nullptr);

signals:
    void diodeAdded(DiodeItem* diode);
    void buttonAdded(ButtonItem* button);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
};