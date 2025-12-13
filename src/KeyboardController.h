#pragma once

#include <QObject>
#include <QVector>

#include "KeyboardControllerProtocol.h"
#include "SerialPortConnectionManager.h"
#include "WorkMode.h"

class MainWindow;

class SerialPortModel;

class KeyboardController : public QObject
{
    Q_OBJECT
public:
    KeyboardController(SerialPortModel* model, MainWindow* view, QObject* parent = nullptr);

private slots:
    void onStatusReceived(Pins pins, const QVector<Pins>& leds);
    void handleHwCmd(Command command);

    void handleAppCommands(Command command, Pins pins);
    void handleWorkModeChanged(WorkMode mode);

private:
    SerialPortModel*             m_model{nullptr};
    SerialPortConnectionManager* connManager{nullptr};
    MainWindow*                  m_view{nullptr};

    WorkMode m_currentMode{WorkMode::Modify};
};
