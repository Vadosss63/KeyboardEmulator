#pragma once

#include <QObject>

#include "WorkMode.h"

class WorkModeState : public QObject
{
    Q_OBJECT

public:
    explicit WorkModeState(QObject* parent = nullptr);

    WorkMode currentMode() const;

public slots:
    void setMode(WorkMode mode);

signals:
    void modeChanged(WorkMode mode);
    void modifyModeChanged(bool enable);
    void workModeChanged(bool enable);
    void checkModeChanged(bool enable);

private:
    WorkMode m_mode{WorkMode::None};
};
