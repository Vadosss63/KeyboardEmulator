#pragma once

#include <QString>

class QWidget;

class IFileDialogService
{
public:
    virtual ~IFileDialogService() = default;

    virtual QString openFile(QWidget*       parent,
                             const QString& title,
                             const QString& directory,
                             const QString& filter) = 0;

    virtual QString saveFile(QWidget*       parent,
                             const QString& title,
                             const QString& directory,
                             const QString& filter) = 0;
};
