#include "QtMessageService.h"

#include <QAbstractButton>
#include <QMessageBox>

QtMessageService::QtMessageService(QObject* parent) : QObject(parent) {}

void QtMessageService::showWarning(QWidget* parent, const QString& title, const QString& text)
{
    QMessageBox::warning(parent, title, text);
}

bool QtMessageService::confirmQuestion(QWidget*       parent,
                                       const QString& title,
                                       const QString& text,
                                       const QString& yesText,
                                       const QString& noText)
{
    QMessageBox msgBox(QMessageBox::Question, title, text, QMessageBox::Yes | QMessageBox::No, parent);
    if (!yesText.isEmpty())
    {
        msgBox.button(QMessageBox::Yes)->setText(yesText);
    }
    if (!noText.isEmpty())
    {
        msgBox.button(QMessageBox::No)->setText(noText);
    }

    return msgBox.exec() == QMessageBox::Yes;
}
