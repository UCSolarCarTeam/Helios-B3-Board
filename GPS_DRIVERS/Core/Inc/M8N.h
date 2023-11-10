/*
 * M8N.h
 *
 *  Created on: Oct 13, 2023
 *      Author: Nathan Ante
 */


#ifndef INC_M8N_H_
#define INC_M8N_H_

#include<stdint.h>

/*-------------- Private Macros --------------*/
#define GPS_DEVICE_ADDRESS (0x42 << 1)	// GPS device address is 0x42, left-shifted for STM32 uses 7-bit address
#define GPS_DATA_REGISTER 0xFF		    // register address for GPS data stream

// In case for "random access" read from i2c (refer to page 38 of data sheet)
#define GPS_DATA_LENGTH_HIGH 0xFD	// register address for GPS data length (high byte)
#define GPS_DATA_LENGTH_LOW 0xFE	// register address for GPS data length (low byte)

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
void CONFIG_Transmit(uint8_t *buffer, uint8_t buflen);

/*  Initialize GPS with our desired configs
 *  i.e. i2c communication with UBX message protocol
*/
void GPS_Initialization(void);

#endif /* INC_M8N_H_ */
