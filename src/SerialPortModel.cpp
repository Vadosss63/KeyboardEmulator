#include "SerialPortModel.h"

#include "logger.h"

SerialPortModel::SerialPortModel(QObject* parent) : QObject(parent), m_serial(new QSerialPort(this))
{
    connect(m_serial, &QSerialPort::readyRead, this, &SerialPortModel::handleReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred, this, &SerialPortModel::handleError);

    m_commandDelayTimer.setSingleShot(true);
    connect(&m_commandDelayTimer, &QTimer::timeout, this, &SerialPortModel::handleQueueDelayTimeout);

    m_ackTimeoutTimer.setSingleShot(true);
    connect(&m_ackTimeoutTimer, &QTimer::timeout, this, &SerialPortModel::handleAckTimeout);
}

SerialPortModel::~SerialPortModel()
{
    closePort();
}

bool SerialPortModel::openPort(const QString& portName, int baudRate)
{
    if (m_serial->isOpen())
    {
        closePort();
    }

    m_serial->setPortName(portName);
    LOG_INFO << "Opening COM port " << portName.toStdString() << " at " << baudRate << " baud" << std::endl;
    m_serial->setBaudRate(baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    const bool opened = m_serial->open(QIODevice::ReadWrite);
    if (!opened)
    {
        LOG_ERR << "Failed to open port " << portName.toStdString() << ": " << m_serial->errorString().toStdString()
                << std::endl;
    }
    else
    {
        LOG_INFO << "Port " << portName.toStdString() << " opened successfully" << std::endl;
        processQueue();
    }
    return opened;
}

void SerialPortModel::closePort()
{
    if (m_serial->isOpen())
    {
        LOG_INFO << "Closing COM port " << m_serial->portName().toStdString() << std::endl;
        m_serial->close();
        clearCommandQueue();
    }
}

void SerialPortModel::clearBuffer()
{
    m_buffer.append(m_serial->readAll());
    m_buffer.clear();
}

void SerialPortModel::sendCommand(Command command, Pins pins)
{
    enqueueCommand(command, pins);
}

void SerialPortModel::sendCommand(Command command)
{
    enqueueCommand(command);
}

void SerialPortModel::handleReadyRead()
{
    m_buffer.append(m_serial->readAll());
    processBuffer();
}

void SerialPortModel::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
    {
        return;
    }

    LOG_ERR << "Serial port error: " << m_serial->errorString().toStdString() << std::endl;
    emit portError(m_serial->errorString());
}

void SerialPortModel::processBuffer()
{
    // find SOF
    while (m_buffer.size() >= 3)
    {
        if (static_cast<uint8_t>(m_buffer.at(0)) != PROTOCOL_SOF)
        {
            m_buffer.remove(0, 1);
            continue;
        }
        uint8_t len = static_cast<uint8_t>(m_buffer.at(1));
        if (m_buffer.size() < len + 2) // wait for full packet
        {
            return;
        }
        QByteArray frame = m_buffer.mid(0, len + 2);
        m_buffer.remove(0, len + 2);

        uint8_t cs = calc_checksum(reinterpret_cast<const uint8_t*>(frame.constData()), len + 1);
        if (cs != static_cast<uint8_t>(frame.at(len + 1)))
        {
            continue;
        }

        parsePacket(frame);
    }
}

void SerialPortModel::parsePacket(QByteArray& frame)
{
    const Packet* rp = reinterpret_cast<const Packet*>(frame.constData());

    if (rp->command == Command::Echo)
    {
        handleCommandAck(Command::Echo);
        emit echoReceived();
        return;
    }

    if (rp->command == Command::StatusUpdate)
    {
        const StatusPayload* status = reinterpret_cast<const StatusPayload*>(rp->payload);

        QVector<Pins> leds;
        leds.reserve(status->leds_num);
        for (int i = 0; i < status->leds_num; ++i)
        {
            leds.append(status->leds[i]);
        }
        emit statusReceived(status->pins, leds);
        return;
    }

    handleCommandAck(rp->command);
    emit receivedCommand(rp->command);
}

void SerialPortModel::enqueueCommand(Command command, Pins pins)
{
    m_commandQueue.enqueue({command, build_packet_for_cmd(command, pins)});
    processQueue();
}

void SerialPortModel::enqueueCommand(Command command)
{
    m_commandQueue.enqueue({command, build_packet_for_cmd(command)});
    processQueue();
}

void SerialPortModel::processQueue()
{
    if (!m_serial || !m_serial->isOpen())
    {
        return;
    }

    if (m_waitingForAck)
    {
        return;
    }

    if (m_commandDelayTimer.isActive())
    {
        return;
    }

    while (!m_commandQueue.isEmpty())
    {
        const QueuedCommand cmd = m_commandQueue.dequeue();

        const std::vector<uint8_t>& packet = cmd.payload;

        m_serial->write(reinterpret_cast<const char*>(packet.data()), packet.size());

        if (requiresAcknowledgement(cmd.command))
        {
            m_waitingForAck = true;
            m_expectedAck   = cmd.command;
            m_ackTimeoutTimer.start(kAckTimeoutMs);
            break;
        }

        if (kInterCommandDelayMs > 0)
        {
            m_commandDelayTimer.start(kInterCommandDelayMs);
            break;
        }
    }
}

void SerialPortModel::handleCommandAck(Command command)
{
    if (!m_waitingForAck)
    {
        return;
    }

    if (command != m_expectedAck)
    {
        return;
    }

    m_waitingForAck = false;
    m_expectedAck   = Command::None;
    m_ackTimeoutTimer.stop();

    if (kInterCommandDelayMs > 0)
    {
        m_commandDelayTimer.start(kInterCommandDelayMs);
        return;
    }

    processQueue();
}

void SerialPortModel::handleQueueDelayTimeout()
{
    processQueue();
}

void SerialPortModel::handleAckTimeout()
{
    if (!m_waitingForAck)
    {
        return;
    }

    LOG_WRN << "Ack timeout for command " << static_cast<int>(m_expectedAck) << std::endl;
    m_waitingForAck = false;
    m_expectedAck   = Command::None;

    if (kInterCommandDelayMs > 0)
    {
        m_commandDelayTimer.start(kInterCommandDelayMs);
        return;
    }

    processQueue();
}

void SerialPortModel::clearCommandQueue()
{
    m_commandQueue.clear();
    m_commandDelayTimer.stop();
    m_ackTimeoutTimer.stop();
    m_waitingForAck = false;
    m_expectedAck   = Command::None;
}

bool SerialPortModel::requiresAcknowledgement(Command command)
{
    switch (command)
    {
        case Command::Echo:
        case Command::ModeDiodeConfig:
        case Command::ModeDiodeConfigDel:
        case Command::ModeDiodeClear:
            return true;
        default:
            return false;
    }
}
