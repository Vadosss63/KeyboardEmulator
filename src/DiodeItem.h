#pragma once

#include <QMenu>
#include <cstdint>

#include "AbstractItem.h"

class DiodeItem : public AbstractItem
{
    Q_OBJECT
public:
    DiodeItem(qreal x, qreal y);
    DiodeItem(const ItemDef& def, QGraphicsItem* parent = nullptr);

    ResizableRectItem* clone() const override;

    void updateTextInfo() override;

public slots:
    void onStatusUpdate(uint8_t pin, bool isOn);
    void onCheckModStatusChanged(bool isCheckMode);

protected:
    void extendDerivedContextMenu(QMenu& menu) override;

private:
    void addConfigMenu(QMenu& menu);

    bool isCheckMod() const;

    bool m_isCheckMod{false};
};
