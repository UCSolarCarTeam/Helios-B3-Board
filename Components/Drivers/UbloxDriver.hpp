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

extern uint8_t UBX_CFG_PRT[];
extern uint8_t UBX_CFG_MSG[];
extern uint8_t UBX_CFG_RATE[];
extern uint8_t UBX_CFG_RESET[];
extern uint8_t UBX_CFG_CFG[];
extern uint8_t UBX_ACK_ACK[];
extern uint8_t UBX_CFG_NAV_PVT[];

extern uint16_t UBX_CFG_PRT_LEN;
extern uint16_t UBX_CFG_MSG_LEN;
extern uint16_t UBX_CFG_RATE_LEN;
extern uint16_t UBX_CFG_RESET_LEN;
extern uint16_t UBX_CFG_CFG_LEN;
extern uint16_t UBX_ACK_ACK_LEN;
extern uint16_t UBX_CFG_NAV_PVT_LEN;

// UBX Class=0x01 (NAV), ID=0x02 (POSLLH), length=0 payload
extern uint8_t UBX_POLL_POSLLH[8];
extern const uint16_t UBX_POLL_POSLLH_LEN;

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
    static uint16_t calculateChecksum(uint8_t* buffer, uint8_t buflen);

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


