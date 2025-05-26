#pragma once

#include <QByteArray>
#include <QObject>
#include <QSerialPort>
#include <QVector>

#include "KeyboardControllerProtocol.h"

class MainWindow;

class SerialPortModel;

class KeyboardController : public QObject
{
    Q_OBJECT
public:
    KeyboardController(SerialPortModel* model, MainWindow* view, QObject* parent = nullptr);

private slots:
    // сигналы от модели
    void onButtonPressed(uint8_t pin1, uint8_t pin2);
    void onButtonReleased(uint8_t pin1, uint8_t pin2);
    void onModeCheckEntered();
    void onModeRunEntered();
    void onStatusReceived(uint8_t status, uint8_t pin1, uint8_t pin2, const QVector<uint8_t>& leds);

    // сигналы от View (MainWindow)
    void handleAppButtonPressed(uint8_t pin1, uint8_t pin2);
    void handleAppButtonReleased(uint8_t pin1, uint8_t pin2);
    void handleAppEnterModeCheck();
    void handleAppEnterModeRun();

private:
    SerialPortModel* m_model;
    MainWindow*      m_view;
};
