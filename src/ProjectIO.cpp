#include "ProjectIO.h"

namespace ProjectIO
{
namespace
{

quint32 crc32_table[256];

struct KbkEntry
{
    quint32 type; // FourCC
    QString name; // может быть пустым
    quint32 offset;
    quint32 size;
    quint32 crc32;
};

void crc32_init()
{
    static bool inited = false;
    if (inited)
    {
        return;
    }
    inited = true;
    for (quint32 i = 0; i < 256; ++i)
    {
        quint32 c = i;
        for (int k = 0; k < 8; ++k)
        {
            c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        }
        crc32_table[i] = c;
    }
}

quint32 crc32_bytes(const QByteArray& a)
{
    crc32_init();
    quint32      crc = ~0u;
    const uchar* p   = reinterpret_cast<const uchar*>(a.constData());
    for (int i = 0; i < a.size(); ++i)
    {
        crc = crc32_table[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
    }
    return ~crc;
}

// --- FourCC ---
constexpr quint32 FCC(char a, char b, char c, char d)
{
    return quint32(quint8(a)) | (quint32(quint8(b)) << 8) | (quint32(quint8(c)) << 16) | (quint32(quint8(d)) << 24);
}

// --- Писатели LE (поверх QIODevice) ---
inline void putU16(QIODevice& d, quint16 v)
{
    quint8 b[2] = {quint8(v), quint8(v >> 8)};
    d.write((char*)b, 2);
}

inline void putU32(QIODevice& d, quint32 v)
{
    quint8 b[4] = {quint8(v), quint8(v >> 8), quint8(v >> 16), quint8(v >> 24)};
    d.write((char*)b, 4);
}

inline quint16 getU16(QIODevice& d)
{
    quint8 b[2];
    d.read((char*)b, 2);
    return quint16(b[0] | (b[1] << 8));
}

inline quint32 getU32(QIODevice& d)
{
    quint8 b[4];
    d.read((char*)b, 4);
    return quint32(b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24));
}
}

bool save(const QString& filePath, const Project& prj, bool withThumbnail)
{
    // Подготовка payload'ов
    QByteArray mani = prj.manifestJson.isEmpty() ? prj.toManifestJson() : prj.manifestJson;

    QByteArray bgPng;
    {
        QBuffer pb(&bgPng);
        pb.open(QIODevice::WriteOnly);
        if (!prj.background.isNull())
        {
            if (!prj.background.save(&pb, "PNG")) return false;
        }
    }

    QByteArray thumbJpg;
    if (withThumbnail && !prj.background.isNull())
    {
        QImage  t = prj.background.scaledToWidth(480, Qt::SmoothTransformation);
        QBuffer tb(&thumbJpg);
        tb.open(QIODevice::WriteOnly);
        t.save(&tb, "JPG", 80);
    }

    struct Payload
    {
        quint32    type;
        QString    name;
        QByteArray data;
    };

    QVector<Payload> payloads;
    payloads.push_back({FCC('M', 'A', 'N', 'I'), {}, mani});
    if (!bgPng.isEmpty())
    {
        payloads.push_back({FCC('B', 'K', 'P', 'N'), {}, bgPng});
    }
    if (!thumbJpg.isEmpty())
    {
        payloads.push_back({FCC('T', 'H', 'M', 'B'), {}, thumbJpg});
    }

    // Атомарная запись
    QSaveFile sf(filePath);
    if (!sf.open(QIODevice::WriteOnly))
    {
        return false;
    }

    // 1) Резервируем место под Header
    const qint64 headerPos = sf.pos();
    for (int i = 0; i < 24; ++i)
    {
        sf.putChar('\0');
    }

    // 2) Пишем payload'ы, собираем TOC
    QVector<KbkEntry> toc;
    for (const auto& p : payloads)
    {
        KbkEntry e;
        e.type   = p.type;
        e.name   = p.name;
        e.offset = sf.pos();
        e.size   = p.data.size();
        e.crc32  = crc32_bytes(p.data);

        if (sf.write(p.data) != p.data.size())
        {
            return false;
        }
        toc.push_back(e);
        while (sf.pos() % 4)
        {
            sf.putChar('\0');
        }
    }

    // 3) TOC
    quint32 tocOffset = sf.pos();
    for (const auto& e : toc)
    {
        putU32(sf, e.type);
        putU32(sf, e.offset);
        putU32(sf, e.size);
        putU32(sf, e.crc32);
        QByteArray nameUtf8 = e.name.toUtf8();
        putU16(sf, quint16(nameUtf8.size()));
        if (!nameUtf8.isEmpty())
        {
            sf.write(nameUtf8);
        }
    }
    quint32 tocCount = toc.size();

    // 4) Header
    sf.seek(headerPos);
    putU32(sf, FCC('K', 'B', 'K', '1')); // magic
    putU16(sf, 1);                       // version
    putU16(sf, 0);                       // flags
    putU32(sf, tocOffset);
    putU32(sf, tocCount);
    putU32(sf, 0); // reserved

    // 5) Готово
    return sf.commit();
}

bool load(const QString& filePath, Project& out)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly))
    {
        return false;
    }

    // Header
    if (f.size() < 24)
    {
        return false;
    }
    quint32 magic = getU32(f);
    quint16 ver   = getU16(f);
    (void)getU16(f); // flags
    quint32 tocOffset = getU32(f);
    quint32 tocCount  = getU32(f);
    (void)getU32(f); // reserved
    if (magic != FCC('K', 'B', 'K', '1') || ver != 1)
    {
        return false;
    }

    if (tocOffset > (quint64)f.size())
    {
        return false;
    }

    // TOC
    f.seek(tocOffset);
    QVector<KbkEntry> toc;
    toc.reserve(tocCount);
    for (quint32 i = 0; i < tocCount; ++i)
    {
        KbkEntry e;
        e.type          = getU32(f);
        e.offset        = getU32(f);
        e.size          = getU32(f);
        e.crc32         = getU32(f);
        quint16 nameLen = getU16(f);
        if (nameLen)
        {
            QByteArray name = f.read(nameLen);
            e.name          = QString::fromUtf8(name);
        }
        toc.push_back(e);
    }

    // Чтение нужных блобов
    auto readBlob = [&](quint32 fourcc) -> QByteArray
    {
        for (const auto& e : toc)
            if (e.type == fourcc)
            {
                if (quint64(e.offset) + e.size > (quint64)f.size())
                {
                    return {};
                }
                f.seek(e.offset);
                QByteArray d = f.read(e.size);
                if (crc32_bytes(d) != e.crc32)
                {
                    return {};
                }
                return d;
            }
        return {};
    };

    QByteArray mani = readBlob(FCC('M', 'A', 'N', 'I'));
    QByteArray bk   = readBlob(FCC('B', 'K', 'P', 'N'));

    if (!mani.isEmpty())
    {
        out              = Project::fromManifestJson(mani);
        out.manifestJson = mani;
    }
    else
    {
        out = Project{};
    }

    if (!bk.isEmpty())
    {
        QImage bg;
        bg.loadFromData(bk, "PNG");
        out.background = bg;
    }

    return true;
}
}