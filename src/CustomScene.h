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

    void clear();

    void setPasteEnabled(bool isEnabled);

signals:
    void diodeAdded(DiodeItem* diode);
    void buttonAdded(ButtonItem* button);

    void pasteItem(QPointF pos);

public slots:
    void setModifiable(bool isMod);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
    bool isModifiable{};
    bool isPasteEnabled{false};
};