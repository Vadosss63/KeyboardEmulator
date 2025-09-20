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

    void setPin1(uint8_t pin);
    void setPin2(uint8_t pin);

    ResizableRectItem* clone() const override;

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

    uint8_t m_pin1 = 0;
    uint8_t m_pin2 = 0;
};
