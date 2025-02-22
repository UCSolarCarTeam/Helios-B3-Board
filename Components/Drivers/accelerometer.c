/**
 ******************************************************************************
 * File Name          : accelerometer.c
 * Description        : Testing accelerometer code.
 ******************************************************************************
*/

#include "accelerometer.h"

/*
 * Check below for requirement in import
#include "3_TSL2591.h"
#include "stm32f4xx_hal_i2c.h"
*/

I2C_HandleTypeDef hi2c3;
#define ACCELEROMETER_DEVICE_ADDR  0x68 // Important choice. Can also be 0x69
/*
 * Read specified register from Lux Sensor
 */
void accelerometer_Read_Byte(uint8_t address, uint8_t* buffer) {
	/* TODO: Implement this
	 * Use the following function to communicate with the peripheral
	 */

	address = address | 0xA0;
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c3, (ACCELEROMETER_DEVICE_ADDR << 1), address, I2C_MEMADD_SIZE_8BIT, buffer, 1, 1000);

	// HAL_I2C_Mem_Read(hi2c, DevAddress, MemAddress, MemAddSize, pData, Size, Timeout);
	/* uint16_t DevAddress: I2C address of the device.
	 * uint16_t MemAddress: Memory/register address within the device.
	 * uint16_t MemAddSize: Size of the memory address (8-bit or 16-bit).
	 * uint8_t *pData: Pointer to the data buffer that will receive the data.
	 * uint16_t Size: Amount of data to read.
	 * uint32_t Timeout: Timeout duration.
	 */


}


/*
 * Write to specific register in Lux Sensor
 */
void accelerometer_Write_Byte(uint8_t address, uint8_t value) {
	/* TODO: Implement this
	 * Use the following function to communicate with the peripheral
	 */

	address = address | 0xA0;
	uint8_t data = value;

	//HAL_I2C_Mem_Write(hi2c, DevAddress, MemAddress, MemAddSize, pData, Size, Timeout);
	HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&hi2c3, (ACCELEROMETER_DEVICE_ADDR << 1), address, I2C_MEMADD_SIZE_8BIT, &value, 1, 1000);
	/* uint16_t  DevAddress: I2C address of the device.
	 * uint16_t MemAddress: Memory/register address within the device.
	 * uint16_t MemAddSize: Size of the memory address (typically I2C_MEMADD_SIZE_8BIT or I2C_MEMADD_SIZE_16BIT).
	 * uint8_t *pData: Pointer to the data to write.
	 * uint16_tSize: Amount of data to write.
	 * uint32_t Timeout: Timeout duration.
	 */
}


/*
 * Initializes the accelerometer Sensor with desired configurations
 */
void accelerometer_Init() {
	uint8_t id_buffer;
	accelerometer_Read_Byte(0x75, &id_buffer);

    if (id_buffer == 0x98) {
    	// Init wakeup protocol the sensor
        uint8_t power_cfg = 0x00;
        accelerometer_Write_Byte(0x6B, power_cfg);

        // Set to ±8g range
        uint8_t accel_cfg = 0x10;
        accelerometer_Write_Byte(0x1C, accel_cfg);
    }
}

/*
 * Poll the data from the sensor
 */
void accelerometer_Poll_Data(int16_t *posX, int16_t *posY, int16_t *posZ){
	 uint8_t buffer[6];

	 HAL_I2C_Mem_Read(&hi2c3, (ACCELEROMETER_DEVICE_ADDR << 1), 0x3B, I2C_MEMADD_SIZE_8BIT, buffer, 6, 1000);

	 //Get the data for x,y, and z
	 *posX = (int16_t)(buffer[0] << 8 | buffer[1]);
	 *posY = (int16_t)(buffer[2] << 8 | buffer[3]);
	 *posZ = (int16_t)(buffer[4] << 8 | buffer[5]);

}
