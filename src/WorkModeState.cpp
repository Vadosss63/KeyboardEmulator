#include "WorkModeState.h"

WorkModeState::WorkModeState(QObject* parent) : QObject(parent) {}

WorkMode WorkModeState::currentMode() const
{
    return m_mode;
}

void WorkModeState::setMode(WorkMode mode)
{
    if (m_mode == mode)
    {
        return;
    }

    m_mode = mode;
    emit modeChanged(m_mode);
    emit modifyModeChanged(m_mode == WorkMode::Modify);
    emit workModeChanged(m_mode == WorkMode::Work);
    emit checkModeChanged(m_mode == WorkMode::Check);
}
