#include "RecentProjects.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>

RecentProjects::RecentProjects(QString settingsKey, int maxCount)
    : m_key(std::move(settingsKey)), m_maxCount(maxCount > 0 ? maxCount : 5)
{
}

QString RecentProjects::normalizePath(const QString& path)
{
    if (path.isEmpty())
    {
        return {};
    }
    QFileInfo     fi(path);
    const QString canon = fi.canonicalFilePath();
    return QDir::toNativeSeparators(canon.isEmpty() ? fi.absoluteFilePath() : canon);
}

QStringList RecentProjects::readRaw() const
{
    QSettings s;
    return s.value(m_key).toStringList();
}

void RecentProjects::writeRaw(const QStringList& list)
{
    QSettings s;
    s.setValue(m_key, list);
}

QStringList RecentProjects::list() const
{
    const QStringList raw = readRaw();

    QStringList cleaned;
    cleaned.reserve(raw.size());

    for (const QString& p : raw)
    {
        const QString n = normalizePath(p);
        if (!n.isEmpty() && QFileInfo::exists(n))
        {
            cleaned << n;
        }
    }
    return cleaned;
}

void RecentProjects::add(const QString& projectPath)
{
    const QString n = normalizePath(projectPath);
    if (n.isEmpty())
    {
        return;
    }

    QStringList cur = list();
    cur.removeAll(n);
    cur.prepend(n);

    while (cur.size() > m_maxCount)
    {
        cur.removeLast();
    }

    writeRaw(cur);

    setLastOpenDir(QFileInfo(n).absolutePath());
}

void RecentProjects::remove(const QString& projectPath)
{
    const QString n = normalizePath(projectPath);
    if (n.isEmpty())
    {
        return;
    }

    QStringList cur = readRaw();

    cur.removeAll(projectPath);
    cur.removeAll(n);
    writeRaw(cur);
}

void RecentProjects::clear()
{
    writeRaw({});
}

void RecentProjects::setMaxCount(int n)
{
    m_maxCount = n > 0 ? n : 5;

    QStringList cur = list();
    while (cur.size() > m_maxCount)
    {
        cur.removeLast();
    }
    writeRaw(cur);
}

QString RecentProjects::lastOpenDir() const
{
    QSettings s;
    return s.value(m_lastDirKey).toString();
}

void RecentProjects::setLastOpenDir(const QString& dirPath)
{
    if (dirPath.isEmpty())
    {
        return;
    }

    QSettings s;
    s.setValue(m_lastDirKey, QDir::toNativeSeparators(QDir(dirPath).absolutePath()));
}
