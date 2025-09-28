#pragma once

#include <stddef.h>
#include <stdint.h>

#include "CommandDefinition.h"
#include "PinsDefinition.h"

// Start of Frame
#define PROTOCOL_SOF 0xAA

#pragma pack(push, 1)
// Packet from Application to Controller
typedef struct
{
    uint8_t sof;      // PROTOCOL_SOF
    uint8_t length;   // bytes after this field up to and including checksum
    Command command;  // one of CMD_*
    Pins    pins;     // pin numbers (1..15) or 0
    uint8_t checksum; // calc_checksum over all previous bytes
} App2Ctrl_Packet;

// Packet from Controller to Application
typedef struct
{
    uint8_t sof;      // PROTOCOL_SOF
    uint8_t length;   // bytes after this field up to and including checksum
    Pins    pins;     // pin numbers (1..15) or 0
    uint8_t leds_num; // number of active LEDs
    Pins    leds[0];  // LED pin numbers (1..15)
    uint8_t checksum; // calc_checksum over all previous bytes
} Ctrl2App_Packet;

#pragma pack(pop)

static inline uint8_t calc_checksum(const uint8_t* data, size_t len)
{
    uint16_t sum = 0;
    for (size_t i = 0; i < len; ++i)
    {
        sum += data[i];
    }
    return (uint8_t)(sum & 0xFF);
}
