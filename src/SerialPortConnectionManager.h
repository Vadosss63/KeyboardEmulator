#pragma once

#include <QObject>
#include <QSerialPortInfo>
#include <QTimer>

#include "SerialPortModel.h"

class SerialPortConnectionManager : public QObject
{
    Q_OBJECT
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

private:
    enum class State
    {
        Disconnected,
        Probing,
        Connected
    };

    void probePorts();
    void probeNextPort();
    void openAndTestPort(const QSerialPortInfo& info);
    void handleConnectSuccess();
    void handleDisconnect(bool restartAutoConnect = true);

    SerialPortModel* m_portModel{nullptr};

    State m_state{State::Disconnected};

    QList<QSerialPortInfo> m_ports{};

    int m_currentPortIndex{-1};

    QTimer m_heartbeatTimer{};
    QTimer m_responseTimer{};

    int  m_heartbeatIntervalMs{5000};
    int  m_responseTimeoutMs{1000};
    bool m_waitingEchoReply{false};

    QString m_currentPortName{};
};
