/*
 * I2C_Drivers.c
 *
 *  Created on: Feb 7, 2024
 *      Author: dominic
 */
#include "I2C_Drivers.h"
#include "cmsis_os.h"

osMutexId_t i2cMutexHandle;
const osMutexAttr_t i2cMutexAttributes = {
  .name = "I2CMutex"
};

/**
  * @brief Sets a pin to a given state
  * @param device_addr: I2C device address, 7-bit
  * @param pin: pin number to write to
  * @param bit_state: The state to set pin to, 0 or 1
  * @retval None
  */
void I2C_write_pin(uint8_t device_addr, uint8_t pin, uint8_t bit_state){
    uint16_t mask = 1 << pin;

    if (bit_state == 1) {
        // Set the bit to 1
        pins |= mask;
    } else if (bit_state == 0) {
        // Set the bit to 0
        pins &= ~mask;
    } else {
        // how did we get neither a 0 or 1?
    }

    uint8_t payload[2] = {pins | 0xFF, (pins | 0xFF00) >> 8};

    osMutexAcquire(i2cMutexHandle, osWaitForever);
    HAL_I2C_Master_Transmit(&hi2c1, device_addr, payload, 2, 1000);
    osMutexRelease(i2cMutexHandle);
}

/**
  * @brief Writes a 16-bit value to the pins
  * @param device_addr: I2C device address, 7-bit
  * @param data: 16-bit value to write
  * @retval None
  */
void I2C_write(uint8_t device_addr, uint16_t data)
{
    pins = data;

    uint8_t payload[2] = {pins | 0xFF, (pins | 0xFF00) >> 8};
    osMutexAcquire(i2cMutexHandle, osWaitForever);
    HAL_I2C_Master_Transmit(&hi2c1, device_addr, payload, 2, 1000);
    osMutexRelease(i2cMutexHandle);

}

/**
  * @brief Reads a 16-bit value from the pins
  * @param device_addr: I2C device address, 7-bit
  * @retval 16-bit value read from the pins
  */
uint16_t I2C_read(uint8_t device_addr)
{
    uint8_t buffer[2];

    osMutexAcquire(i2cMutexHandle, osWaitForever);
    HAL_I2C_Master_Receive(&hi2c1, device_addr, buffer, 2, 1000);
    osMutexRelease(i2cMutexHandle);

    uint16_t data = buffer[1] | (buffer[0] << 8);

    return data;
}
