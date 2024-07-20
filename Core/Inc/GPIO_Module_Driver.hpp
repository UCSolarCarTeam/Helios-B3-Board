/*
 * I2C_Drivers.h
 *
 *  Created on: Feb 7, 2024
 *      Author: dominic
 */
#include "stdint.h"
#include "Mutex.hpp"
#include "cmsis_os.h"
#include "main.h"

#ifndef INC_I2C_DRIVERS_H_
#define INC_I2C_DRIVERS_H_
#include "SystemDefines.hpp"

#define PCA8575_ADDR 0x20 << 1;

void PCA8575_Init(Mutex* mutex);
void PCA8575_Write(uint16_t device_addr, uint16_t data);
void PCA8575_WritePin(uint8_t device_addr, uint16_t pin, uint8_t bit_state);
uint16_t PCA8575_Read(uint8_t device_addr);
uint8_t PCA8575_ReadPin(uint8_t device_addr);

//Test Methods
uint16_t PCA8575_DataTest(uint8_t device_addr);
uint16_t PCA8575_PinTest(uint8_t device_addr);
uint16_t PCA8575_PinWaitTest(uint8_t device_addr);

#endif /* INC_I2C_DRIVERS_H_ */
