#include "KeyboardController.h"

#include <QSerialPortInfo>

#include "SerialPortModel.h"

KeyboardController::KeyboardController(SerialPortModel* model, MainWindow* view, QObject* parent)
    : QObject(parent), m_model(model), m_view(view)
{
    // Model -> Controller slots
    connect(m_model, &SerialPortModel::statusReceived, this, &KeyboardController::onStatusReceived);

    // View -> Controller
    connect(m_view, &MainWindow::appExecuteCommand, this, &KeyboardController::handleAppCommands);

    connect(m_view, &MainWindow::comPortSelected, [this](const QString& portName) { m_model->openPort(portName); });
    connect(m_view, &MainWindow::workModeChanged, this, &KeyboardController::handleWorkModeChanged);
}

void KeyboardController::onStatusReceived(Pins pins, const QVector<Pins>& leds)
{
    if (isConfUpdateInProgress)
    {
        isConfUpdateInProgress = false;
        m_view->workModeChanged(m_currentMode);
        return;
    }

    m_view->updateStatus(pins, leds);
}

void KeyboardController::handleAppCommands(Command command, Pins pins)
{
    isConfUpdateInProgress = (command == Command::ModeDiodeConfig || command == Command::ModeDiodeConfigDel ||
                              command == Command::ModeDiodeClear);
    m_model->sendCommand(command, pins);
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
