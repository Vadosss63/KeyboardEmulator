#include "KeyboardController.h"

#include "MainWindow.h"
#include "SerialPortConnectionManager.h"
#include "SerialPortModel.h"
#include "logger.h"

namespace
{
const char* toString(WorkMode mode)
{
    switch (mode)
    {
        case WorkMode::Work:
            return "Work";
        case WorkMode::Check:
            return "Check";
        case WorkMode::Modify:
            return "Modify";
        case WorkMode::DiodeConf:
            return "DiodeConf";
        default:
            return "Unknown";
    }
}

const char* toString(Command command)
{
    switch (command)
    {
        case Command::ModeRun:
            return "ModeRun";
        case Command::ModeCheckKeyboard:
            return "ModeCheckKeyboard";
        case Command::ModeConfigure:
            return "ModeConfigure";
        case Command::ModeDiodeConfig:
            return "ModeDiodeConfig";
        case Command::ModeDiodeConfigDel:
            return "ModeDiodeConfigDel";
        case Command::ModeDiodeClear:
            return "ModeDiodeClear";
        case Command::ButtonPressed:
            return "ButtonPressed";
        case Command::ButtonReleased:
            return "ButtonReleased";
        case Command::DiodePressed:
            return "DiodePressed";
        case Command::DiodeReleased:
            return "DiodeReleased";
        case Command::Echo:
            return "Echo";
        case Command::StatusUpdate:
            return "StatusUpdate";
        default:
            return "Unknown";
    }
}
} // namespace

KeyboardController::KeyboardController(SerialPortModel* model, MainWindow* view, QObject* parent)
    : QObject(parent), m_model(model), m_view(view)
{
    // Model -> Controller slots
    connect(m_model, &SerialPortModel::statusReceived, this, &KeyboardController::onStatusReceived);
    connect(m_model, &SerialPortModel::receivedCommand, this, &KeyboardController::handleHwCmd);

    // View -> Controller
    connect(m_view, &MainWindow::appExecuteCommand, this, &KeyboardController::handleAppCommands);
    connect(
        m_view, &MainWindow::comPortSelected, this, [this](const QString& portName) { m_model->openPort(portName); });
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
    LOG_INFO << "App command " << toString(command) << " P1=" << static_cast<int>(pins.pin1)
             << " P2=" << static_cast<int>(pins.pin2) << std::endl;
    m_model->sendCommand(command, pins);
}

void KeyboardController::handleHwCmd(Command command)
{
    LOG_INFO << "Hardware command received: " << toString(command) << std::endl;
    const bool isConfigCommand = (command == Command::ModeDiodeConfig || command == Command::ModeDiodeConfigDel ||
                                  command == Command::ModeDiodeClear);

    if (isConfigCommand)
    {
        emit m_view->workModeChanged(m_currentMode);
        return;
    }
}

void KeyboardController::handleWorkModeChanged(WorkMode mode)
{
    LOG_INFO << "Switching to work mode " << toString(mode) << std::endl;
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
