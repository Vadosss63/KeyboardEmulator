#pragma once

#include <QObject>
#include <QString>

class QAction;
class QMainWindow;

class ComPortMenu : public QObject
{
    Q_OBJECT

public:
    explicit ComPortMenu(QMainWindow* window, QObject* parent = nullptr);

    void setStatusText(const QString& text);

signals:
    void refreshRequested();

private:
    QMainWindow* m_window{nullptr};
    QAction*     m_statusAction{nullptr};
};
