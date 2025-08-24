#pragma once

#include <QFont>

inline constexpr int  margin{8};
inline constexpr int  textSize{14};
inline constexpr auto textFamily{"Arial"};

inline QFont getDefaultFont()
{
    return QFont(QFont(textFamily, textSize));
}
