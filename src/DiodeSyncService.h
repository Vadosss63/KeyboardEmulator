#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QVector>

#include "CommandDefinition.h"
#include "PinsDefinition.h"

class SerialPortModel;

class DiodeSyncService : public QObject
{
    Q_OBJECT

public:
    explicit DiodeSyncService(SerialPortModel* model, QObject* parent = nullptr);

public slots:
    void reset(const QVector<Pins>& diodes);
    void upsert(Pins pins);
    void remove(Pins pins);
    void handleConnectionEstablished();
    void handleConnectionLost();

private:
    void sendFullState();
    void sendCommand(Command command, Pins pins) const;
    void sendCommand(Command command) const;

    static QString keyFor(Pins pins);

    SerialPortModel*     m_model{nullptr};
    QHash<QString, Pins> m_diodeStates;

    bool m_connected{false};
    bool m_fullSyncRequired{false};
};
