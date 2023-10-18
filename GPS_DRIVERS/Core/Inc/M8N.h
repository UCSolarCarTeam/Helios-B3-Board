/*
 * M8N.h
 *
 *  Created on: Oct 13, 2023
 *      Author: Nathan Ante
 */

#ifndef INC_M8N_H_
#define INC_M8N_H_

/*
 * This struct consists of the values parsed from the message rectrieved from the receiver.
 * Created an identifier called NavData for ease of use
 */
typedef struct UBX_M8N_NAV_POSLLH {
	uint32_t 	iTOW;	// GPS time of week of the navigation epoch (ms)
	int32_t 	lon;	// Longitude (deg)
	int32_t 	lat;	// Latitude (deg)
	int32_t 	height;	// Height above ellipsoid (mm)
	int32_t 	hMSL;	// Height above mean sea level (mm)
	uint32_t 	hAcc;	// Horizontal accuracy estimate (mm)
	uint32_t 	vAcc;	// Vertical accuracy estimate (mm)
} NavData;

/*
 * Declared as an extern so that main could use this
 */
extern NAV_POSLLH_Data data;

void UBX_M8N_NAV_POSLLH_Parsing(uint8_t *buffer);
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

uint8_t UBX_M8N_CHECKSUM_Check(uint8_t *buffer, uint8_t buflen);
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
 *		1 if data is valid
 *		0 if data is invalid
 *
 *	Refer to U-Blox 8 / U-Blox M8 Receiver desciption and protocol specification
 *	Section 32.4 UBX Checksum
 *	Page 171
 */

#endif /* INC_M8N_H_ */
