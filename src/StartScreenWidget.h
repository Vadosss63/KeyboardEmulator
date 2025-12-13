#pragma once

#include <QFrame>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QStringList>
#include <QWidget>

class StartScreenWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StartScreenWidget(QWidget* parent = nullptr);

    void setRecentEntries(const QStringList& entries);

signals:
    void loadImageRequested();
    void loadProjectRequested();
    void recentItemActivated(const QString& path);
    void clearRecentRequested();

private:
    void resizeRecentListToContents();

    QListWidget* recentList{nullptr};
    QPushButton* clearButton{nullptr};
};
