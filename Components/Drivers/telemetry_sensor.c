/**
 ******************************************************************************
 * File Name          : accelerometer.c
 * Description        : Testing accelerometer code.
 ******************************************************************************
*/
#include "telemetry_sensor.h"

/*
 * Check below for requirement in import
#include "3_TSL2591.h"
#include "stm32f4xx_hal_i2c.h"
*/


#define ACCELEROMETER_DEVICE_ADDR  0x68 // Important choice. Can also be 0x69
extern I2C_HandleTypeDef hi2c1;

#define ACCEL_ADDR               (ACCELEROMETER_DEVICE_ADDR << 1)  // Added pre-shifted I2C address
#define ACCEL_SENSITIVITY_4G     8192.0f   // Added LSB per g at ±4g
#define G_CONVERSION             9.80665f  // Added for the conversion g → m/s² conversion
#define GYRO_SENSITIVITY_500DPS  65.5f     // Added LSB per dps at ±500dps
/*
 * Read specified register from Lux Sensor
 */
void Accelerometer_Read_Byte(uint8_t address, uint8_t* buffer) {
	/* TODO: Implement this
	 * Use the following function to communicate with the peripheral
	 */

	//Remove bottom two lines, logic is flawed greatly
	//address = address | 0xA0; //why do this? --figure out why
	//HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, (ACCELEROMETER_DEVICE_ADDR << 1), address, I2C_MEMADD_SIZE_8BIT, buffer, 1, 1000);

	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, (ACCELEROMETER_DEVICE_ADDR<<1), address, I2C_MEMADD_SIZE_8BIT, buffer, 1, 1000);  // no address |= 0xA0


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
void Accelerometer_Write_Byte(uint8_t address, uint8_t value) {
	/* TODO: Implement this
	 * Use the following function to communicate with the peripheral
	 */

	//Removed two rows below:
	//address = address | 0xA0; //why do this?
	//HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&hi2c1, (ACCELEROMETER_DEVICE_ADDR << 1), address, I2C_MEMADD_SIZE_8BIT, &value, 1, 1000);
	HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&hi2c1, (ACCELEROMETER_DEVICE_ADDR<<1), address, I2C_MEMADD_SIZE_8BIT, &value, 1, 1000);

	//HAL_I2C_Mem_Write(hi2c, DevAddress, MemAddress, MemAddSize, pData, Size, Timeout);
	//HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&hi2c1, (ACCELEROMETER_DEVICE_ADDR << 1), address, I2C_MEMADD_SIZE_8BIT, &value, 1, 1000);
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

void telemetry_sensor_Init() {
	/*
	//Setup for sensor
	uint8_t pm1 = 0x81;
	HAL_I2C_Mem_Write(&hi2c1, ACCEL_ADDR, 0x6B, I2C_MEMADD_SIZE_8BIT, &pm1, 1, 1000);
	HAL_Delay(45);

	//Accel configuration
	uint8_t id_buffer;
	Accelerometer_Read_Byte(0x75, &id_buffer);
    if (id_buffer == 0xA9) {
    	// Init wakeup protocol the sensor
        uint8_t power_cfg = 0x00;
        Accelerometer_Write_Byte(0x6B, power_cfg);

        //Is this going to be a user select thing, or are displaying all of the options?
        uint8_t accel_cfg_2 = 0x00; //Set to ±2g range
        uint8_t accel_cfg_4 = 0x10; //Set to ±4g range
        uint8_t accel_cfg_8 = 0x20; //Set to ±8g range
        uint8_t accel_cfg_16 = 0x30; //Set to ±16g range

        Accelerometer_Write_Byte(0x1C, accel_cfg_4);
    }

    //Wake-on motion interrupt
    bool wake_on_motion_enable = false; //So since there is no condition to implement when switching from normal accel to wake on mode i created a bool to chek whether user wants wajke on to be enabled or not
    if (wake_on_motion_enable) {
    	wake_on_motion();
    }

    //Gyro configuration
	uint8_t gyro_cfg_1 = 0x00;	//±250 dps range
	uint8_t gyro_cfg_2 = 0x01;  //±500 dps range
	uint8_t gyro_cfg_3 = 0x02;  //±1000 dps range
	uint8_t gyro_cfg_4 = 0x03;  //±2000 dps range
	gyroscope_write(0x1B, gyro_cfg_1);

	//Gyro low-power mode
	uint8_t LP_mode = 0;
	HAL_I2C_Mem_Read(&hi2c1, (ACCELEROMETER_DEVICE_ADDR << 1), 0x1E, I2C_MEMADD_SIZE_8BIT, &LP_mode, 1, 1000);

	if (LP_mode & (1 << 7)) {  		//Checking if low-power gyro enabled (bit 7=1)
		//possible low-power cfgs:
		// Filter cfgs (G_AVGCFG, Ton, Averages, Noise BW)
		uint8_t avg_filter_cfg_0 = 0x00;
		uint8_t avg_filter_cfg_1 = 0x01;
		uint8_t avg_filter_cfg_2 = 0x02;
		uint8_t avg_filter_cfg_3 = 0x03;
		uint8_t avg_filter_cfg_4 = 0x04;
		uint8_t avg_filter_cfg_5 = 0x05;
		uint8_t avg_filter_cfg_6 = 0x06;
		uint8_t avg_filter_cfg_7 = 0x07;
		LP_read(0x1E, LP_mode, avg_filter_cfg_0);
	}

	//Accel wake-on-motion

	*/
	uint8_t buf;

	// Power-on reset & Clock changes
	uint8_t pm1 = 0x81;                                          // Addition
	HAL_I2C_Mem_Write(&hi2c1, ACCEL_ADDR, 0x6B, I2C_MEMADD_SIZE_8BIT, &pm1, 1, 1000);      // Added
	HAL_Delay(20);                                               // Addition: datasheet allows up to 30 ms

	// Set sample rate divider for 100 Hz uses this -> Div = (1 kHz/100 Hz) - 1 = 9
	uint8_t smplrt = 0x09;                                       // Added
	HAL_I2C_Mem_Write(&hi2c1, ACCEL_ADDR, 0x19, I2C_MEMADD_SIZE_8BIT, &smplrt, 1, 1000);   // Added

	// accelerometer to ±4 g
	uint8_t accel_cfg_4g = 0x08;                                 // Add
	Accelerometer_Write_Byte(0x1C, accel_cfg_4g);                // This was changed from 0x00 ±2 g to 0x8 ±4 g so it works better for vehicle application

	// Enable 44.8 Hz low-pass filter on accel
	uint8_t accel_cfg2 = 0x03;                                   // Added
	Accelerometer_Write_Byte(0x1D, accel_cfg2);                  // Added

	// 5) Configure gyroscope to ±500 dps (GYRO_CONFIG[1:0] = 01)
	uint8_t gyro_cfg_500 = 0x08;                                 // Addition
	gyroscope_write(0x1B, gyro_cfg_500);                         // This was changed from 0x00 which is ±250 dps to 0x01 which is ±500 dps

	// 6) Enable 41 Hz low-pass filter on gyro (CONFIG_DLPF_CFG = 0x03)
	uint8_t gyro_dlpf = 0x03;                       // Added this for the low pass filter
	gyroscope_write(0x1A, gyro_dlpf);               // Added as well
}

/*
 * Poll the data from the sensor
 */
void Accelerometer_Poll_Data(int16_t *posX, int16_t *posY, int16_t *posZ){
	/*
	 uint8_t buffer[6];

	 HAL_I2C_Mem_Read(&hi2c1, (ACCELEROMETER_DEVICE_ADDR << 1), 0x3B, I2C_MEMADD_SIZE_8BIT, buffer, 6, 1000);

	 //Get the data for x,y, and z
	 *posX = (int16_t)(buffer[0] << 8 | buffer[1]);
	 *posY = (int16_t)(buffer[2] << 8 | buffer[3]);
	 *posZ = (int16_t)(buffer[4] << 8 | buffer[5]);
	*/

	uint8_t buf[6];
	HAL_I2C_Mem_Read(&hi2c1, ACCEL_ADDR,0x3B, I2C_MEMADD_SIZE_8BIT, buf, 6,1000);    // Added

	int16_t rawX = (int16_t)(buf[0]<<8 | buf[1]);
	int16_t rawY = (int16_t)(buf[2]<<8 | buf[3]);
	int16_t rawZ = (int16_t)(buf[4]<<8 | buf[5]);

	float gX = rawX / ACCEL_SENSITIVITY_4G;                      // Added
	float gY = rawY / ACCEL_SENSITIVITY_4G;                      // Added
	float gZ = rawZ / ACCEL_SENSITIVITY_4G;                      // Added

	*posX = (int16_t)(gX * G_CONVERSION + 0.5f);                  // Added
	*posY = (int16_t)(gY * G_CONVERSION + 0.5f);                  // Addition
	*posZ = (int16_t)(gZ * G_CONVERSION + 0.5f);                  // Addition
}

/**
*  Read temperature from sensor
*
*     TEMP_degC = ((TEMP_OUT – RoomTemp_Offset)/Temp_Sensitivity) + 25degC
*          - RoomTemp_Offset  = 25.0 deg
*          - Temp_Sensitivity = 326.8
*/

void temperature_data(int16_t *temperature) { //try to fix later

    uint8_t buffer[2];
    HAL_I2C_Mem_Read(&hi2c1, (ACCELEROMETER_DEVICE_ADDR << 1), 0x41, I2C_MEMADD_SIZE_8BIT, buffer, 2, 1000);
    int16_t raw_temp = (int16_t)(buffer[0] << 8 | buffer[1]);
    float temp_degC = ((raw_temp - 25.0f) / 326.8f) + 25.0f;
    *temperature = (int16_t)(temp_degC + 0.5f);
   // *temperature = (int16_t)((raw_temp / 326.8f + 25.0f) + 0.5f);   // Round up/down to nearest int <-_ I think the math is wrong but are they constants?
}

/**
* Read gyrpscope values
*/
//
//void gyro_config() {
//	uint8_t fs_SEL = 0x00;
//	HAL_I2C_Mem_Write(&hi2c1, (ACCELEROMETER_DEVICE_ADDR << 1), 0x1B, I2C_MEMADD_SIZE_8BIT, &fs_SEL, 1, 1000);
//}

void gyroscope_write(uint8_t address, uint8_t value) {

	HAL_I2C_Mem_Write(&hi2c1, (ACCELEROMETER_DEVICE_ADDR << 1), address, I2C_MEMADD_SIZE_8BIT, &value, 1, 1000);
}

void gyroscope_data(int16_t*gyroX, int16_t *gyroY, int16_t *gyroZ) {
	/*
     uint8_t buffer[6];
     HAL_I2C_Mem_Read(&hi2c1, (ACCELEROMETER_DEVICE_ADDR << 1), 0x43, I2C_MEMADD_SIZE_8BIT, buffer, 6, 1000);
     //Get the data for x, y, and z
     *gyroX = (int16_t)((buffer[0] << 8 | buffer[1]) / 131.0f);
     *gyroY = (int16_t)((buffer[2] << 8 | buffer[3]) / 131.0f);
     *gyroZ = (int16_t)((buffer[4] << 8 | buffer[5]) / 131.0f);
     */
	uint8_t buf[6];
	HAL_I2C_Mem_Read(&hi2c1, ACCEL_ADDR, 0x43, I2C_MEMADD_SIZE_8BIT, buf, 6, 1000);  // Addition

	int16_t rawX = (int16_t)(buf[0]<<8 | buf[1]);
	int16_t rawY = (int16_t)(buf[2]<<8 | buf[3]);
	int16_t rawZ = (int16_t)(buf[4]<<8 | buf[5]);

	float dpsX = rawX / GYRO_SENSITIVITY_500DPS;                // This was changed from raw/131.0f to raw/65.5f
	float dpsY = rawY / GYRO_SENSITIVITY_500DPS;                // Add
	float dpsZ = rawZ / GYRO_SENSITIVITY_500DPS;                // Add

	*gyroX = (int16_t)(dpsX + 0.5f);                             // Add
	*gyroY = (int16_t)(dpsY + 0.5f);                             // Add
	*gyroZ = (int16_t)(dpsZ + 0.5f);                             // Add
}

void LP_read(uint8_t address, uint8_t mode_value, uint8_t cfg_value) {
	uint8_t value = mode_value | cfg_value; //gives actual value that gets written into register in next line
	HAL_I2C_Mem_Write(&hi2c1, (ACCELEROMETER_DEVICE_ADDR << 1), address, I2C_MEMADD_SIZE_8BIT, &value, 1, 1000);
}

void wake_on_motion() {
	//From page 24 of datasheet
    Accelerometer_Write_Byte(0x6B, 0x00);
    Accelerometer_Write_Byte(0x6C, 0x3F);
    Accelerometer_Write_Byte(0x1D, 0x09);
    Accelerometer_Write_Byte(0x38, 0xE0);
    Accelerometer_Write_Byte(0x1F, 0x14);
    Accelerometer_Write_Byte(0x69, 0x20);
    Accelerometer_Write_Byte(0x19, 0x05);
    Accelerometer_Write_Byte(0x6B, 0x20);
}
