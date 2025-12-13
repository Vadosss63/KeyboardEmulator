#pragma once

#include <QGraphicsView>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QStackedWidget>
#include <cstdint>

#include "CommandDefinition.h"
#include "CustomScene.h"
#include "PinsDefinition.h"
#include "RecentProjects.h"
#include "WorkMode.h"

class ImageZoomWidget;
class StartScreenWidget;
class WorkModeToolbar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

signals:
    // View → Controller
    void appExecuteCommand(Command command, Pins pins = {0, 0});

    void comPortSelected(const QString& portName);

    void updateDiodeStatus(Pins pins);

    void updateButtonStatus(Pins pins, bool isPressed);

    void clearButtonStatus();
    void clearDiodeStatus();

    void workModeChanged(WorkMode mode);

    void modifyModStatusChanged(bool isModifiable);
    void workingModStatusChanged(bool isActive);
    void checkModStatusChanged(bool isActive);

    void projectReady(bool isReady);

    void refreshComPortList();

public slots:
    // Controller → View
    void updateStatus(Pins pins, const QVector<Pins>& leds);

    void addDiodeItem(DiodeItem* diode);
    void addButtonItem(ButtonItem* button);
    void addResizableItem(ResizableRectItem* item);

    void updatePinStatus(AbstractItem* item);

    void updateComPort(const QString& portName);

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
    void handleRecentProjectRequested(const QString& path);
    void handleClearRecentRequested();

private:
    void refreshRecentProjects();

    void createImageViewer();

    void setupScene();

    void setupMenus();

    void clearItems();
    void setBackgroundImage(const QPixmap& pixmap);

    bool isWorkingMode() const;
    bool isModifyMode() const;
    bool isCheckMode() const;

    bool loadProjectFromPath(const QString& path);

    RecentProjects m_recent{QStringLiteral("recentProjects"), 5};

    StartScreenWidget* startScreen{nullptr};

    QStackedWidget* stackedWidget{nullptr};

    QPixmap backgroundImage{};

    CustomScene*   scene{nullptr};
    QGraphicsView* view{nullptr};

    ImageZoomWidget* imageViewer{nullptr};

    QList<DiodeItem*>  diodeItems{};
    QList<ButtonItem*> buttonItems{};

    WorkModeToolbar* workModeUi{nullptr};

    QAction* currentComPort{nullptr};

    WorkMode currentWorkMode{WorkMode::Work};

    ResizableRectItem* copiedItem{nullptr};
};
