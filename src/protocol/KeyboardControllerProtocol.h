#pragma once

#include <stddef.h>
#include <stdint.h>

#include "CommandDefinition.h"
#include "PinsDefinition.h"

// Start of Frame
#define PROTOCOL_SOF 0xAA

#pragma pack(push, 1)

// Common packet structure
typedef struct
{
    uint8_t sof;        // PROTOCOL_SOF
    uint8_t length;     // bytes after this field up to and including checksum
    Command command;    // one of CMD_*
    uint8_t payload[0]; // command-specific payload
    //uint8_t checksum; // calc_checksum over all previous bytes
} Packet;

typedef struct
{
    Pins    pins;     // pin numbers (1..15) or 0
    uint8_t leds_num; // number of active LEDs
    Pins    leds[0];  // LED pin numbers (1..15)
} StatusPayload;

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

template <typename T>
static inline std::vector<uint8_t> build_packet_for_cmd(Command cmd, const T& payload)
{
    constexpr size_t payload_size = sizeof(T);
    constexpr size_t total_size   = offsetof(Packet, payload) + payload_size + sizeof(uint8_t); // + checksum

    std::vector<uint8_t> out_buffer(total_size);

    Packet* pkt  = reinterpret_cast<Packet*>(out_buffer.data());
    pkt->sof     = PROTOCOL_SOF;
    pkt->length  = static_cast<uint8_t>(total_size - 2); // exclude sof and length
    pkt->command = cmd;

    memcpy(pkt->payload, &payload, payload_size);

    constexpr size_t len_before_checksum = total_size - 1; // exclude checksum

    out_buffer.back() = calc_checksum(reinterpret_cast<const uint8_t*>(pkt), len_before_checksum);

    return out_buffer;
}
