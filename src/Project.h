#pragma once

#include <QImage>
#include <QtCore>

struct ButtonDef
{
    QString id;
    int     p1 = 0;
    int     p2 = 0;
    QRectF  rect{0, 0, 0, 0};
};

struct LedDef
{
    QString id;
    int     pin      = 0;
    bool    inverted = false;
    QRectF  rect{0, 0, 0, 0};
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
