#include "ComPortMenu.h"

#include <QMenuBar>

ComPortMenu::ComPortMenu(QObject* parent) : QObject(parent) {}

QMenu* ComPortMenu::createMenu(const QString& title)
{
    if (!menu)
    {
        menu  = new QMenu;
        group = new QActionGroup(menu);
        group->setExclusive(true);
        connect(group, &QActionGroup::triggered, this, &ComPortMenu::onActionTriggered);
        connect(menu, &QMenu::aboutToShow, this, &ComPortMenu::refreshMenu);
        menu->addAction(QStringLiteral("Loading..."))->setEnabled(false);
    }
    baseTitle = title;
    updateTitle();
    return menu;
}

void ComPortMenu::addToMenuBar(QMenuBar* bar)
{
    if (!menu)
    {
        createMenu(baseTitle);
    }

    if (bar)
    {
        bar->addMenu(menu);
    }
}

void ComPortMenu::setTestPortVisible(bool on, const QString& name)
{
    showTest = on;
    if (!name.isEmpty()) testName = name;
}

void ComPortMenu::setCurrentPort(const QString& portName)
{
    currentPort = portName;
    updateTitle();
}

void ComPortMenu::refreshMenu()
{
    rebuildActions();
}

void ComPortMenu::onActionTriggered(QAction* action)
{
    const QString name = action->data().toString();
    setCurrentPort(name);
    emit portSelected(name);
}

void ComPortMenu::updateTitle()
{
    if (!menu)
    {
        return;
    }
    const QString shown = currentPort.isEmpty() ? QStringLiteral("None") : currentPort;
    menu->setTitle(QStringLiteral("%1: %2").arg(baseTitle, shown));
}

void ComPortMenu::rebuildActions()
{
    if (!menu)
    {
        return;
    }
    menu->clear();

    {
        QAction* none = menu->addAction(QStringLiteral("None"));
        none->setCheckable(true);
        none->setData(QString{});
        none->setChecked(currentPort.isEmpty());
        none->setActionGroup(group);
    }

    menu->addSeparator();

    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& info : ports)
    {
        const QString name = info.portName();
        QAction*      act  = menu->addAction(name);
        act->setCheckable(true);
        act->setData(name);
        act->setChecked(name == currentPort);
        act->setActionGroup(group);
    }

    if (showTest)
    {
        QAction* test = menu->addAction(testName);
        test->setCheckable(true);
        test->setData(testName);
        test->setChecked(testName == currentPort);
        test->setActionGroup(group);
    }

    if (ports.isEmpty() && !showTest)
    {
        auto* noPorts = menu->addAction(QStringLiteral("No ports found"));
        noPorts->setEnabled(false);
    }
}
