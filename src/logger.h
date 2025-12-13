#pragma once

#include <QDateTime>
#include <QString>
#include <iomanip>
#include <iostream>

#define LOG_INFO (std::cout << "[" << GetCurrentTime() << "] /INF: ")
#define LOG_WRN (std::cout << "[" << GetCurrentTime() << "] /WRN: ")
#define LOG_ERR (std::cerr << "[" << GetCurrentTime() << "] /ERR: ")

inline std::string GetCurrentTime()
{
    return QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toStdString();
}