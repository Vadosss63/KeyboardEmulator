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

    void handleAppCommands(Command command, Pins pins);

    void handleWorkModeChanged(WorkMode mode);

private:
    SerialPortModel* m_model;
    MainWindow*      m_view;

    WorkMode m_currentMode{WorkMode::Modify};

    bool isConfUpdateInProgress{false};
};
