#include "DiodeItem.h"

#include <QAction>
#include <QCursor>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

DiodeItem::DiodeItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent) : ResizableRectItem(x, y, w, h, parent)
{
    QColor offColor = m_offColor;
    offColor.setAlphaF(0.3);
    m_offBrush = QBrush(offColor);

    QColor onColor = m_onColor;
    onColor.setAlphaF(0.3);
    m_onBrush = QBrush(onColor);

    setPen(QPen(m_offColor, 2));
    setBrush(m_offBrush);
}

DiodeItem::DiodeItem(const LedDef& def, QGraphicsItem* parent)
    : ResizableRectItem(def.rect.x(), def.rect.y(), def.rect.width(), def.rect.height(), parent)
    , m_pin(def.pin)
    , m_inverted(def.inverted)
{
    QColor offColor = m_offColor;
    offColor.setAlphaF(0.3);
    m_offBrush = QBrush(offColor);

    QColor onColor = m_onColor;
    onColor.setAlphaF(0.3);
    m_onBrush = QBrush(onColor);

    setPen(QPen(m_offColor, 2));
    setBrush(m_offBrush);
}

LedDef DiodeItem::getDefinition() const
{
    LedDef def;
    def.rect     = rectItem();
    def.pin      = m_pin;
    def.inverted = m_inverted;
    return def;
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

    QAction* pinAct = menu.addAction(tr("Pin: %1").arg(m_pin));
    pinAct->setEnabled(false);
    QAction* invAct = menu.addAction(tr("Inverted: %1").arg(m_inverted));
    invAct->setEnabled(false);
}

void DiodeItem::setupDeleteItemAction(QAction* deleteAction)
{
    connect(deleteAction, &QAction::triggered, this, [this] { emit removeDiode(this); });
}

void DiodeItem::addConfigMenu(QMenu& menu)
{
    QMenu* pinMenu = menu.addMenu(tr("Set Pin"));
    QMenu* invMenu = menu.addMenu(tr("Inversion"));

    // Pin selection 1..15
    for (uint8_t i = 1; i <= 15; ++i)
    {
        QAction* act = pinMenu->addAction(QString::number(i));
        connect(act,
                &QAction::triggered,
                this,
                [this, i]()
                {
                    m_pin = i;
                    qDebug() << "DiodeItem: Pin set to" << i;
                    emit pinAssigned(m_pin);
                });
    }

    QAction* invOn  = invMenu->addAction(tr("Non-inverted"));
    QAction* invOff = invMenu->addAction(tr("Inverted"));
    invOn->setData(QVariant(false));
    invOff->setData(QVariant(true));
    connect(invOn,
            &QAction::triggered,
            this,
            [this]()
            {
                m_inverted = false;
                qDebug() << "DiodeItem: Non-inverted";
                emit inversionChanged(m_inverted);
            });
    connect(invOff,
            &QAction::triggered,
            this,
            [this]()
            {
                m_inverted = true;
                qDebug() << "DiodeItem: Inverted";
                emit inversionChanged(m_inverted);
            });

    menu.addSeparator();
}

void DiodeItem::updateAppearance()
{
    setBrush(m_active ? m_onBrush : m_offBrush);
    QColor color = m_active ? m_onColor : m_offColor;
    setPen(QPen(color, 2));
    update();
}
