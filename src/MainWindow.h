#pragma once

#include <QGraphicsView>
#include <QMainWindow>
#include <QMenu>
#include <cstdint>

#include "CustomScene.h"

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

public slots:
    // Controller → View
    void markButtonPressed(uint8_t pin1, uint8_t pin2);
    void markButtonReleased(uint8_t pin1, uint8_t pin2);
    void enterCheckMode();
    void enterRunMode();
    void updateStatus(uint8_t status, uint8_t pin1, uint8_t pin2, const QVector<uint8_t>& leds);

    void addDiodeItem(DiodeItem* diode);
    void addButtonItem(ButtonItem* button);

private:
    void setupScene();
    void setupToolbar();
    void setupMenus();
    void refreshComPorts();

    CustomScene*   scene;
    QGraphicsView* view;

    QAction* loadImgAction;
    QAction* modeCheckAction;
    QAction* modeRunAction;
    QMenu*   comMenu;
};