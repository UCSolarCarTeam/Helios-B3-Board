//Added pragma because it was in CAN
#pragma once

//Includes only main and stdint, stdint may not be needed will check
#include "main.h"
#include <stdint.h>
#include <stdbool.h>

//Program is in C, need this to allow it to run with C++
#ifdef __cplusplus
extern "C" {
#endif

void Accelerometer_Read_Byte(uint8_t address, uint8_t* buffer);
void Accelerometer_Write_Byte(uint8_t address, uint8_t value);
void telemetry_sensor_Init();
void Accelerometer_Poll_Data(int16_t *posX, int16_t *posY, int16_t *posZ);
void temperature_data(int16_t *temperature);
void gyroscope_write(uint8_t address, uint8_t value);
void gyroscope_data(int16_t*gyroX, int16_t *gyroY, int16_t *gyroZ);
void LP_read(uint8_t address, uint8_t mode_value, uint8_t cfg_value);
void wake_on_motion();

#define ACCELEROMETER_DEVICE_ADDR b110100X




#ifdef __cplusplus
}
#endif
// https://www.codesdope.com/blog/article/making-a-queue-using-linked-list-in-c/
