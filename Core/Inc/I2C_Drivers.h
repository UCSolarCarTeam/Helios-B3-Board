/*
 * I2C_Drivers.h
 *
 *  Created on: Feb 7, 2024
 *      Author: dominic
 */
#include "stdint.h";

#ifndef INC_I2C_DRIVERS_H_
#define INC_I2C_DRIVERS_H_

#define PCA8575_ADDR 0x20 << 1;

uint16_t pins = 0;

void I2C_init();
void I2C_write_pin(uint8_t device_addr, uint8_t pin, uint8_t bit_state);
void I2C_write(uint8_t device_addr, uint16_t data);
uint16_t I2C_read(uint8_t device_addr);

#endif /* INC_I2C_DRIVERS_H_ */
