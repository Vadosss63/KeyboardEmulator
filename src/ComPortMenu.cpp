#include "ComPortMenu.h"

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>

ComPortMenu::ComPortMenu(QMainWindow* window, QObject* parent) : QObject(parent), m_window(window)
{
    Q_ASSERT(m_window);
    auto* menu = new QMenu(tr("COM Порт"), m_window);

    m_statusAction = menu->addAction(tr("Соединение: нет"));
    menu->addSeparator();

    QAction* refreshAction = menu->addAction(tr("Поиск устройства"));
    connect(refreshAction, &QAction::triggered, this, &ComPortMenu::refreshRequested);

    m_window->menuBar()->addMenu(menu);
}

void ComPortMenu::setStatusText(const QString& text)
{
    if (m_statusAction)
    {
        m_statusAction->setText(text);
    }
}
