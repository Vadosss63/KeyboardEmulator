#include "SceneController.h"

#include <QGraphicsPixmapItem>
#include <QPointF>

#include "AbstractItem.h"
#include "ButtonItem.h"
#include "CustomScene.h"
#include "DiodeItem.h"
#include "ResizableRectItem.h"

SceneController::SceneController(CustomScene* scene, QObject* parent) : QObject(parent), m_scene(scene)
{
    Q_ASSERT(m_scene);
    connect(m_scene, &CustomScene::diodeAdded, this, &SceneController::handleSceneDiodeAdded);
    connect(m_scene, &CustomScene::buttonAdded, this, &SceneController::handleSceneButtonAdded);
    connect(m_scene, &CustomScene::pasteItem, this, &SceneController::handleScenePaste);
}

void SceneController::setModifyMode(bool enabled)
{
    m_modifyMode = enabled;
    for (ResizableRectItem* item : m_diodes)
    {
        if (item)
        {
            item->setResizable(enabled);
        }
    }
    for (ResizableRectItem* item : m_buttons)
    {
        if (item)
        {
            item->setResizable(enabled);
        }
    }
}

void SceneController::setBackground(const QPixmap& pixmap)
{
    if (!m_scene)
    {
        return;
    }

    m_scene->clear();
    m_background = pixmap;
    auto* pix    = new QGraphicsPixmapItem(m_background);
    pix->setZValue(-1);
    m_scene->addItem(pix);

    m_diodes.clear();
    m_buttons.clear();
    clearClipboard();
}

const QPixmap& SceneController::background() const
{
    return m_background;
}

const QList<DiodeItem*>& SceneController::diodes() const
{
    return m_diodes;
}

const QList<ButtonItem*>& SceneController::buttons() const
{
    return m_buttons;
}

void SceneController::addExistingDiode(DiodeItem* diode)
{
    if (!m_scene || !diode)
    {
        return;
    }

    m_scene->addItem(diode);
    registerDiode(diode, /*createdByScene=*/false);
}

void SceneController::addExistingButton(ButtonItem* button)
{
    if (!m_scene || !button)
    {
        return;
    }

    m_scene->addItem(button);
    registerButton(button, /*createdByScene=*/false);
}

void SceneController::copyItem(ResizableRectItem* item)
{
    if (!item)
    {
        clearClipboard();
        return;
    }

    m_copiedItem.reset(item->clone());
    updatePasteAvailability();
}

void SceneController::deleteItem(AbstractItem* item)
{
    if (!item)
    {
        return;
    }

    if (auto* diode = dynamic_cast<DiodeItem*>(item))
    {
        removeDiode(diode);
        return;
    }

    if (auto* button = dynamic_cast<ButtonItem*>(item))
    {
        removeButton(button);
        return;
    }
}

void SceneController::handleSceneDiodeAdded(DiodeItem* diode)
{
    registerDiode(diode, /*createdByScene=*/true);
}

void SceneController::handleSceneButtonAdded(ButtonItem* button)
{
    registerButton(button, /*createdByScene=*/true);
}

void SceneController::handleScenePaste(QPointF pos)
{
    if (!m_scene || !m_copiedItem)
    {
        return;
    }

    std::unique_ptr<ResizableRectItem> clone(m_copiedItem->clone());
    clone->setPos(clone->pos() + QPointF(10, 10));

    ResizableRectItem* raw = clone.get();

    if (auto* diode = dynamic_cast<DiodeItem*>(raw))
    {
        clone.release();
        m_scene->addItem(diode);
        registerDiode(diode, /*createdByScene=*/true);
        return;
    }

    if (auto* button = dynamic_cast<ButtonItem*>(raw))
    {
        clone.release();
        m_scene->addItem(button);
        registerButton(button, /*createdByScene=*/true);
        return;
    }
}

void SceneController::registerDiode(DiodeItem* diode, bool createdByScene)
{
    if (!diode)
    {
        return;
    }

    m_diodes.append(diode);
    registerResizable(diode);
    connect(diode, &DiodeItem::removeItem, this, &SceneController::deleteItem);
    emit diodeReady(diode);

    if (createdByScene)
    {
        // scene already has the item, nothing additional
    }
}

void SceneController::registerButton(ButtonItem* button, bool createdByScene)
{
    if (!button)
    {
        return;
    }

    m_buttons.append(button);
    registerResizable(button);
    connect(button, &ButtonItem::removeItem, this, &SceneController::deleteItem);
    emit buttonReady(button);

    if (createdByScene)
    {
        // scene already has the item
    }
}

void SceneController::registerResizable(ResizableRectItem* item)
{
    connect(item, &ResizableRectItem::itemCopied, this, &SceneController::copyItem);
    item->setResizable(m_modifyMode);
}

void SceneController::removeDiode(DiodeItem* diode)
{
    if (!diode)
    {
        return;
    }

    emit diodeAboutToBeRemoved(diode);
    if (m_scene)
    {
        m_scene->removeItem(diode);
    }
    m_diodes.removeOne(diode);
    delete diode;
}

void SceneController::removeButton(ButtonItem* button)
{
    if (!button)
    {
        return;
    }

    emit buttonAboutToBeRemoved(button);
    if (m_scene)
    {
        m_scene->removeItem(button);
    }
    m_buttons.removeOne(button);
    delete button;
}

void SceneController::clearClipboard()
{
    m_copiedItem.reset();
    updatePasteAvailability();
}

void SceneController::updatePasteAvailability() const
{
    if (m_scene)
    {
        m_scene->setPasteEnabled(m_copiedItem != nullptr);
    }
}
