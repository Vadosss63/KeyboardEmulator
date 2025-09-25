#pragma once

#include <QByteArray>
#include <QObject>
#include <QSerialPort>
#include <QVector>

#include "KeyboardControllerProtocol.h"

class MainWindow;

class SerialPortModel;

enum class WorkMode;

class KeyboardController : public QObject
{
    Q_OBJECT
public:
    KeyboardController(SerialPortModel* model, MainWindow* view, QObject* parent = nullptr);

private slots:
    void onStatusReceived(uint8_t pin1, uint8_t pin2, const QVector<uint8_t>& leds);

    // signals from View
    void handleAppButtonPressed(uint8_t pin1, uint8_t pin2);
    void handleAppButtonReleased(uint8_t pin1, uint8_t pin2);
    void handleAppDiodePressed(uint8_t pin1, uint8_t pin2);
    void handleAppDiodeReleased(uint8_t pin1, uint8_t pin2);
    void handleAppEnterModeCheck();
    void handleAppEnterModeRun();
    void handleAppEnterModeConfigure();
    void handleWorkModeChanged(WorkMode mode);

private:
    SerialPortModel* m_model;
    MainWindow*      m_view;
};
