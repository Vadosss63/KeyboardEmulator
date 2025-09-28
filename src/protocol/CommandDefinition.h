#pragma once

#include <cstdint>

enum class Command : uint8_t
{
    None               = 0x00,
    ButtonPressed      = 0x01,
    ButtonReleased     = 0x02,
    ModeCheckKeyboard  = 0x03,
    ModeRun            = 0x04,
    ModeConfigure      = 0x05,
    ModeDiodeConfig    = 0x06,
    ModeDiodeConfigDel = 0x07,
    DiodePressed       = 0x08,
    DiodeReleased      = 0x09
};
