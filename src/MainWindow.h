#pragma once

#include <QGraphicsView>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QStackedWidget>
#include <cstdint>
#include <memory>

#include "CommandDefinition.h"
#include "CustomScene.h"
#include "IFileDialogService.h"
#include "IMessageService.h"
#include "PinsDefinition.h"
#include "RecentProjects.h"
#include "WorkMode.h"
#include "WorkModeState.h"

class ImageZoomWidget;
class StartScreenWidget;
class ComPortMenu;
class WorkModeToolbar;
class SceneController;
class WorkModeState;

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

    void updatePinStatus(AbstractItem* item);

    void updateComPort(const QString& portName);
    void showWarning(const QString& title, const QString& text);

private slots:
    void saveProject();
    void loadProject();

private slots:
    void loadImage();

    void enableSceneMode(bool enable);

    void handleRecentProjectRequested(const QString& path);
    void handleClearRecentRequested();
    void bindDiodeItem(DiodeItem* diode);
    void bindButtonItem(ButtonItem* button);
    void handleDiodeRemoval(DiodeItem* diode);
    void handleButtonRemoval(ButtonItem* button);

private:
    void refreshRecentProjects();

    void createImageViewer();

    void setupScene();

    void setupMenus();

    void clearItems();
    void setBackgroundImage(const QPixmap& pixmap);

    WorkMode currentMode() const;

    bool loadProjectFromPath(const QString& path);

    RecentProjects m_recent{QStringLiteral("recentProjects"), 5};

    StartScreenWidget* startScreen{nullptr};

    QStackedWidget* stackedWidget{nullptr};

    QPixmap backgroundImage{};

    CustomScene*   scene{nullptr};
    QGraphicsView* view{nullptr};

    ImageZoomWidget* imageViewer{nullptr};

    WorkModeToolbar* workModeUi{nullptr};
    WorkModeState*   workModeState{nullptr};
    ComPortMenu*     comPortMenu{nullptr};
    SceneController* sceneController{nullptr};

    std::unique_ptr<IFileDialogService> fileDialogs;
    std::unique_ptr<IMessageService>    messageService;
};
