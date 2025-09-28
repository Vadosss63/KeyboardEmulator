#ifndef KEYBOARDCONTROLLERPROTOCOL_H
#define KEYBOARDCONTROLLERPROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#include "PinsDefinition.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Start of Frame
#define PROTOCOL_SOF 0xAA

// Commands: App -> Controller
#define CMD_BTN_PRESSED 0x01
#define CMD_BTN_RELEASED 0x02
#define CMD_MODE_CHECK_KEYBOARD 0x03
#define CMD_MODE_RUN 0x04
#define CMD_MODE_CONFIGURE 0x05
#define CMD_MODE_DIODE_CONF 0x06
#define CMD_DIODE_PRESSED 0x07
#define CMD_DIODE_RELEASED 0x08

#pragma pack(push, 1)
    // Packet from Application to Controller
    // Bytes: SOF | Length | Command | Pin1 | Pin2 | Checksum
    typedef struct
    {
        uint8_t sof;      // PROTOCOL_SOF
        uint8_t length;   // bytes after this field up to and including checksum
        uint8_t command;  // one of CMD_*
        Pins    pins;     // pin numbers (1..15) or 0
        uint8_t checksum; // calc_checksum over all previous bytes
    } App2Ctrl_Packet;

    // Packet from Controller to Application
    // Bytes: SOF | Length | Pin1 | Pin2 | LED pins | Checksum
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
    /**
 * @brief Calculate simple checksum: sum of bytes modulo 256
 * @param data Pointer to buffer
 * @param len  Number of bytes to include
 * @return Low byte of sum
 */
    static inline uint8_t calc_checksum(const uint8_t* data, size_t len)
    {
        uint16_t sum = 0;
        for (size_t i = 0; i < len; ++i)
        {
            sum += data[i];
        }
        return (uint8_t)(sum & 0xFF);
    }

#ifdef __cplusplus
}
#endif

#endif // KEYBOARDCONTROLLERPROTOCOL_H
