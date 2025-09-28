#pragma once

#include <QMenu>
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

protected:
    void extendDerivedContextMenu(QMenu& menu) override;

private:
    void addPinConfigMenu(QMenu& menu);
};
