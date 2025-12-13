#pragma once

#include <QObject>

#include "IFileDialogService.h"

class QtFileDialogService : public QObject, public IFileDialogService
{
    Q_OBJECT

public:
    explicit QtFileDialogService(QObject* parent = nullptr);

    QString openFile(QWidget* parent, const QString& title, const QString& directory, const QString& filter) override;

    QString saveFile(QWidget* parent, const QString& title, const QString& directory, const QString& filter) override;
};
