#include "SerialPortModel.h"

SerialPortModel::SerialPortModel(QObject* parent) : QObject(parent), m_serial(new QSerialPort(this))
{
    connect(m_serial, &QSerialPort::readyRead, this, &SerialPortModel::handleReadyRead);
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
        m_serial->close();
    }
}

void SerialPortModel::sendCommand(uint8_t command, uint8_t pin1, uint8_t pin2)
{
    App2Ctrl_Packet pkt;
    pkt.sof     = PROTOCOL_SOF;
    pkt.length  = sizeof(pkt) - offsetof(App2Ctrl_Packet, command); // command+pin1+pin2+checksum
    pkt.command = command;
    pkt.pin1    = pin1;
    pkt.pin2    = pin2;
    // SOF + Length + Command + Pin1 + Pin2
    const size_t len_before_checksum = offsetof(App2Ctrl_Packet, checksum);

    pkt.checksum = calc_checksum(reinterpret_cast<const uint8_t*>(&pkt), len_before_checksum);
    m_serial->write(reinterpret_cast<const char*>(&pkt), sizeof(pkt));
}

void SerialPortModel::handleReadyRead()
{
    m_buffer.append(m_serial->readAll());
    processBuffer();
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
    const Ctrl2App_Packet* rp = reinterpret_cast<const Ctrl2App_Packet*>(frame.constData());

    QVector<uint8_t> leds;
    leds.reserve(15);
    for (int i = 0; i < 15; ++i)
    {
        leds.append(rp->leds[i]);
    }
    emit statusReceived(rp->pin1, rp->pin2, leds);
}
