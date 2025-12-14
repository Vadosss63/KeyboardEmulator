#pragma once

#include <QObject>
#include <QSerialPortInfo>
#include <QSet>
#include <QTimer>

#include "SerialPortModel.h"

class SerialPortConnectionManager : public QObject
{
    Q_OBJECT

    enum class State
    {
        Disconnected,
        Probing,
        Connected
    };

public:
    explicit SerialPortConnectionManager(SerialPortModel* portModel, QObject* parent = nullptr);

    void startAutoConnect();
    void stopAutoConnect();

    bool    isConnected() const;
    QString currentPortName() const;

    void setHeartbeatInterval(int ms);

    bool sendCommand(Command cmd);

signals:
    void connected(const QString& portName);
    void disconnected();
    void connectionError(const QString& message);

private slots:
    void onEchoReceived();
    void onPortError(const QString& description);
    void onHeartbeatTimeout();
    void onResponseTimeout();
    void monitorAvailablePorts();

private:
    void probePorts();
    void probeNextPort();
    void openAndTestPort(const QSerialPortInfo& info);
    void handleConnectSuccess();
    void handleDisconnect(bool restartAutoConnect = true);
    void updatePortMonitorState();

    QSet<QString> collectPortNames() const;

    SerialPortModel* m_portModel{nullptr};

    State m_state{State::Disconnected};

    QList<QSerialPortInfo> m_ports{};

    int m_currentPortIndex{-1};

    QTimer m_heartbeatTimer{};
    QTimer m_responseTimer{};
    QTimer m_portMonitorTimer{};

    int  m_heartbeatIntervalMs{5000};
    int  m_responseTimeoutMs{1000};
    bool m_waitingEchoReply{false};

    QString       m_currentPortName{};
    QSet<QString> m_lastObservedPorts{};

    static constexpr auto testPort{"/tmp/ttyV1"};
};
