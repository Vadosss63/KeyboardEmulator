#pragma once

#include <QBrush>
#include <QColor>
#include <QMenu>
#include <QPen>
#include <cstdint>

#include "Project.h"
#include "ResizableRectItem.h"

class DiodeItem : public ResizableRectItem
{
    Q_OBJECT
public:
    DiodeItem(qreal x, qreal y);
    DiodeItem(const LedDef& def, QGraphicsItem* parent = nullptr);

    LedDef getDefinition() const;

    void setPin(uint8_t pin);
    void setInverted(bool inverted);

public slots:
    void onStatusUpdate(uint8_t pin, bool isOn);

signals:
    void removeDiode(DiodeItem* item);

protected:
    void extendDerivedContextMenu(QMenu& menu) override;
    void setupDeleteItemAction(QAction* deleteAction) override;

private:
    void addConfigMenu(QMenu& menu);

    void updateTextInfo();

    uint8_t m_pin      = 0;
    bool    m_inverted = false;
};
