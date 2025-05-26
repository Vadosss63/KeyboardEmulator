#ifndef KEYBOARDCONTROLLERPROTOCOL_H
#define KEYBOARDCONTROLLERPROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Start of Frame
#define PROTOCOL_SOF 0xAA

// Status codes
#define STATUS_OK 0x00
#define STATUS_ERROR 0xFF

// Commands: App -> Controller
#define CMD_BTN_PRESSED 0x01
#define CMD_BTN_RELEASED 0x02
#define CMD_MODE_CHECK_KEYBOARD 0x03
#define CMD_MODE_RUN 0x04

// Responses: Controller -> App (ACK values)
#define CMD_ACK_BTN_PRESSED 0x81
#define CMD_ACK_BTN_RELEASED 0x82
#define CMD_ACK_MODE_CHECK 0x83
#define CMD_ACK_MODE_RUN 0x84
#define CMD_NACK 0xE0

#pragma pack(push, 1)

    // Packet from Application to Controller
    // Bytes: SOF | Length | Command | Pin1 | Pin2 | Checksum
    typedef struct
    {
        uint8_t sof;      // PROTOCOL_SOF
        uint8_t length;   // bytes after this field up to and including checksum
        uint8_t command;  // one of CMD_*
        uint8_t pin1;     // pin number 1 (1..15) or 0
        uint8_t pin2;     // pin number 2 (1..15) or 0
        uint8_t checksum; // calc_checksum over all previous bytes
    } App2Ctrl_Packet;

    // Packet from Controller to Application
    // Bytes: SOF | Length | Command | Status | Pin1 | Pin2 | LED[1..15] | Checksum
    typedef struct
    {
        uint8_t sof;      // PROTOCOL_SOF
        uint8_t length;   // bytes after this field up to and including checksum
        uint8_t command;  // echo or ACK code
        uint8_t status;   // STATUS_OK or STATUS_ERROR
        uint8_t pin1;     // current pin1 (in check mode) or 0
        uint8_t pin2;     // current pin2 (in check mode) or 0
        uint8_t leds[15]; // 0x00=off, 0x01=on for each LED
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
