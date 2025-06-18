#pragma once

#include "main.h"
#include <stdint.h>
#include "cmsis_os.h"
#include "stm32l1xx_hal.h"


/*-------------- Private Macros --------------*/
#define GPS_DEVICE_ADDRESS (0x42 << 1)	// GPS device address is 0x42, left-shifted for STM32 uses 7-bit address
#define GPS_DATA_REGISTER 0xFF		    // register address for GPS data stream

// In case for "random access" read from i2c (refer to page 38 of data sheet)
#define GPS_DATA_LENGTH_HIGH 0xFD	// register address for GPS data length (high byte)
#define GPS_DATA_LENGTH_LOW 0xFE	// register address for GPS data length (low byte)

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
    // Checksum will be calculated
	0x8D, 0x7A
};

uint8_t UBX_CFG_MSG[] = {
    0xB5, 0x62,     // Sync Chars
    0x06, 0x01,     // Class and Message ID for Message Configuration
    0x02, 0x00,     // Length (8 bytes)
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
	0xB5, 0x62, 0x06, 0x04, 0x04, 0x00,	// header and class/id bytes and length
	0xFF, 0xFF,							// navBbrMask
	0x00, 0x00,							// resetMode
	// checksum
	0x0C, 0x5D
};

// might be used...
uint8_t UBX_CFG_CFG[] = {
	0xB5, 0x62, 0x06, 0x09,	0x0D, 0x00,	// header and class/id bytes and length
	// payload
	0xFF, 0xFF, 0x00, 0x00, 			// clearMask
	0x00, 0x00, 0x00, 0x00,				// saveMask
	0xFF, 0xFF, 0x00, 0x00,				// loadMask
	0x17,								// deviceMask
	// Checksum bytes (to-be-added)
	0x2F, 0xAE
};

uint8_t UBX_ACK_ACK[] = {
	0xB5, 0x62, 0x05, 0x01, 0x02, 0x00,	// header and class/id bytes and length
	0x00,								// classID of the acknowledged message	(to-be-added)
	0x00,								// messageID of the acknowledged message (to-be-added)
	// Checksum bytes (to-be-added)
	0x00, 0x00
};

// Position, Velocity and Time configuration
uint8_t UBX_CFG_NAV_PVT[] = {
    0xB5, 0x62,     // Sync Chars
    0x06, 0x01,     // Class and Message ID for Message Configuration
    0x08, 0x00,     // Length (8 bytes)
    0x01, 0x07,     // Class and Message ID (NAV-PVT)
    0x01, 0x00,     // Rate (1 = every navigation solution)
    0x00, 0x00, 0x00, 0x00,
    // Checksum will be calculated
	0x37, 0x37
};

/*
 * This struct consists of the values parsed from the message rectrieved from the receiver.
 * Created an identifier called NavData for ease of use
 */
typedef struct UBX_M8N_NAV_POSLLH {
	uint32_t iTOW;	// GPS time of week of the navigation epoch (ms)
	int32_t lon;	// Longitude (deg)
	int32_t lat;	// Latitude (deg)
	int32_t height;	// Height above ellipsoid (mm)
	int32_t hMSL;	// Height above mean sea level (mm)
	uint32_t hAcc;	// Horizontal accuracy estimate (mm)
	uint32_t vAcc;	// Vertical accuracy estimate (mm)
} NavData;

/* 	This function parses the payload retreived from the NAV_POSLLH message and
 * 	stores the values in the NAV_POSLLH_Data struct object. These are real world values,
 * 	description and units for these values are commented in the struct declaration above.
 *
 * 	Arguments:
 * 		pointer to buffer
 *
 * 	Refer to U-Blox 8 / U-Blox M8 Receiver desciption and protocol specification
 *	Section 32.17.16 UBX-NAV-POSLLH (0x01 0x02)
 *	Page 374
 */
void UBX_M8N_NAV_POSLLH_Parsing(uint8_t *buffer, NavData* data);

/*	This function validates the data retrieved from the GPS receiver using a checksum.
 * 	Checksum is calculated over the message, starting and including CLASS field byte up
 * 	until but not including the Checksum field bytes. After calculating the checksum from
 * 	the message payload, compare with CHECKSUM fields in buffer, if equal then data from
 * 	message is valid.
 *
 *	Arguments:
 *		pointer to buffer(message)
 *		length of buffer
 *
 *	Returns:
 *		16 bit unsigned integer / 2 byte checksum
 *		i.e. [CK_A][CK_B] where CK_A and CK_B are 8 bit unsigned integers / byte
 *
 *	Refer to U-Blox 8 / U-Blox M8 Receiver desciption and protocol specification
 *	Section 32.4 UBX Checksum
 *	Page 171
 */
uint16_t UBX_M8N_CHECKSUM(uint8_t *buffer, uint8_t buflen);


/*  This function sets a desired configuration in the GPS receiver, it does this by transmitting
 *  a UBX-CFG message to the receiver, upon receiving, the receiver will transmit either a
 *  UBX-ACK-ACK message upon successful configuration, or a UBX-ACK-NAK message upon unsuccessful
 *  configuration.
 *
 *  Arguments:
 * 		pointer to buffer(config message)
 * 		length of buffer
 *
 * 	No returns, function will terminate program upon unsuccessful configuration
 *
 *  Refer to U-Blox 8 / U-Blox M8 Receiver desciption and protocol specification
*/
void CONFIG_Transmit(uint8_t *buffer, uint16_t buflen);

/* WRITE COMMENT
 *
 */
HAL_StatusTypeDef WaitUntilI2CReady(uint32_t timeout_ms);

/*  Initialize GPS with our desired configs
 *  i.e. i2c communication with UBX message protocol
*/
void GPS_Initialization(void);

/* After writing to the GPS Data Stream Register (i.e. configs and such),
 * the receiver should store its response in the same register. It also stores
 * the length of the response on registers 0xFD (high-byte) abd 0xFE (low-byte).
 *
 * This function retrieves the length from registers 0xFD and 0xFE and returns it as a uint16_t
 */
uint16_t UBX_GET_LENGTH(void);


void UBX_Transmit(uint8_t *buffer, uint16_t buflen);

void UBX_Receive(uint8_t *buffer, uint16_t buflen);


