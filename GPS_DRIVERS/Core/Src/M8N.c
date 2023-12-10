/*
 * M8N.c
 *
 *  Created on: Oct 13, 2023
 *      Author: Nathan Ante
 */
#include "M8N.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;

/*------------------------------- GPS Functions -------------------------------*/
uint8_t UBX_CFG_PRT[] = {
	0xB5, 0x62, 0x06, 0x00, 0x14, 0x00,		// header and class/id bytes and length
	// payload
	0x00, 0x00,								// Port Identifier & reserved
	0x00, 0x00,								// txReady settings
	0x42, 0x00, 0x00, 0x00,  				// I2C mode flags
	0x00,  0x00, 0x00, 0x00,				// reserved
	0x01, 0x00,								// inProtoMask
	0x01, 0x00, 							// outProtoMask
	0x00, 0x00,								// extended TX timeout
	0x00, 0x00,								// reserved
	// Checksum bytes (to-be-added)
	0xA0, 0x96
};

uint8_t UBX_CFG_MSG[] = {
	0xB5, 0x62, 0x06, 0x01,	0x08, 0x00,	// header and class/id bytes and length
	// payload
	0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	// Checksum bytes (to-be-added)
	0x13, 0xBF
};

uint8_t UBX_CFG_RATE[] = {
	0xB5, 0x62, 0x06, 0x08,	0x06, 0x00,	// header and class/id bytes and length
	// payload
	0xE8, 0x03, 						// measRate(ms)
	0x01, 0x00, 						// navRate(cycles
	0x01, 0x00,							// timeRef - 1:GPS time
	// Checksum bytes (to-be-added)
	0x01, 0x39
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

/*------------------------------- GPS Functions -------------------------------*/

void UBX_Transmit(uint8_t *buffer, uint16_t buflen) {
	HAL_StatusTypeDef hal = HAL_I2C_Mem_Write(&hi2c1, GPS_DEVICE_ADDRESS, GPS_DATA_REGISTER, 1, buffer, buflen, 100);
	if (hal != HAL_OK) {
		printf("HAL Status: %d | I2C Error: %d | Class and ID: %#X %#X\r\n", hal, hi2c1.ErrorCode, buffer[2], buffer[3]);
	}
}

void UBX_Receive(uint8_t *buffer, uint16_t buflen) {
	HAL_StatusTypeDef hal = HAL_I2C_Mem_Read(&hi2c1, GPS_DEVICE_ADDRESS, GPS_DATA_REGISTER, 1, buffer, buflen, 100);
		if (hal != HAL_OK) {
			printf("HAL Status: %d | I2C Error: %d\r\n", hal, hi2c1.ErrorCode);
	}
}



/*	This function validates the data retrieved from the GPS receiver using a checksum.
 * 	Checksum is calculated over the message, starting and including CLASS field byte up
 * 	until but not including the Checksum field bytes.
 *
 * 	Essentially, don't include the first and last 2 bytes of the buffer.
 *
 *	After calculating the checksum values CK_A and CK_B, return it as a 16 bit unsigned integer
 *  Where CK_A is the high byte, and CK_B is the low byte
 *
 *	Check documentation if you need more info, details in header file
 */
uint16_t UBX_M8N_CHECKSUM(uint8_t* buffer, uint8_t buflen) {

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
	// return ((CK_A == buffer[buflen - 2]) && (CK_B == buffer[buflen - 1]));

	// return 2 byte checksum
	return ((CK_A<<8) | CK_B);
}

/* This function parses the payload from a NAV_POSLLH message
 * The payload is in little endian format, so left shift the bytes
 * Follow reference from driver and protocol description
 */
void UBX_M8N_NAV_POSLLH_Parsing(uint8_t *buffer, NavData* data) {
	data->iTOW = buffer[9]<<24 | buffer[8]<<16 | buffer[7]<<8 | buffer[6];
	data->lon = buffer[13]<<24 | buffer[12]<<16 | buffer[11]<<8 | buffer[10];
	data->lat = buffer[17]<<24 | buffer[16]<<16 | buffer[15]<<8 | buffer[14];
	data->height = buffer[21]<<24 | buffer[20]<<16 | buffer[19]<<8 | buffer[18];
	data->hMSL = buffer[25]<<24 | buffer[24]<<16 | buffer[23]<<8 | buffer[22];
	data->hAcc = buffer[29]<<24 | buffer[28]<<16 | buffer[27]<<8 | buffer[26];
	data->vAcc = buffer[33]<<24 | buffer[32]<<16 | buffer[31]<<8 | buffer[30];
}

/* This function sets a desired configuration in the GPS receiver
 * It takes in a pointer to the configuration message buffer, as well as its size
 * Calls Error_Handler() if something goes wrong
*/
void CONFIG_Transmit(uint8_t* buffer, uint16_t buflen) {
	HAL_StatusTypeDef hal;    				// HAL return status
	printf("Size of config: %d\r\n", buflen);

	// transmit desired CONFIG to GPS receiver
	hal = HAL_I2C_Mem_Write(&hi2c1, GPS_DEVICE_ADDRESS, GPS_DATA_LENGTH_HIGH, 1, buffer, buflen, HAL_MAX_DELAY);
	if (hal != HAL_OK) {
		printf("CONFIG transmit went wrong\r\n");
	}
	// get the length of the CONFIG message response
	uint16_t message_length = UBX_GET_LENGTH();
	printf("Message Length: %d\r\n", message_length);

	if (message_length != 0) {
		// create a buffer of
		uint8_t config_response[message_length];
		config_response[0] = GPS_DATA_REGISTER;

		// Recieve the CONFIG response
		hal = HAL_I2C_Master_Receive(&hi2c1, GPS_DEVICE_ADDRESS | 0x01, config_response, message_length, HAL_MAX_DELAY);
		if (hal != HAL_OK) {
			printf("CONFIG response went wrong!\r\n");
			printf("Error code: %08lX\r\n", hi2c1.ErrorCode);
		} else {
			printf("Length: %d | Headers: %X %X |Class: %X | ID: %X | rest: %X %X %X %X %X %X\r\n", message_length, config_response[0], config_response[1], config_response[2], config_response[3], config_response[4], config_response[5], config_response[6], config_response[7], config_response[8], config_response[9]);
			// see if response was a ACK message
		}
	}
}

uint16_t UBX_GET_LENGTH() {
	uint8_t ubx_length[2];
	HAL_StatusTypeDef hal = HAL_I2C_Mem_Read(&hi2c1, GPS_DEVICE_ADDRESS | 0x01, GPS_DATA_LENGTH_HIGH, 1, ubx_length, sizeof(ubx_length), 100);
	if (hal != HAL_OK) {
		printf("Read for length went wrong");
	}

	printf("High: %X | Low: %X\r\n",ubx_length[0], ubx_length[1]);
	// return length as uint16_t
	return ((ubx_length[0] << 8) | (ubx_length[1]));
}

void GPS_Initialization(void) {
	HAL_StatusTypeDef hal;
	hal = HAL_I2C_Master_Transmit(&hi2c1, GPS_DEVICE_ADDRESS, UBX_CFG_CFG, sizeof(UBX_CFG_CFG), HAL_MAX_DELAY);
	if (hal != HAL_OK) {
		// something went wrong with transmit (exit)
		printf("UBX-CFG-CFG went wrong!\r\n");
	}
	HAL_Delay(2000);

	printf("Starting MSG\r\n");
	CONFIG_Transmit(UBX_CFG_MSG, sizeof(UBX_CFG_MSG)/sizeof(UBX_CFG_MSG[0]));
	HAL_Delay(1000);

	printf("Starting PRT\r\n");
	CONFIG_Transmit(UBX_CFG_PRT, sizeof(UBX_CFG_PRT)/sizeof(UBX_CFG_PRT[0]));
	HAL_Delay(1000);

//	uint8_t test2[] = "msg CONFIG STARTS here\r\n";
//	HAL_UART_Transmit(&huart1, test2, sizeof(test2), HAL_MAX_DELAY);

//	uint8_t test3[] = "rate CONFIG STARTS here\r\n";
//	HAL_UART_Transmit(&huart1, test3, sizeof(test3), HAL_MAX_DELAY);
	printf("Starting RATE\r\n");
	CONFIG_Transmit(UBX_CFG_RATE, sizeof(UBX_CFG_RATE)/sizeof(UBX_CFG_RATE[0]));
	HAL_Delay(1000);
}

/*------------------------------- Extra Functions for testing purposes -------------------------------*/
