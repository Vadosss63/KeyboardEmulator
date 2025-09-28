#pragma once

#include <QByteArray>
#include <QObject>
#include <QSerialPort>
#include <QVector>

#include "KeyboardControllerProtocol.h"

class MainWindow;

class SerialPortModel : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortModel(QObject* parent = nullptr);
    ~SerialPortModel();

    bool openPort(const QString& portName, int baudRate = QSerialPort::Baud115200);
    void closePort();

public slots:
    // App -> Controller
    void sendCommand(uint8_t command, Pins pins = {0, 0});

signals:
    // Ctrl -> App
    void statusReceived(Pins pins, const QVector<Pins>& leds);

private slots:
    void handleReadyRead();

private:
    void processBuffer();

    void parsePacket(QByteArray& frame);

    QSerialPort* m_serial;
    QByteArray   m_buffer;
};
