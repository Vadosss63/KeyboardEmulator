#pragma once

#include <QByteArray>
#include <QObject>
#include <QQueue>
#include <QSerialPort>
#include <QTimer>
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

public:
    void sendCommand(Command command, Pins pins);
    void sendCommand(Command command);

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

    void enqueueCommand(Command command, Pins pins);
    void enqueueCommand(Command command);
    void processQueue();
    void handleCommandAck(Command command);
    void handleQueueDelayTimeout();
    void handleAckTimeout();
    void clearCommandQueue();

    static bool requiresAcknowledgement(Command command);

    struct QueuedCommand
    {
        Command command{Command::None};

        std::vector<uint8_t> payload;
    };

    QSerialPort* m_serial;
    QByteArray   m_buffer;

    QQueue<QueuedCommand> m_commandQueue;

    QTimer  m_commandDelayTimer;
    QTimer  m_ackTimeoutTimer;
    bool    m_waitingForAck{false};
    Command m_expectedAck{Command::None};

    static constexpr int kInterCommandDelayMs = 5;
    static constexpr int kAckTimeoutMs        = 200;
};
