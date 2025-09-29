#pragma once

#include <cstdint>

enum class Command : uint8_t
{
    None               = 0x00,
    Echo               = 0x01,
    ButtonPressed      = 0x02,
    ButtonReleased     = 0x03,
    ModeCheckKeyboard  = 0x04,
    ModeRun            = 0x05,
    ModeConfigure      = 0x06,
    ModeDiodeConfig    = 0x07,
    ModeDiodeConfigDel = 0x08,
    ModeDiodeClear     = 0x09,
    DiodePressed       = 0x0A,
    DiodeReleased      = 0x0B
};
