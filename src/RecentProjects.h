#pragma once

#include <QString>
#include <QStringList>

class RecentProjects
{
public:
    explicit RecentProjects(QString settingsKey = QStringLiteral("recentProjects"), int maxCount = 5);

    QStringList list() const;

    void add(const QString& projectPath);
    void remove(const QString& projectPath);
    void clear();

    int  maxCount() const { return m_maxCount; }
    void setMaxCount(int n);

    QString lastOpenDir() const;
    void    setLastOpenDir(const QString& dirPath);

    QString key() const { return m_key; }

    static QString normalizePath(const QString& path);

private:
    QStringList readRaw() const;
    void        writeRaw(const QStringList&);

private:
    QString m_key;
    int     m_maxCount;

    const QString m_lastDirKey = QStringLiteral("lastOpenDir");
};
