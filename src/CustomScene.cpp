#include "CustomScene.h"

#include <QMenu>

#include "TextDefinitions.h"

CustomScene::CustomScene(QObject* parent) : QGraphicsScene(parent)
{
    setupItems();
}

void CustomScene::clear()
{
    QGraphicsScene::clear();
    setupItems();
}

void CustomScene::setupItems()
{
    setupAppVersionItem();
    setupStatusItem();
}

void CustomScene::setupAppVersionItem()
{
    appVersionItem = addText(QStringLiteral(APP_VERSION));
    appVersionItem->setDefaultTextColor(Qt::red);
    appVersionItem->setFont(getDefaultFont());
    appVersionItem->setVisible(true);

    connect(this, &QGraphicsScene::sceneRectChanged, this, [this](const QRectF&) { updateAppVersionPos(); });

    updateAppVersionPos();
}

void CustomScene::updateAppVersionPos()
{
    if (!appVersionItem)
    {
        return;
    }

    const QRectF scRect   = sceneRect();
    const QRectF textRect = appVersionItem->boundingRect();

    const qreal x = std::max((scRect.right() - textRect.width()), scRect.left());
    const qreal y = scRect.top();

    appVersionItem->setPos(x, y);
}

void CustomScene::updateStatus(uint8_t pin1, uint8_t pin2, const QVector<uint8_t>& leds)
{
    // Update the status text item with the current pin and LED information
    QString statusText = QString("Status: Pins (%1, %2) LEDs: ").arg(pin1).arg(pin2);
    for (int i = 0; i < leds.size(); ++i)
    {
        statusText.append(QString::number(leds[i]));
        if (i < leds.size() - 1)
        {
            statusText.append(", ");
        }
    }
    statusItem->setPlainText(statusText);
}

void CustomScene::showStatus(bool on)
{
    statusItem->setVisible(on);
}

void CustomScene::setupStatusItem()
{
    statusItem = addText("Status: Ready");
    statusItem->setDefaultTextColor(Qt::red);
    statusItem->setFont(getDefaultFont());
}

void CustomScene::setModifiable(bool isMod)
{
    isModifiable = isMod;
}

void CustomScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    if (!isModifiable)
    {
        return;
    }

    QGraphicsScene::contextMenuEvent(event);
    if (event->isAccepted())
    {
        return;
    }

    QMenu    menu;
    QAction* actButton = menu.addAction("Добавить кнопку");
    QAction* actDiode  = menu.addAction("Добавить диод");

    QAction* chosen = menu.exec(event->screenPos());
    if (!chosen)
    {
        return;
    }

    QPointF pos = event->scenePos();

    if (chosen == actButton)
    {
        auto* btn = new ButtonItem(pos.x(), pos.y());
        addItem(btn);
        buttonAdded(btn);
    }
    else if (chosen == actDiode)
    {
        auto* d = new DiodeItem(pos.x(), pos.y());
        addItem(d);
        diodeAdded(d);
    }

    event->accept();
}
