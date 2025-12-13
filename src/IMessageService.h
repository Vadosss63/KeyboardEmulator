#pragma once

#include <QString>

class QWidget;

class IMessageService
{
public:
    virtual ~IMessageService() = default;

    virtual void showWarning(QWidget* parent, const QString& title, const QString& text) = 0;

    virtual bool confirmQuestion(QWidget*       parent,
                                 const QString& title,
                                 const QString& text,
                                 const QString& yesText = QString(),
                                 const QString& noText  = QString()) = 0;
};
