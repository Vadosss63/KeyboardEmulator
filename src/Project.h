#pragma once

#include <QImage>
#include <QString>
#include <QtCore>

struct ButtonDef
{
    ButtonDef() = default;
    ButtonDef(qreal x, qreal y) : rect{x, y, 80, 80} {}

    QString color = "#0000FF";

    bool isCircular = false;

    int    p1 = 0;
    int    p2 = 0;
    QRectF rect{0, 0, 0, 0};
};

struct LedDef
{
    LedDef() = default;
    LedDef(qreal x, qreal y) : rect{x, y, 80, 80} {}

    QString color = "#00FF00";

    bool isCircular = true;

    int    pin1 = 0;
    int    pin2 = 0;
    QRectF rect{0, 0, 0, 0};
};

class Project
{
public:
    QImage     background;
    QByteArray manifestJson; // UTF-8

    QList<ButtonDef> buttons;
    QList<LedDef>    leds;

    QByteArray     toManifestJson() const;
    static Project fromManifestJson(const QByteArray&);
};
