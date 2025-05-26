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
    if (m_serial->isOpen()) m_serial->close();
}

void SerialPortModel::sendCommand(uint8_t command, uint8_t pin1, uint8_t pin2)
{
    App2Ctrl_Packet pkt;
    pkt.sof     = PROTOCOL_SOF;
    pkt.length  = sizeof(pkt) - offsetof(App2Ctrl_Packet, command) + 1; // command+pin1+pin2+checksum
    pkt.command = command;
    pkt.pin1    = pin1;
    pkt.pin2    = pin2;
    // checksum over sof, length, command, pin1, pin2
    pkt.checksum = calc_checksum(reinterpret_cast<uint8_t*>(&pkt), pkt.length + 2);
    m_serial->write(reinterpret_cast<const char*>(&pkt), sizeof(pkt));
}

void SerialPortModel::handleReadyRead()
{
    m_buffer.append(m_serial->readAll());
    processBuffer();
}

void SerialPortModel::processBuffer()
{
    // ищем SOF
    while (m_buffer.size() >= 3)
    {
        if (static_cast<uint8_t>(m_buffer.at(0)) != PROTOCOL_SOF)
        {
            m_buffer.remove(0, 1);
            continue;
        }
        uint8_t len = static_cast<uint8_t>(m_buffer.at(1));
        if (m_buffer.size() < len + 2) // ждём полный пакет
            return;
        QByteArray frame = m_buffer.mid(0, len + 2);
        m_buffer.remove(0, len + 2);

        // проверяем контрольную сумму
        uint8_t cs = calc_checksum(reinterpret_cast<const uint8_t*>(frame.constData()), len + 1);
        if (cs != static_cast<uint8_t>(frame.at(len + 1))) continue; // битый пакет

        // разбираем
        const Ctrl2App_Packet* rp = reinterpret_cast<const Ctrl2App_Packet*>(frame.constData());
        QVector<uint8_t>       leds;
        leds.reserve(15);
        for (int i = 0; i < 15; ++i)
            leds.append(rp->leds[i]);

        switch (rp->command)
        {
            case CMD_ACK_BTN_PRESSED:
                emit buttonPressed(rp->pin1, rp->pin2);
                break;
            case CMD_ACK_BTN_RELEASED:
                emit buttonReleased(rp->pin1, rp->pin2);
                break;
            case CMD_ACK_MODE_CHECK:
                emit modeCheckEntered();
                break;
            case CMD_ACK_MODE_RUN:
                emit modeRunEntered();
                break;
            default:
                emit statusReceived(rp->status, rp->pin1, rp->pin2, leds);
                break;
        }
    }
}