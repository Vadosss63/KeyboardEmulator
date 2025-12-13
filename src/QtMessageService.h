#pragma once

#include <QObject>

#include "IMessageService.h"

class QtMessageService : public QObject, public IMessageService
{
    Q_OBJECT

public:
    explicit QtMessageService(QObject* parent = nullptr);

    void showWarning(QWidget* parent, const QString& title, const QString& text) override;
    bool confirmQuestion(QWidget*       parent,
                         const QString& title,
                         const QString& text,
                         const QString& yesText = QString(),
                         const QString& noText  = QString()) override;
};
