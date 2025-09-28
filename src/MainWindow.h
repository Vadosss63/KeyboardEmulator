#pragma once

#include <QGraphicsView>
#include <QLabel>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QMenu>
#include <QStackedWidget>
#include <cstdint>

#include "ComPortMenu.h"
#include "CustomScene.h"
#include "PinsDefinition.h"
#include "RecentProjects.h"

class ImageZoomWidget;
class QListWidget;

enum class WorkMode
{
    Work = 0,
    Check,
    Modify,
    DiodeConf,
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

signals:
    // View → Controller
    void appButtonPressed(Pins pins);
    void appButtonReleased(Pins pins);
    void appDiodePressed(Pins pins);
    void appDiodeReleased(Pins pins);
    void appDiodePinConfigChanged(Pins newPins);

    void comPortSelected(const QString& portName);

    void updateDiodeStatus(Pins pins);

    void updateButtonStatus(Pins pins, bool isPressed);

    void workModeChanged(WorkMode mode);

    void modifyModStatusChanged(bool isModifiable);
    void workingModStatusChanged(bool isActive);
    void checkModStatusChanged(bool isActive);

    void projectReady(bool isReady);

public slots:
    // Controller → View
    void updateStatus(Pins pins, const QVector<Pins>& leds);

    void addDiodeItem(DiodeItem* diode);
    void addButtonItem(ButtonItem* button);
    void addResizableItem(ResizableRectItem* item);

    void updatePinStatus(AbstractItem* item);

private slots:
    void handleNewWorkMode(WorkMode mode);

    void saveProject();
    void loadProject();

private slots:
    void deleteButton(ButtonItem* button);
    void deleteDiode(DiodeItem* diode);

    void deleteItem(AbstractItem* item);

    void loadImage();

    void enableSceneMode(bool enable);

    void copyItem(ResizableRectItem* item);

    void pasteItem(QPointF pos);

    void openRecentItem(QListWidgetItem* item);
    void clearRecentList();

private:
    void createStartWidget();

    void resizeRecentListToContents();

    void updateRecentListWidget();

    void createImageViewer();

    void setupScene();
    void setupToolbar();

    void setupMenus();

    void clearItems();
    void setBackgroundImage(const QPixmap& pixmap);

    bool isWorkingMode() const;
    bool isModifyMode() const;
    bool isCheckMode() const;

    bool loadProjectFromPath(const QString& path);

    QWidget* startWidget{nullptr};

    RecentProjects m_recent{QStringLiteral("recentProjects"), 5};

    QListWidget* recentList{nullptr};

    QStackedWidget* stackedWidget{nullptr};

    QPixmap backgroundImage{};

    CustomScene*   scene{nullptr};
    QGraphicsView* view{nullptr};

    ImageZoomWidget* imageViewer{nullptr};

    QList<DiodeItem*>  diodeItems{};
    QList<ButtonItem*> buttonItems{};

    QAction* loadImgAction{nullptr};
    QAction* modeCheckAction{nullptr};
    QAction* modeRunAction{nullptr};
    QAction* modifyAction{nullptr};

    QAction* saveProjectAction{nullptr};
    QAction* loadProjectAction{nullptr};

    QAction* statusAction{nullptr};

    ComPortMenu* comMenu{nullptr};

    WorkMode currentWorkMode{WorkMode::Work};

    ResizableRectItem* copiedItem{nullptr};
};