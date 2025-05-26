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
    void sendCommand(uint8_t command, uint8_t pin1 = 0, uint8_t pin2 = 0);

signals:
    // Ctrl -> App
    void buttonPressed(uint8_t pin1, uint8_t pin2);
    void buttonReleased(uint8_t pin1, uint8_t pin2);
    void modeCheckEntered();
    void modeRunEntered();
    void statusReceived(uint8_t status, uint8_t pin1, uint8_t pin2, const QVector<uint8_t>& leds);

private slots:
    void handleReadyRead();

private:
    void processBuffer();

    QSerialPort* m_serial;
    QByteArray   m_buffer;
};
