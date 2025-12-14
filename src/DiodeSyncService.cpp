#include "DiodeSyncService.h"

#include "SerialPortModel.h"

DiodeSyncService::DiodeSyncService(SerialPortModel* model, QObject* parent) : QObject(parent), m_model(model) {}

void DiodeSyncService::reset(const QVector<Pins>& diodes)
{
    m_diodeStates.clear();
    for (const Pins& pins : diodes)
    {
        m_diodeStates.insert(keyFor(pins), pins);
    }

    m_fullSyncRequired = true;
    sendFullState();
}

void DiodeSyncService::upsert(Pins pins)
{
    m_diodeStates.insert(keyFor(pins), pins);

    if (m_connected && !m_fullSyncRequired)
    {
        sendCommand(Command::ModeDiodeConfig, pins);
    }
}

void DiodeSyncService::remove(Pins pins)
{
    m_diodeStates.remove(keyFor(pins));

    if (m_connected && !m_fullSyncRequired)
    {
        sendCommand(Command::ModeDiodeConfigDel, pins);
    }
}

void DiodeSyncService::handleConnectionEstablished()
{
    m_connected        = true;
    m_fullSyncRequired = true;
    sendFullState();
}

void DiodeSyncService::handleConnectionLost()
{
    m_connected        = false;
    m_fullSyncRequired = true;
}

void DiodeSyncService::sendFullState()
{
    if (!m_connected || !m_model || !m_fullSyncRequired)
    {
        return;
    }

    sendCommand(Command::ModeDiodeClear);
    for (const Pins& pins : qAsConst(m_diodeStates))
    {
        sendCommand(Command::ModeDiodeConfig, pins);
    }

    m_fullSyncRequired = false;
}

void DiodeSyncService::sendCommand(Command command, Pins pins) const
{
    if (!m_model)
    {
        return;
    }

    m_model->sendCommand(command, pins);
}

void DiodeSyncService::sendCommand(Command command) const
{
    if (!m_model)
    {
        return;
    }

    m_model->sendCommand(command);
}

QString DiodeSyncService::keyFor(Pins pins)
{
    return QString::number(pins.pin1) + QLatin1Char(':') + QString::number(pins.pin2);
}
