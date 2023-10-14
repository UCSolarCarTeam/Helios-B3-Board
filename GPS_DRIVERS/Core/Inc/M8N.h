/*
 * M8N.h
 *
 *  Created on: Oct 13, 2023
 *      Author: Nathan Ante
 */

#ifndef INC_M8N_H_
#define INC_M8N_H_

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
