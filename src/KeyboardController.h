#pragma once

#include <QByteArray>
#include <QObject>
#include <QSerialPort>
#include <QVector>

#include "KeyboardControllerProtocol.h"
#include "MainWindow.h"

class SerialPortModel;

class KeyboardController : public QObject
{
    Q_OBJECT
public:
    KeyboardController(SerialPortModel* model, MainWindow* view, QObject* parent = nullptr);

private slots:
    void onStatusReceived(Pins pins, const QVector<Pins>& leds);

    // signals from View
    void handleAppButtonPressed(Pins pins);
    void handleAppButtonReleased(Pins pins);
    void handleAppDiodePressed(Pins pins);
    void handleAppDiodeReleased(Pins pins);
    void handleAppDiodePinConfigChanged(Pins newPins);

    void handleWorkModeChanged(WorkMode mode);

private:
    void handleAppEnterModeCheck();
    void handleAppEnterModeRun();
    void handleAppEnterModeConfigure();

    SerialPortModel* m_model;
    MainWindow*      m_view;

    WorkMode m_currentMode{WorkMode::Modify};

    bool isConfUpdateInProgress{false};
};
