#pragma once

#include <QBrush>
#include <QMenu>
#include <QPen>
#include <cstdint>

#include "Project.h"
#include "ResizableRectItem.h"

class ButtonItem : public ResizableRectItem
{
    Q_OBJECT
public:
    ButtonItem(qreal          x,
               qreal          y,
               qreal          w      = 80,
               qreal          h      = 80,
               QGraphicsItem* parent = nullptr,
               uint8_t        pin1   = 0,
               uint8_t        pin2   = 0);

    ButtonItem(const ButtonDef& def, QGraphicsItem* parent = nullptr);

    ButtonDef getDefinition() const;

    void setPin1(uint8_t pin);
    void setPin2(uint8_t pin);

signals:
    void buttonPressed(uint8_t pin1, uint8_t pin2);
    void buttonReleased(uint8_t pin1, uint8_t pin2);
    void removeButton(ButtonItem* item);

public slots:
    void onStatusUpdate(uint8_t pin1, uint8_t pin2, bool isPressed);
    void setClickable(bool isClickable);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    void setupDeleteItemAction(QAction* deleteAction) override;
    void extendDerivedContextMenu(QMenu& menu) override;

private:
    void updateAppearance();
    void addPinConfigMenu(QMenu& menu);

    void updateTextInfo();

    bool    m_active{};
    bool    m_clickable{};
    QBrush  m_normalBrush;
    QBrush  m_activeBrush;
    uint8_t m_pin1{};
    uint8_t m_pin2{};
};
