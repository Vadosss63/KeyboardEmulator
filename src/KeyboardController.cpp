#include "KeyboardController.h"

#include "SerialPortConnectionManager.h"
#include "SerialPortModel.h"

KeyboardController::KeyboardController(SerialPortModel* model, MainWindow* view, QObject* parent)
    : QObject(parent), m_model(model), m_view(view)
{
    // Model -> Controller slots
    connect(m_model, &SerialPortModel::statusReceived, this, &KeyboardController::onStatusReceived);
    connect(m_model, &SerialPortModel::receivedCommand, this, &KeyboardController::handleHwCmd);

    // View -> Controller
    connect(m_view, &MainWindow::appExecuteCommand, this, &KeyboardController::handleAppCommands);
    connect(m_view, &MainWindow::comPortSelected, [this](const QString& portName) { m_model->openPort(portName); });
    connect(m_view, &MainWindow::workModeChanged, this, &KeyboardController::handleWorkModeChanged);

    connManager = new SerialPortConnectionManager(m_model, this);

    connect(connManager,
            &SerialPortConnectionManager::connected,
            this,
            [this](const QString& port) { m_view->updateComPort(port); });

    connect(connManager,
            &SerialPortConnectionManager::disconnected,
            this,
            [this]() { m_view->updateComPort(QString("Не подключено")); });

    connect(connManager,
            &SerialPortConnectionManager::connectionError,
            this,
            [this](const QString& err) { m_view->updateComPort(QString("Ошибка: %1").arg(err)); });

    connManager->startAutoConnect();

    connect(
        m_view,
        &MainWindow::refreshComPortList,
        this,
        [this]()
        {
            m_view->updateComPort(QString("Поиск..."));
            connManager->stopAutoConnect();
            connManager->startAutoConnect();
        },
        Qt::QueuedConnection);
}

void KeyboardController::onStatusReceived(Pins pins, const QVector<Pins>& leds)
{
    m_view->updateStatus(pins, leds);
}

void KeyboardController::handleAppCommands(Command command, Pins pins)
{
    m_model->sendCommand(command, pins);
}

void KeyboardController::handleHwCmd(Command command)
{
    const bool isConfigCommand = (command == Command::ModeDiodeConfig || command == Command::ModeDiodeConfigDel ||
                                  command == Command::ModeDiodeClear);

    if (isConfigCommand)
    {
        m_view->workModeChanged(m_currentMode);
        return;
    }
}

void KeyboardController::handleWorkModeChanged(WorkMode mode)
{
    m_currentMode = mode;

    switch (mode)
    {
        case WorkMode::Work:
            m_model->sendCommand(Command::ModeRun);
            break;
        case WorkMode::Check:
            m_model->sendCommand(Command::ModeCheckKeyboard);
            break;
        case WorkMode::Modify:
            m_model->sendCommand(Command::ModeConfigure);
            break;
        default:
            break;
    }
}
