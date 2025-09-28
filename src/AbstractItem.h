#pragma once

#include <QBrush>
#include <QMenu>
#include <QPen>
#include <cstdint>

#include "PinsDefinition.h"
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

    bool isShowExtendedMenu() const;
    void setShowExtendedMenu(bool isShow);

    virtual void updateTextInfo() = 0;

signals:
    void buttonPressed(Pins pins);
    void buttonReleased(Pins pins);
    void removeItem(AbstractItem* item);
    void pinsChanged(AbstractItem* item);

public slots:
    void setClickable(bool isClickable);
    void clearStatus();

    void onStatusUpdate(Pins pins);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    void setupDeleteItemAction(QAction* deleteAction) override;

private:
    bool isShowMenu{true};

    bool m_clickable{false};

    uint8_t m_pin1{};
    uint8_t m_pin2{};
};
