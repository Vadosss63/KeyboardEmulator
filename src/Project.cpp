#include "Project.h"

namespace
{

QJsonObject rectToJson(const QRectF& r)
{
    return QJsonObject{{"x", r.x()}, {"y", r.y()}, {"w", r.width()}, {"h", r.height()}};
}
QRectF rectFromJson(const QJsonValue& v)
{
    auto o = v.toObject();
    return QRectF(o.value("x").toDouble(), o.value("y").toDouble(), o.value("w").toDouble(), o.value("h").toDouble());
}

}

QByteArray Project::toManifestJson() const
{
    QJsonObject root;
    root["format"]  = "keyboard-config";
    root["version"] = 1;

    // canvas metadata
    QJsonObject canvas;
    canvas["background"] = "assets/background.png";
    canvas["size_px"]    = QJsonObject{{"w", background.width()}, {"h", background.height()}};
    root["canvas"]       = canvas;

    // buttons
    QJsonArray jButtons;
    for (const auto& b : buttons)
    {
        QJsonObject jb;
        jb["color"]      = b.color;
        jb["isCircular"] = b.isCircular;
        jb["p1"]         = b.p1;
        jb["p2"]         = b.p2;
        jb["rect"]       = rectToJson(b.rect);
        jButtons.push_back(jb);
    }
    root["buttons"] = jButtons;

    // leds
    QJsonArray jLeds;
    for (const auto& l : leds)
    {
        QJsonObject jl;
        jl["color"]      = l.color;
        jl["isCircular"] = l.isCircular;
        jl["pin"]        = l.pin;
        jl["inverted"]   = l.inverted;
        jl["rect"]       = rectToJson(l.rect);
        jLeds.push_back(jl);
    }
    root["leds"] = jLeds;

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented);
}

Project Project::fromManifestJson(const QByteArray& json)
{
    Project p;
    p.manifestJson = json;

    QJsonParseError err{};
    QJsonDocument   doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
    {
        return p;
    }

    QJsonObject root = doc.object();

    // buttons
    p.buttons.clear();
    for (const auto& v : root.value("buttons").toArray())
    {
        if (!v.isObject())
        {
            continue;
        }

        auto      jb = v.toObject();
        ButtonDef b;
        b.color      = jb.value("color").toString();
        b.isCircular = jb.value("isCircular").toBool();
        b.p1         = jb.value("p1").toInt();
        b.p2         = jb.value("p2").toInt();
        b.rect       = rectFromJson(jb.value("rect"));
        p.buttons.push_back(b);
    }

    // leds
    p.leds.clear();
    for (const auto& v : root.value("leds").toArray())
    {
        if (!v.isObject())
        {
            continue;
        }

        auto   jl = v.toObject();
        LedDef l;
        l.color      = jl.value("color").toString();
        l.isCircular = jl.value("isCircular").toBool();
        l.pin        = jl.value("pin").toInt();
        l.inverted   = jl.value("inverted").toBool(false);
        l.rect       = rectFromJson(jl.value("rect"));
        p.leds.push_back(l);
    }

    return p;
}
