#include "SerialPortModel.h"

#include <QDebug>

SerialPortModel::SerialPortModel(QObject* parent) : QObject(parent), m_serial(new QSerialPort(this))
{
    connect(m_serial, &QSerialPort::readyRead, this, &SerialPortModel::handleReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred, this, &SerialPortModel::handleError);
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
    m_serial->setBaudRate(baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    return m_serial->open(QIODevice::ReadWrite);
}

void SerialPortModel::closePort()
{
    if (m_serial->isOpen())
    {
        qDebug() << "Closing COM port:" << m_serial->portName();
        m_serial->close();
    }
}

void SerialPortModel::clearBuffer()
{
    m_buffer.append(m_serial->readAll());
    m_buffer.clear();
}

void SerialPortModel::sendCommand(Command command, Pins pins)
{
    auto packet = build_packet_for_cmd(command, pins);
    m_serial->write(reinterpret_cast<const char*>(packet.data()), packet.size());
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

    emit receivedCommand(rp->command);
}
