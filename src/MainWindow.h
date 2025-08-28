#pragma once

#include <QGraphicsView>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <cstdint>

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
    void comPortSelected(const QString& portName);

    void updateDiodeStatus(uint8_t pin, bool isOn);

    void updateButtonStatus(uint8_t pin1, uint8_t pin2, bool isPressed);

    void workModeChanged(WorkMode mode);

    void modifyModStatusChanged(bool isModifiable);
    void workingModStatusChanged(bool isActive);

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

private:
    void setupScene();
    void setupToolbar();
    void setupMenus();
    void refreshComPorts();

    bool isWorkingMode() const;
    bool isModifyMode() const;
    bool isCheckMode() const;

    QPixmap backgroundImage;

    CustomScene*   scene;
    QGraphicsView* view;

    QList<DiodeItem*>  diodeItems;
    QList<ButtonItem*> buttonItems;

    QAction* loadImgAction;
    QAction* modeCheckAction;
    QAction* modeRunAction;
    QAction* modifyAction;

    QAction* saveProjectAction;
    QAction* loadProjectAction;

    QMenu* comMenu;

    /// TODO: Check initial state
    WorkMode currentWorkMode{WorkMode::Work};
};