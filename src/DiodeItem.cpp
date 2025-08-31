#include "DiodeItem.h"

#include <QAction>
#include <QCursor>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

DiodeItem::DiodeItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent, uint8_t pin, bool inverted)
    : ResizableRectItem(x, y, w, h, parent), m_pin(pin), m_inverted(inverted)
{
    QColor offColor = m_offColor;
    offColor.setAlphaF(0.3);
    m_offBrush = QBrush(offColor);

    QColor onColor = m_onColor;
    onColor.setAlphaF(0.3);
    m_onBrush = QBrush(onColor);

    setPen(QPen(m_offColor, 2));
    setBrush(m_offBrush);

    setPin(pin);
    setInverted(inverted);
}

DiodeItem::DiodeItem(const LedDef& def, QGraphicsItem* parent)
    : DiodeItem(def.rect.x(), def.rect.y(), def.rect.width(), def.rect.height(), parent, def.pin, def.inverted)
{
}

LedDef DiodeItem::getDefinition() const
{
    LedDef def;
    def.rect     = rectItem();
    def.pin      = m_pin;
    def.inverted = m_inverted;
    return def;
}

void DiodeItem::setPin(uint8_t pin)
{
    m_pin = pin;
    updateTextInfo();
}

void DiodeItem::setInverted(bool inverted)
{
    m_inverted = inverted;
    updateTextInfo();
    onStatusUpdate(m_pin, false);
}

void DiodeItem::onStatusUpdate(uint8_t pin, bool isOn)
{
    if (pin != m_pin)
    {
        return;
    }

    bool actualState = m_inverted ? !isOn : isOn;
    m_active         = actualState;

    updateAppearance();
}

void DiodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);
    painter->setPen(pen());
    if (option->state & QStyle::State_Selected)
    {
        QPen selPen = pen();
        selPen.setStyle(Qt::DashLine);
        painter->setPen(selPen);
    }
    painter->setBrush(brush());
    painter->drawEllipse(rect());
}

QPainterPath DiodeItem::shape() const
{
    QPainterPath path;
    path.addEllipse(rect());
    return path;
}

void DiodeItem::extendDerivedContextMenu(QMenu& menu)
{
    if (isModifyMod())
    {
        addConfigMenu(menu);
    }

    QAction* pinAct = menu.addAction(tr("Пин: %1").arg(m_pin));
    pinAct->setEnabled(false);
    QAction* invAct = menu.addAction(tr("Инверсия: %1").arg(m_inverted));
    invAct->setEnabled(false);
}

void DiodeItem::setupDeleteItemAction(QAction* deleteAction)
{
    connect(deleteAction, &QAction::triggered, this, [this] { emit removeDiode(this); });
}

void DiodeItem::addConfigMenu(QMenu& menu)
{
    QAction* circleShape = menu.addAction(tr("Круглая форма"));
    connect(circleShape, &QAction::triggered, this, &DiodeItem::makeRectShape);

    QMenu* pinMenu = menu.addMenu(tr("Установить пин"));
    QMenu* invMenu = menu.addMenu(tr("Инверсия"));

    // Pin selection 1..15
    for (uint8_t i = 1; i <= 15; ++i)
    {
        QAction* act = pinMenu->addAction(QString::number(i));
        connect(act, &QAction::triggered, this, [this, i]() { setPin(i); });
    }

    QAction* invOn  = invMenu->addAction(tr("Non-inverted"));
    QAction* invOff = invMenu->addAction(tr("Inverted"));
    invOn->setData(QVariant(false));
    invOff->setData(QVariant(true));
    connect(invOn, &QAction::triggered, this, [this]() { setInverted(false); });
    connect(invOff, &QAction::triggered, this, [this]() { setInverted(true); });

    menu.addSeparator();
}

void DiodeItem::updateTextInfo()
{
    const QString info = QString("P:%1\nI:%2").arg(m_pin).arg(m_inverted);
    setInfoText(info);
}

void DiodeItem::updateAppearance()
{
    setBrush(m_active ? m_onBrush : m_offBrush);
    QColor color = m_active ? m_onColor : m_offColor;
    setPen(QPen(color, 2));
    update();
}
