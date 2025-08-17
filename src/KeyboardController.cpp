#include "KeyboardController.h"

#include <QSerialPortInfo>

#include "MainWindow.h"
#include "SerialPortModel.h"

KeyboardController::KeyboardController(SerialPortModel* model, MainWindow* view, QObject* parent)
    : QObject(parent), m_model(model), m_view(view)
{
    // Model -> Controller slots
    connect(m_model, &SerialPortModel::buttonPressed, this, &KeyboardController::onButtonPressed);
    connect(m_model, &SerialPortModel::buttonReleased, this, &KeyboardController::onButtonReleased);
    connect(m_model, &SerialPortModel::modeCheckEntered, this, &KeyboardController::onModeCheckEntered);
    connect(m_model, &SerialPortModel::modeRunEntered, this, &KeyboardController::onModeRunEntered);
    connect(m_model, &SerialPortModel::statusReceived, this, &KeyboardController::onStatusReceived);

    // View -> Controller
    connect(m_view, &MainWindow::appButtonPressed, this, &KeyboardController::handleAppButtonPressed);
    connect(m_view, &MainWindow::appButtonReleased, this, &KeyboardController::handleAppButtonReleased);
    connect(m_view, &MainWindow::appEnterModeCheck, this, &KeyboardController::handleAppEnterModeCheck);
    connect(m_view, &MainWindow::appEnterModeRun, this, &KeyboardController::handleAppEnterModeRun);

    connect(m_view, &MainWindow::comPortSelected, [this](const QString& portName) { m_model->openPort(portName); });
}

// Handlers Model -> update View
void KeyboardController::onButtonPressed(uint8_t pin1, uint8_t pin2)
{
    m_view->markButtonPressed(pin1, pin2);
}
void KeyboardController::onButtonReleased(uint8_t pin1, uint8_t pin2)
{
    m_view->markButtonReleased(pin1, pin2);
}
void KeyboardController::onModeCheckEntered()
{
    m_view->enterCheckMode();
}
void KeyboardController::onModeRunEntered()
{
    m_view->enterRunMode();
}
void KeyboardController::onStatusReceived(uint8_t status, uint8_t pin1, uint8_t pin2, const QVector<uint8_t>& leds)
{
    m_view->updateStatus(status, pin1, pin2, leds);
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
void KeyboardController::handleAppEnterModeCheck()
{
    m_model->sendCommand(CMD_MODE_CHECK_KEYBOARD);
}
void KeyboardController::handleAppEnterModeRun()
{
    m_model->sendCommand(CMD_MODE_RUN);
}
