#include "SerialPortConnectionManager.h"

#include <QDebug>

SerialPortConnectionManager::SerialPortConnectionManager(SerialPortModel* portModel, QObject* parent)
    : QObject(parent), m_portModel(portModel)
{
    Q_ASSERT(m_portModel);

    connect(m_portModel, &SerialPortModel::echoReceived, this, &SerialPortConnectionManager::onEchoReceived);

    connect(m_portModel, &SerialPortModel::portError, this, &SerialPortConnectionManager::onPortError);

    m_heartbeatTimer.setSingleShot(false);
    connect(&m_heartbeatTimer, &QTimer::timeout, this, &SerialPortConnectionManager::onHeartbeatTimeout);

    m_responseTimer.setSingleShot(true);
    connect(&m_responseTimer, &QTimer::timeout, this, &SerialPortConnectionManager::onResponseTimeout);
}

bool SerialPortConnectionManager::isConnected() const
{
    return m_state == State::Connected;
}

QString SerialPortConnectionManager::currentPortName() const
{
    return m_currentPortName;
}

void SerialPortConnectionManager::setHeartbeatInterval(int ms)
{
    m_heartbeatIntervalMs = ms;
    if (m_heartbeatTimer.isActive())
    {
        m_heartbeatTimer.start(m_heartbeatIntervalMs);
    }
}

bool SerialPortConnectionManager::sendCommand(Command cmd, Pins pins)
{
    if (!isConnected())
    {
        return false;
    }

    m_portModel->sendCommand(cmd, pins);
    return true;
}

void SerialPortConnectionManager::startAutoConnect()
{
    if (m_state != State::Disconnected)
    {
        return;
    }

    probePorts();
}

void SerialPortConnectionManager::stopAutoConnect()
{
    if (m_heartbeatTimer.isActive())
    {
        m_heartbeatTimer.stop();
    }
    m_responseTimer.stop();
    m_portModel->closePort();
    m_state = State::Disconnected;
    m_currentPortName.clear();
    emit disconnected();
}

void SerialPortConnectionManager::probePorts()
{
    m_ports.clear();

    for (const auto& info : QSerialPortInfo::availablePorts())
    {
        const QString portName = info.portName();

        if (portName.startsWith("COM") || portName.startsWith("ttyUSB"))
        {
            m_ports.append(info);
        }
    }

    m_ports.append(QSerialPortInfo("/tmp/ttyV1")); // for testing on Linux

    m_currentPortIndex = -1;

    m_state = State::Probing;

    if (m_ports.isEmpty())
    {
        emit connectionError(tr("Нет доступных COM-портов"));
        m_state = State::Disconnected;
        return;
    }

    probeNextPort();
}

void SerialPortConnectionManager::probeNextPort()
{
    m_responseTimer.stop();
    m_waitingEchoReply = false;

    m_portModel->closePort();
    m_currentPortName.clear();

    ++m_currentPortIndex;
    if (m_currentPortIndex >= m_ports.size())
    {
        m_state = State::Disconnected;
        emit connectionError(tr("Не удалось найти устройство"));
        emit disconnected();
        return;
    }

    const auto& info = m_ports[m_currentPortIndex];
    openAndTestPort(info);
}

void SerialPortConnectionManager::openAndTestPort(const QSerialPortInfo& info)
{
    const QString portName = (info.portName().isEmpty() ? "/tmp/ttyV1" : info.portName());
    qDebug() << "Selected COM port:" << portName;

    if (!m_portModel->openPort(portName))
    {
        probeNextPort();
        return;
    }

    m_currentPortName = portName;

    m_waitingEchoReply = true;
    m_portModel->clearBuffer();
    m_portModel->sendCommand(Command::Echo);
    m_responseTimer.start(m_responseTimeoutMs);
}

void SerialPortConnectionManager::onEchoReceived()
{
    if (!m_waitingEchoReply)
    {
        return;
    }

    m_waitingEchoReply = false;
    m_responseTimer.stop();

    if (m_state == State::Probing)
    {
        handleConnectSuccess();
    }
}

void SerialPortConnectionManager::handleConnectSuccess()
{
    m_state = State::Connected;
    emit connected(m_currentPortName);

    m_heartbeatTimer.start(m_heartbeatIntervalMs);
}

void SerialPortConnectionManager::onPortError(const QString& description)
{
    emit connectionError(description);
    //handleDisconnect(/*restartAutoConnect=*/true);
}

void SerialPortConnectionManager::onHeartbeatTimeout()
{
    if (!isConnected())
    {
        return;
    }

    if (m_waitingEchoReply)
    {
        return;
    }

    m_waitingEchoReply = true;
    m_portModel->sendCommand(Command::Echo);
    m_responseTimer.start(m_responseTimeoutMs);
}

void SerialPortConnectionManager::onResponseTimeout()
{
    if (m_state == State::Probing)
    {
        probeNextPort();
        return;
    }

    if (m_state == State::Connected)
    {
        handleDisconnect(/*restartAutoConnect=*/true);
    }
}

void SerialPortConnectionManager::handleDisconnect(bool restartAutoConnect)
{
    m_heartbeatTimer.stop();
    m_responseTimer.stop();
    m_waitingEchoReply = false;

    if (m_state == State::Connected)
    {
        emit disconnected();
    }

    m_state = State::Disconnected;
    m_portModel->closePort();
    m_currentPortName.clear();

    if (restartAutoConnect)
    {
        startAutoConnect();
    }
}
