#pragma once

#include <QBrush>
#include <QMenu>
#include <QPen>
#include <cstdint>

#include "Project.h"
#include "ResizableRectItem.h"

class AbstractItem : public ResizableRectItem
{
    Q_OBJECT
public:
    AbstractItem(const ItemDef& def, QGraphicsItem* parent = nullptr);

    ItemDef getDefinition() const;

    void setPin1(uint8_t pin);
    void setPin2(uint8_t pin);

    uint8_t getPin1() const;
    uint8_t getPin2() const;

    virtual void updateTextInfo() = 0;

signals:
    void buttonPressed(uint8_t pin1, uint8_t pin2);
    void buttonReleased(uint8_t pin1, uint8_t pin2);
    void removeItem(AbstractItem* item);

public slots:
    void setClickable(bool isClickable);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    void setupDeleteItemAction(QAction* deleteAction) override;

private:
    bool m_clickable{};

    uint8_t m_pin1{};
    uint8_t m_pin2{};
};
