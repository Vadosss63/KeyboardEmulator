#include "QtFileDialogService.h"

#include <QFileDialog>

QtFileDialogService::QtFileDialogService(QObject* parent) : QObject(parent) {}

QString QtFileDialogService::openFile(QWidget*       parent,
                                      const QString& title,
                                      const QString& directory,
                                      const QString& filter)
{
    return QFileDialog::getOpenFileName(parent, title, directory, filter);
}

QString QtFileDialogService::saveFile(QWidget*       parent,
                                      const QString& title,
                                      const QString& directory,
                                      const QString& filter)
{
    return QFileDialog::getSaveFileName(parent, title, directory, filter);
}
