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

    void clearBuffer();

public slots:
    void sendCommand(Command command, Pins pins = {0, 0});

signals:
    void statusReceived(Pins pins, const QVector<Pins>& leds);

    void receivedCommand(Command command);

    void echoReceived();

    void portError(const QString& description);

private slots:
    void handleReadyRead();

    void handleError(QSerialPort::SerialPortError error);

private:
    void processBuffer();

    void parsePacket(QByteArray& frame);

    QSerialPort* m_serial;
    QByteArray   m_buffer;
};
