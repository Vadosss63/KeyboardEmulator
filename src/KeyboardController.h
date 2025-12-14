#pragma once

#include <QObject>
#include <QString>
#include <QVector>

#include "KeyboardControllerProtocol.h"
#include "SerialPortConnectionManager.h"
#include "WorkMode.h"

class MainWindow;

class SerialPortModel;
class DiodeSyncService;

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
    void handleComPortSelected(const QString& portName);
    void handleConnectionEstablished(const QString& port);
    void handleConnectionLost();
    void handleConnectionError(const QString& err);
    void handleRefreshComPortList();

private:
    SerialPortModel*             m_model{nullptr};
    SerialPortConnectionManager* connManager{nullptr};
    MainWindow*                  m_view{nullptr};
    DiodeSyncService*            m_diodeSync{nullptr};

    WorkMode m_currentMode{WorkMode::Modify};
};
