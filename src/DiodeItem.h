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
    DiodeItem(qreal          x,
              qreal          y,
              qreal          w        = 80,
              qreal          h        = 80,
              QGraphicsItem* parent   = nullptr,
              uint8_t        pin      = 0,
              bool           inverted = false);
    DiodeItem(const LedDef& def, QGraphicsItem* parent = nullptr);

    LedDef getDefinition() const;

    void setPin(uint8_t pin);
    void setInverted(bool inverted);

public slots:
    void onStatusUpdate(uint8_t pin, bool isOn);

signals:
    void removeDiode(DiodeItem* item);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    QPainterPath shape() const override;

    void extendDerivedContextMenu(QMenu& menu) override;
    void setupDeleteItemAction(QAction* deleteAction) override;

private:
    void updateAppearance();
    void addConfigMenu(QMenu& menu);

    void updateTextInfo();

    bool m_active = false;

    QBrush m_offBrush;
    QBrush m_onBrush;
    QColor m_offColor{Qt::gray};
    QColor m_onColor{Qt::green};

    uint8_t m_pin      = 0;
    bool    m_inverted = false;
};
