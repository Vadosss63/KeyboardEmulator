#pragma once

#include <QMenu>
#include <cstdint>

#include "AbstractItem.h"
#include "PinsDefinition.h"

class DiodeItem : public AbstractItem
{
    Q_OBJECT
public:
    DiodeItem(qreal x, qreal y);
    DiodeItem(const ItemDef& def, QGraphicsItem* parent = nullptr);

    ResizableRectItem* clone() const override;

    void updateTextInfo() override;

public slots:
    void onStatusUpdate(Pins pins);
    void onCheckModStatusChanged(bool isCheckMode);

protected:
    void extendDerivedContextMenu(QMenu& menu) override;

private:
    void addConfigMenu(QMenu& menu);

    bool isCheckMod() const;

    bool m_isCheckMod{false};
};
