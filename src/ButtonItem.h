#pragma once

#include <QBrush>
#include <QMenu>
#include <QPen>
#include <cstdint>

#include "ResizableRectItem.h"

class ButtonItem : public ResizableRectItem
{
    Q_OBJECT
public:
    ButtonItem(qreal x, qreal y, qreal w = 80, qreal h = 80, QGraphicsItem* parent = nullptr);

signals:
    void pinsAssigned(uint8_t pin1, uint8_t pin2);
    void buttonPressed(uint8_t pin1, uint8_t pin2);
    void buttonReleased(uint8_t pin1, uint8_t pin2);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    void extendContextMenu(QMenu& menu) override;
    bool handleDerivedContextMenuAction(QAction* action) override;

private:
    void updateAppearance();
    void showPinConfigMenu(const QPoint& screenPos);

    bool    m_active = false;
    QBrush  m_normalBrush;
    QBrush  m_activeBrush;
    uint8_t m_pin1 = 0;
    uint8_t m_pin2 = 0;
};