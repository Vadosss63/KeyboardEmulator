#pragma once

#include <QActionGroup>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QSerialPortInfo>

class ComPortMenu : public QObject
{
    Q_OBJECT
public:
    explicit ComPortMenu(QObject* parent = nullptr);

    QMenu* createMenu(const QString& baseTitle = QStringLiteral("COM Port"));

    void setCurrentPort(const QString& portName);

    void addToMenuBar(QMenuBar* bar);

    void setTestPortVisible(bool on, const QString& name = QStringLiteral("/tmp/ttyV1"));

signals:
    void portSelected(const QString& name);

private slots:
    void refreshMenu();
    void onActionTriggered(QAction*);

private:
    void updateTitle();
    void rebuildActions();

    QMenu*        menu{};
    QActionGroup* group{};
    QString       baseTitle{QStringLiteral("COM Port")};
    QString       currentPort;
    bool          showTest{};
    QString       testName{QStringLiteral("/tmp/ttyV1")};
};
