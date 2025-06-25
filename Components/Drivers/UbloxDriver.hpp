#pragma once

#ifndef UbloxDriver
#define UbloxDriver

#include "main.h"
#include <stdint.h>
#include "cmsis_os.h"
#include "stm32l1xx_hal.h"
#include "SystemDefines.hpp"

//Private Macros
#define GPS_DEVICE_ADDRESS    (0x42 << 1)
#define GPS_DATA_REGISTER     0xFF
#define GPS_DATA_LENGTH_HIGH  0xFD
#define GPS_DATA_LENGTH_LOW   0xFE

/*
// UBX  Buffers
uint8_t UBX_CFG_PRT[] = {
    0xB5, 0x62,     // Sync Chars
    0x06, 0x00,     // Class and Message ID for Port Configuration
    0x14, 0x00,     // Length (20 bytes)
    0x00, 0x00,     // Port Identifier (0 = I2C)
    0x00, 0x00,     // txReady settings
    0x00, 0x00, 0x00, 0x00,  // I2C mode flags (cleared)
    0x00, 0x00, 0x00, 0x00,  // reserved
    0x03, 0x00,     // inProtoMask (NMEA + UBX)
    0x03, 0x00,     // outProtoMask (NMEA + UBX)
    0x00, 0x00,     // extended TX timeout
    0x00, 0x00,     // reserved
    // Checksum will be calculated by calculateChecksum()
    0x8D, 0x7A
};

uint8_t UBX_CFG_MSG[] = {
    0xB5, 0x62,     // Sync Chars
    0x06, 0x01,     // Class and Message ID for Message Configuration
    0x08, 0x00,     // Length (8 bytes)
    0x01, 0x02,     // Class and Message ID to configure (NAV-POSLLH)
    0x01, 0x00,     // Rate (1 = every navigation solution)
    0x00, 0x00, 0x00, 0x00,  // Reserved
    // Checksum will be calculated
    0x32, 0x32
};

uint8_t UBX_CFG_RATE[] = {
    0xB5, 0x62,     // Sync Chars
    0x06, 0x08,     // Class and Message ID for Rate Configuration
    0x06, 0x00,     // Length (6 bytes)
    0xE8, 0x03,     // Measurement Rate (1000 ms = 1 Hz)
    0x01, 0x00,     // Navigation Rate (1 cycle)
    0x01, 0x00,     // Time Reference (GPS time)
    // Checksum will be calculated
    0x0D, 0x3D
};

uint8_t UBX_CFG_RESET[] = {
    0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, // header and class/id bytes and length
    0xFF, 0xFF,                         // navBbrMask
    0x00, 0x00,                         // resetMode
    // Checksum will be calculated
    0x0C, 0x5D
};

uint8_t UBX_CFG_CFG[] = {
    0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, // header and class/id bytes and length
    0xFF, 0xFF, 0x00, 0x00,             // clearMask
    0x00, 0x00, 0x00, 0x00,             // saveMask
    0xFF, 0xFF, 0x00, 0x00,             // loadMask
    0x17,                               // deviceMask
    // Checksum will be calculated
    0x2F, 0xAE
};

uint8_t UBX_ACK_ACK[] = {
    0xB5, 0x62, 0x05, 0x01, 0x02, 0x00, // header and class/id bytes and length
    0x00,                               // classID of acknowledged message
    0x00,                               // messageID of acknowledged message
    // Checksum will be calculated
    0x00, 0x00
};

uint8_t UBX_CFG_NAV_PVT[] = {
    0xB5, 0x62,     // Sync Chars
    0x06, 0x01,     // Class and Message ID for Message Configuration
    0x08, 0x00,     // Length (8 bytes)
    0x01, 0x07,     // Class and Message ID (NAV-PVT)
    0x01, 0x00,     // Rate (1 = every navigation solution)
    0x00, 0x00, 0x00, 0x00,             // Reserved
    // Checksum will be calculated
    0x37, 0x37
};

*/

//Structs
typedef struct UBX_M8N_NAV_POSLLH {
    uint32_t iTOW;   // GPS time of week (ms)
    int32_t lon;     // Longitude (deg)
    int32_t lat;     // Latitude (deg)
    int32_t height;  // Height above ellipsoid (mm)
    int32_t hMSL;    // Height above mean sea level (mm)
    uint32_t hAcc;   // Horizontal accuracy estimate (mm)
    uint32_t vAcc;   // Vertical accuracy estimate (mm)
} NavData;

// CLASSES BELOW
class UBXMessage {
public:
    UBXMessage(uint8_t class_id, uint8_t msg_id, uint8_t length);
    virtual ~UBXMessage() = default;
    virtual void poll();                // override for polling behavior
    virtual void parse(uint8_t* buffer); // override for parsing behavior
    uint16_t calculateChecksum(uint8_t* buffer, uint8_t buflen);

protected:
    uint8_t msg_class;
    uint8_t msg_id;
    uint8_t length;
};

class GPSDevice {
public:
    static void UBX_Transmit(uint8_t* buffer, uint16_t buflen);
    static void UBX_Receive(uint8_t* buffer, uint16_t buflen);
    static uint16_t UBX_GET_LENGTH();
    static void CONFIG_Transmit(uint8_t* buffer, uint16_t buflen);
    static void GPS_Initialization();
    static void UBX_M8N_NAV_POSLLH_Parsing(uint8_t* buffer, NavData* data);
    static HAL_StatusTypeDef WaitUntilI2CReady(uint32_t timeout_ms);
};


#endif //UbloxDriver definition
