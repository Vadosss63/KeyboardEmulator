#pragma once

#include <QGraphicsView>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QStackedWidget>
#include <cstdint>

#include "ComPortMenu.h"
#include "CustomScene.h"

enum class WorkMode
{
    Work = 0,
    Check,
    Modify,
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

signals:
    // View → Controller
    void appButtonPressed(uint8_t pin1, uint8_t pin2);
    void appButtonReleased(uint8_t pin1, uint8_t pin2);
    void appEnterModeCheck();
    void appEnterModeRun();
    void appEnterModeConfigure();
    void comPortSelected(const QString& portName);

    void updateDiodeStatus(uint8_t pin, bool isOn);

    void updateButtonStatus(uint8_t pin1, uint8_t pin2, bool isPressed);

    void workModeChanged(WorkMode mode);

    void modifyModStatusChanged(bool isModifiable);
    void workingModStatusChanged(bool isActive);

    void projectReady(bool isReady);

public slots:
    // Controller → View
    void updateStatus(uint8_t pin1, uint8_t pin2, const QVector<uint8_t>& leds);

    void addDiodeItem(DiodeItem* diode);
    void addButtonItem(ButtonItem* button);
    void addResizableItem(ResizableRectItem* item);

private slots:
    void handleNewWorkMode(WorkMode mode);

    void saveProject();
    void loadProject();

private slots:
    void deleteButton(ButtonItem* button);
    void deleteDiode(DiodeItem* diode);
    void loadImage();

    void enableSceneMode(bool enable);

    void copyItem(ResizableRectItem* item);

    void pasteItem(QPointF pos);

private:
    void createStartWidget();

    void setupScene();
    void setupToolbar();

    void setupMenus();

    void clearItems();
    void setBackgroundImage(const QPixmap& pixmap);

    bool isWorkingMode() const;
    bool isModifyMode() const;
    bool isCheckMode() const;

    QWidget* startWidget{nullptr};

    QStackedWidget* stackedWidget{nullptr};

    QPixmap backgroundImage{};

    CustomScene*   scene{nullptr};
    QGraphicsView* view{nullptr};

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