#pragma once

#include <QImage>
#include <QString>
#include <QtCore>

struct ItemDef
{
    ItemDef() = default;
    ItemDef(qreal x, qreal y, const QString& color, bool isCircular)
        : rect{x, y, 80, 80}, color(color), isCircular(isCircular)
    {
    }

    QString color{};

    bool isCircular{false};

    int    p1{0};
    int    p2{0};
    QRectF rect{0, 0, 0, 0};
};

struct ButtonDef : public ItemDef
{
    ButtonDef(qreal x, qreal y) : ItemDef(x, y, "#0000FF", false) {}

    ButtonDef() : ButtonDef(0, 0) {}
};

struct LedDef : public ItemDef
{
    LedDef(qreal x, qreal y) : ItemDef(x, y, "#00FF00", true) {}

    LedDef() : LedDef(0, 0) {}
};

class Project
{
public:
    QImage     background;
    QByteArray manifestJson; // UTF-8

    QList<ItemDef> buttons;
    QList<ItemDef>  leds;

    QByteArray     toManifestJson() const;
    static Project fromManifestJson(const QByteArray&);
};
