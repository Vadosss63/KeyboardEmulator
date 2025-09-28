#include "KeyboardController.h"

#include <QSerialPortInfo>

#include "SerialPortModel.h"

KeyboardController::KeyboardController(SerialPortModel* model, MainWindow* view, QObject* parent)
    : QObject(parent), m_model(model), m_view(view)
{
    // Model -> Controller slots
    connect(m_model, &SerialPortModel::statusReceived, this, &KeyboardController::onStatusReceived);

    // View -> Controller
    connect(m_view, &MainWindow::appButtonPressed, this, &KeyboardController::handleAppButtonPressed);
    connect(m_view, &MainWindow::appButtonReleased, this, &KeyboardController::handleAppButtonReleased);
    connect(m_view, &MainWindow::appDiodePressed, this, &KeyboardController::handleAppDiodePressed);
    connect(m_view, &MainWindow::appDiodeReleased, this, &KeyboardController::handleAppDiodeReleased);
    connect(m_view, &MainWindow::appDiodePinConfigChanged, this, &KeyboardController::handleAppDiodePinConfigChanged);

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

// Handlers View -> send to Model
void KeyboardController::handleAppButtonPressed(Pins pins)
{
    m_model->sendCommand(CMD_BTN_PRESSED, pins);
}
void KeyboardController::handleAppButtonReleased(Pins pins)
{
    m_model->sendCommand(CMD_BTN_RELEASED, pins);
}

void KeyboardController::handleAppDiodePressed(Pins pins)
{
    m_model->sendCommand(CMD_DIODE_PRESSED, pins);
}

void KeyboardController::handleAppDiodeReleased(Pins pins)
{
    m_model->sendCommand(CMD_DIODE_RELEASED, pins);
}

void KeyboardController::handleAppDiodePinConfigChanged(Pins newPins)
{
    m_model->sendCommand(CMD_MODE_DIODE_CONF, newPins);
    bool isConfUpdateInProgress = true;
}

void KeyboardController::handleAppEnterModeCheck()
{
    m_model->sendCommand(CMD_MODE_CHECK_KEYBOARD);
}
void KeyboardController::handleAppEnterModeRun()
{
    m_model->sendCommand(CMD_MODE_RUN);
}

void KeyboardController::handleAppEnterModeConfigure()
{
    m_model->sendCommand(CMD_MODE_CONFIGURE);
}

void KeyboardController::handleWorkModeChanged(WorkMode mode)
{
    m_currentMode = mode;

    switch (mode)
    {
        case WorkMode::Work:
            handleAppEnterModeRun();
            break;
        case WorkMode::Check:
            handleAppEnterModeCheck();
            break;
        case WorkMode::Modify:
            handleAppEnterModeConfigure();
            break;
        default:
            break;
    }
}
