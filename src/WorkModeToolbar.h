#pragma once

#include <QObject>
#include <QString>

#include "WorkMode.h"

class QAction;
class QActionGroup;
class QMainWindow;
class QMenu;
class QToolBar;
class QToolButton;

class WorkModeToolbar : public QObject
{
    Q_OBJECT

public:
    explicit WorkModeToolbar(QMainWindow* window, QObject* parent = nullptr);

    void setCurrentMode(WorkMode mode);

public slots:
    void setProjectReady(bool ready);
    void setStatusText(const QString& text);

signals:
    void workModeSelected(WorkMode mode);
    void loadImageRequested();
    void saveProjectRequested();
    void loadProjectRequested();

private:
    QAction* actionForMode(WorkMode mode) const;
    void     updateModifyActionsVisibility(WorkMode mode);
    void     updateStatusVisibility();

    QMainWindow* m_window{nullptr};
    QToolBar*    m_toolbar{nullptr};
    QToolButton* m_modeButton{nullptr};

    QAction* m_actionCheck{nullptr};
    QAction* m_actionWork{nullptr};
    QAction* m_actionModify{nullptr};

    QAction* m_loadImageAction{nullptr};
    QAction* m_saveProjectAction{nullptr};
    QAction* m_loadProjectAction{nullptr};
    QAction* m_statusAction{nullptr};

    WorkMode m_currentMode{WorkMode::Modify};
    bool     m_projectReady{false};
};
