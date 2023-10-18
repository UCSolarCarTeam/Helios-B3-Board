/*
 * M8N.c
 *
 *  Created on: Oct 13, 2023
 *      Author: Nathan Ante
 */
#include "M8N.h"

NAV_POSLLH_Data data;

/*	This function validates the data retrieved from the GPS receiver using a checksum.
 * 	Checksum is calculated over the message, starting and including CLASS field byte up
 * 	until but not including the Checksum field bytes.
 *
 * 	Essentially, don't include the first and last 2 bytes of the buffer.
 *
 *	After calculating the checksum values CK_A and CK_B, compare with the buffer/message's
 *	checksum bytes, compare CK_A with buffer's second last byte, and compare CK_B with last byte.
 *	If both are equal to each of their pairs, then data retrieved from GPS receiver is valid.
 *
 *	Check documentation if you need more info, details in header file
 */
uint8_t UBX_M8N_CHECKSUM_Check(uint8_t* buffer, uint8_t buflen) {

	// These values will be used to compare with the buffer's
	uint8_t CK_A = 0, CK_B = 0;

	// loop to go through buffer payload
	// start at index 2 since first two bytes of buffer are not included in checksum calculation
	// do not include last 2 bytes of buffer since they are also not included in checksum calculation
	for (int i = 2; i < buflen - 2; i++) {
		CK_A = CK_A + buffer[i];
		CK_B = CK_B + CK_A;
	}

	// After calculating checksum, compare with checksum bytes from buffer
	// Return 1 if both are equal to buffer checksum, return 0 if not
	return ((CK_A == buffer[buflen - 2]) && (CK_B == buffer[buflen - 1]));

}


/* This function parses the payload from a NAV_POSLLH message
 * The payload is in little endian format, so left shift the bytes
 * Follow reference from driver and protocol description
 */
void UBX_M8N_NAV_POSLLH_Parsing(uint8_t *buffer, NAV_POSLLH_Data* data) {
	data->iTOW = buffer[9]<<24 | buffer[8]<<16 | buffer[7]<<8 | buffer[6];
	data->lon = buffer[13]<<24 | buffer[12]<<16 | buffer[11]<<8 | buffer[10];
	data.lan = buffer[17]<<24 | buffer[16]<<16 | buffer[15]<<8 | buffer[14];
	data->height = buffer[21]<<24 | buffer[20]<<16 | buffer[19]<<8 | buffer[18];
	data->hMSL = buffer[25]<<24 | buffer[24]<<16 | buffer[23]<<8 | buffer[22];
	data->hAcc = buffer[29]<<24 | buffer[28]<<16 | buffer[27]<<8 | buffer[26];
	data->vAcc = buffer[33]<<24 | buffer[32]<<16 | buffer[31]<<8 | buffer[30];
}
