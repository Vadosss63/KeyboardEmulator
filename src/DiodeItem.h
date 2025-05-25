#pragma once

#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

#include "ResizableRectItem.h"

class DiodeItem : public ResizableRectItem
{
public:
    DiodeItem(qreal x, qreal y, qreal w = 80, qreal h = 80, QGraphicsItem* parent = nullptr);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    QPainterPath shape() const override;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void updateAppearance();

    bool m_active = false;

    QBrush m_offBrush;
    QBrush m_onBrush;

    QColor m_offColor{Qt::gray};
    QColor m_onColor{Qt::green};
};
