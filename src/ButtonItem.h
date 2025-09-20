#pragma once

#include <QBrush>
#include <QMenu>
#include <QPen>
#include <cstdint>

#include "AbstractItem.h"

class ButtonItem : public AbstractItem
{
    Q_OBJECT
public:
    ButtonItem(qreal x, qreal y);

    ButtonItem(const ItemDef& def, QGraphicsItem* parent = nullptr);

    ResizableRectItem* clone() const override;

    void updateTextInfo() override;

public slots:
    void onStatusUpdate(uint8_t pin1, uint8_t pin2, bool isPressed);

protected:
    void extendDerivedContextMenu(QMenu& menu) override;

private:
    void addPinConfigMenu(QMenu& menu);
};
