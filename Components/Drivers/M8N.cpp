/*
 * M8N.c
 *
 *  Created on: May 24, 2025
 *      Author: jazebzafar
 */

#include <M8N.hpp>
#include "main.h"
#include <string.h>
#include "CubeDefines.hpp"

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;

/*------------------------------- GPS Functions -------------------------------*/

/*------------------------------- GPS Functions -------------------------------*/


void UBX_Transmit(uint8_t *buffer, uint16_t buflen) {
	HAL_StatusTypeDef hal = HAL_I2C_Mem_Write(&hi2c1, GPS_DEVICE_ADDRESS, GPS_DATA_REGISTER, 1, buffer, buflen, 100);
	if (hal != HAL_OK) {
		//CUBE_PRINT("HAL Status: %d | I2C Error: %d | Class and ID: %#X %#X\r\n", hal, hi2c1.ErrorCode, buffer[2], buffer[3]);
	}else{
		CUBE_PRINT("[ ^ ]UBX Transmit Successful\n");
	}
}

void UBX_Receive(uint8_t *buffer, uint16_t buflen) {
	HAL_StatusTypeDef hal = HAL_I2C_Mem_Read(&hi2c1, GPS_DEVICE_ADDRESS, GPS_DATA_REGISTER, 1, buffer, buflen, 100);
		if (hal != HAL_OK) {
			//CUBE_PRINT("HAL Status: %d | I2C Error: %\r\n", hal, hi2c1.ErrorCode);
	}else{
		CUBE_PRINT("[ ^ ]UBX Receive Successful\n");
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
	return ((CK_A << 8) | CK_B);
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
	HAL_StatusTypeDef hal;  // HAL return status

	// transmit desired CONFIG to GPS receiver
	hal = HAL_I2C_Master_Transmit(&hi2c1, GPS_DEVICE_ADDRESS, buffer, buflen, HAL_MAX_DELAY);
			//(&hi2c1, GPS_DEVICE_ADDRESS, GPS_DATA_LENGTH_HIGH, 1, buffer, buflen, HAL_MAX_DELAY)


	if (hal != HAL_OK) {
		CUBE_PRINT("CONFIG transmit went wrong\r\n");
		CUBE_PRINT("	[i]0x%x\r\n", hal);
	}
	// get the length of the CONFIG message response
	uint16_t message_length = UBX_GET_LENGTH();
	CUBE_PRINT("Message Length: %d\r\n", message_length);

	if (message_length != 0) {
		// create a buffer of
		uint8_t config_response[message_length];
		config_response[0] = GPS_DATA_REGISTER;

		// Recieve the CONFIG response
		hal = HAL_I2C_Master_Receive(&hi2c1, GPS_DEVICE_ADDRESS | 0x01, config_response, message_length, HAL_MAX_DELAY);
		if (hal != HAL_OK) {
			CUBE_PRINT("CONFIG response went wrong!\r\n");
			CUBE_PRINT("Error code: %08lX\r\n", hi2c1.ErrorCode);
		} else {
			CUBE_PRINT("Length: %d | Headers: %X %X |Class: %X | ID: %X | rest: %X %X %X %X %X %X\r\n", message_length, config_response[0], config_response[1], config_response[2], config_response[3], config_response[4], config_response[5], config_response[6], config_response[7], config_response[8], config_response[9]);
			// see if response was a ACK message
		}
	}
}

uint16_t UBX_GET_LENGTH() {
	uint8_t ubx_length[2];
	HAL_StatusTypeDef hal = HAL_I2C_Mem_Read(&hi2c1, GPS_DEVICE_ADDRESS | 0x01, GPS_DATA_LENGTH_HIGH, 1, ubx_length, sizeof(ubx_length), 100);
	if (hal != HAL_OK) {
		CUBE_PRINT("Read for length went wrong");
	}

	CUBE_PRINT("High: %X | Low: %X\r\n",ubx_length[0], ubx_length[1]);
	// return length as uint16_t
	return ((ubx_length[0] << 8) | (ubx_length[1]));
}

void GPS_Initialization(void) {
	HAL_StatusTypeDef hal;

	// Poll till hal is no longer busy

	CUBE_PRINT("Starting MSG\r\n");
	CONFIG_Transmit(UBX_CFG_MSG, sizeof(UBX_CFG_MSG)/sizeof(UBX_CFG_MSG[0]));
	CUBE_PRINT("Starting PRT\r\n");
	CONFIG_Transmit(UBX_CFG_PRT, sizeof(UBX_CFG_PRT)/sizeof(UBX_CFG_PRT[0]));
	CUBE_PRINT("Starting RATE\r\n");
	CONFIG_Transmit(UBX_CFG_RATE, sizeof(UBX_CFG_RATE)/sizeof(UBX_CFG_RATE[0]));
	CUBE_PRINT("Starting NAV\r\n");
	CONFIG_Transmit(UBX_CFG_NAV_PVT, sizeof(UBX_CFG_NAV_PVT)/sizeof(UBX_CFG_NAV_PVT[0]));

	//hal = HAL_I2C_Master_Transmit(&hi2c1, GPS_DEVICE_ADDRESS, UBX_CFG_CFG, sizeof(UBX_CFG_CFG), HAL_MAX_DELAY);

	osDelay(500);
}

/*------------------------------- Extra Functions for testing purposes -------------------------------*/



