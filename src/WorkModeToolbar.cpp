#include "WorkModeToolbar.h"

#include <QAction>
#include <QActionGroup>
#include <QMainWindow>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>

WorkModeToolbar::WorkModeToolbar(QMainWindow* window, QObject* parent) : QObject(parent), m_window(window)
{
    Q_ASSERT(m_window);
    m_toolbar   = m_window->addToolBar(tr("WorkMode"));
    auto* menu  = new QMenu(m_window);
    auto* group = new QActionGroup(menu);
    group->setExclusive(true);

    m_actionCheck = menu->addAction(tr("Проверка клавиатуры"));
    m_actionCheck->setCheckable(true);
    m_actionCheck->setData(static_cast<int>(WorkMode::Check));
    m_actionCheck->setActionGroup(group);

    m_actionWork = menu->addAction(tr("Режим работы"));
    m_actionWork->setCheckable(true);
    m_actionWork->setData(static_cast<int>(WorkMode::Work));
    m_actionWork->setActionGroup(group);

    m_actionModify = menu->addAction(tr("Режим модификации"));
    m_actionModify->setCheckable(true);
    m_actionModify->setData(static_cast<int>(WorkMode::Modify));
    m_actionModify->setActionGroup(group);

    m_modeButton = new QToolButton(m_window);
    m_modeButton->setPopupMode(QToolButton::InstantPopup);
    m_modeButton->setMenu(menu);
    m_modeButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_modeButton->setEnabled(false);
    m_toolbar->setVisible(false);

    m_toolbar->addWidget(m_modeButton);

    connect(group,
            &QActionGroup::triggered,
            this,
            [this](QAction* act)
            {
                if (!act)
                {
                    return;
                }
                m_modeButton->setDefaultAction(act);
                const auto mode = static_cast<WorkMode>(act->data().toInt());
                emit       workModeSelected(mode);
            });

    m_loadImageAction = m_toolbar->addAction(tr("Загрузка изображения"));
    connect(m_loadImageAction, &QAction::triggered, this, &WorkModeToolbar::loadImageRequested);

    m_saveProjectAction = m_toolbar->addAction(tr("Сохранить проект"));
    m_saveProjectAction->setEnabled(false);
    connect(m_saveProjectAction, &QAction::triggered, this, &WorkModeToolbar::saveProjectRequested);

    m_loadProjectAction = m_toolbar->addAction(tr("Загрузить проект"));
    connect(m_loadProjectAction, &QAction::triggered, this, &WorkModeToolbar::loadProjectRequested);

    m_statusAction = m_toolbar->addAction(tr("Pins: P1: , P2: , LEDs: "));
    m_statusAction->setCheckable(false);
    m_statusAction->setVisible(false);

    setCurrentMode(WorkMode::Modify);
}

void WorkModeToolbar::setCurrentMode(WorkMode mode)
{
    m_currentMode = mode;
    if (QAction* act = actionForMode(mode))
    {
        act->setChecked(true);
        if (m_modeButton)
        {
            m_modeButton->setDefaultAction(act);
        }
    }

    updateModifyActionsVisibility(mode);
    updateStatusVisibility();
}

void WorkModeToolbar::setProjectReady(bool ready)
{
    m_projectReady = ready;
    if (m_modeButton)
    {
        m_modeButton->setEnabled(ready);
    }
    if (m_saveProjectAction)
    {
        m_saveProjectAction->setEnabled(ready && m_saveProjectAction->isVisible());
    }
}

void WorkModeToolbar::setStatusText(const QString& text)
{
    if (m_statusAction)
    {
        m_statusAction->setText(text);
    }
}

void WorkModeToolbar::setVisible(bool visible)
{
    if (m_toolbar)
    {
        m_toolbar->setVisible(visible);
    }
}

QAction* WorkModeToolbar::actionForMode(WorkMode mode) const
{
    switch (mode)
    {
        case WorkMode::Check:
            return m_actionCheck;
        case WorkMode::Work:
            return m_actionWork;
        case WorkMode::Modify:
        default:
            return m_actionModify;
    }
}

void WorkModeToolbar::updateModifyActionsVisibility(WorkMode mode)
{
    const bool visible = (mode == WorkMode::Modify);
    if (m_loadImageAction)
    {
        m_loadImageAction->setVisible(visible);
    }
    if (m_loadProjectAction)
    {
        m_loadProjectAction->setVisible(visible);
    }
    if (m_saveProjectAction)
    {
        m_saveProjectAction->setVisible(visible);
        m_saveProjectAction->setEnabled(visible && m_projectReady);
    }
}

void WorkModeToolbar::updateStatusVisibility()
{
    if (m_statusAction)
    {
        m_statusAction->setVisible(m_currentMode == WorkMode::Check);
    }
}
