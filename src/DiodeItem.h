#pragma once

#include <QBrush>
#include <QColor>
#include <QMenu>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>
#include <cstdint>

#include "Project.h"
#include "ResizableRectItem.h"

class DiodeItem : public ResizableRectItem
{
    Q_OBJECT
public:
    DiodeItem(qreal x, qreal y, qreal w = 80, qreal h = 80, QGraphicsItem* parent = nullptr);
    DiodeItem(const LedDef& def, QGraphicsItem* parent = nullptr);

    LedDef getDefinition() const;

public slots:
    void onStatusUpdate(uint8_t pin, bool isOn);

signals:
    void pinAssigned(uint8_t pin);
    void inversionChanged(bool inverted);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    QPainterPath shape() const override;

    void handleDerivedContextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
    void updateAppearance();
    void addConfigMenu(QMenu& menu);

    bool m_active = false;

    QBrush m_offBrush;
    QBrush m_onBrush;
    QColor m_offColor{Qt::gray};
    QColor m_onColor{Qt::green};

    uint8_t m_pin      = 0;
    bool    m_inverted = false;
};
