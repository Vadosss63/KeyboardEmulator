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

    void updateStatus(uint8_t pin1, uint8_t pin2, const QVector<uint8_t>& leds);
    void showStatus(bool on);

    void addStatusItem();

signals:
    void diodeAdded(DiodeItem* diode);
    void buttonAdded(ButtonItem* button);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
    QGraphicsTextItem* statusItem = nullptr;
};