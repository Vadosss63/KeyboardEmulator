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

void KeyboardController::onStatusReceived(uint8_t pin1, uint8_t pin2, const QVector<uint8_t>& leds)
{
    if (isConfUpdateInProgress)
    {
        isConfUpdateInProgress = false;
        m_view->workModeChanged(m_currentMode);
        return;
    }

    m_view->updateStatus(pin1, pin2, leds);
}

// Handlers View -> send to Model
void KeyboardController::handleAppButtonPressed(uint8_t pin1, uint8_t pin2)
{
    m_model->sendCommand(CMD_BTN_PRESSED, pin1, pin2);
}
void KeyboardController::handleAppButtonReleased(uint8_t pin1, uint8_t pin2)
{
    m_model->sendCommand(CMD_BTN_RELEASED, pin1, pin2);
}

void KeyboardController::handleAppDiodePressed(uint8_t pin1, uint8_t pin2)
{
    m_model->sendCommand(CMD_DIODE_PRESSED, pin1, pin2);
}

void KeyboardController::handleAppDiodeReleased(uint8_t pin1, uint8_t pin2)
{
    m_model->sendCommand(CMD_DIODE_RELEASED, pin1, pin2);
}

void KeyboardController::handleAppDiodePinConfigChanged(uint8_t newPin1, uint8_t newPin2)
{
    m_model->sendCommand(CMD_MODE_DIODE_CONF, newPin1, newPin2);
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
