#pragma once

#include <QList>
#include <QObject>
#include <QPixmap>
#include <memory>

#include "WorkMode.h"

class ButtonItem;
class CustomScene;
class DiodeItem;
class QGraphicsPixmapItem;
class ResizableRectItem;
class AbstractItem;

class SceneController : public QObject
{
    Q_OBJECT

public:
    explicit SceneController(CustomScene* scene, QObject* parent = nullptr);

    void           setBackground(const QPixmap& pixmap);
    const QPixmap& background() const;

    const QList<DiodeItem*>&  diodes() const;
    const QList<ButtonItem*>& buttons() const;

    void addExistingDiode(DiodeItem* diode);
    void addExistingButton(ButtonItem* button);

public slots:
    void setModifyMode(bool enabled);
    void copyItem(ResizableRectItem* item);
    void deleteItem(AbstractItem* item);

signals:
    void diodeReady(DiodeItem* diode);
    void buttonReady(ButtonItem* button);
    void diodeAboutToBeRemoved(DiodeItem* diode);
    void buttonAboutToBeRemoved(ButtonItem* button);

private slots:
    void handleSceneDiodeAdded(DiodeItem* diode);
    void handleSceneButtonAdded(ButtonItem* button);
    void handleScenePaste(QPointF pos);

private:
    void registerDiode(DiodeItem* diode, bool createdByScene);
    void registerButton(ButtonItem* button, bool createdByScene);
    void registerResizable(ResizableRectItem* item);
    void removeDiode(DiodeItem* diode);
    void removeButton(ButtonItem* button);
    void clearClipboard();
    void updatePasteAvailability() const;

    CustomScene*                       m_scene{nullptr};
    QList<DiodeItem*>                  m_diodes;
    QList<ButtonItem*>                 m_buttons;
    std::unique_ptr<ResizableRectItem> m_copiedItem;
    QPixmap                            m_background;
    bool                               m_modifyMode{false};
};
