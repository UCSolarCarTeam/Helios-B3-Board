/*
 * I2C_Drivers.c
 *
 *  Created on: Feb 7, 2024
 *      Author: dominic
 */
#include <GPIO.h>
#include "cmsis_os.h"

osMutexId_t i2cMutexHandle;
const osMutexAttr_t i2cMutexAttributes = {
  .name = "I2CMutex"
};

I2C_HandleTypeDef hi2c1;

/**
 * @brief Initializes the I2C1 peripheral
*/
void PCA8575_Init() {
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

/**
  * @brief Writes a 16-bit value to the pins
  * @param device_addr: I2C device address, 7-bit
  * @param data: 16-bit value to write
  * @retval None
  */
void PCA8575_Write(uint8_t device_addr, uint16_t data){
    pins = data;

    uint8_t payload[2] = {pins | 0xFF, (pins | 0xFF00) >> 8};
    osMutexAcquire(i2cMutexHandle, osWaitForever);
    HAL_I2C_Master_Transmit(&hi2c1, device_addr, payload, 2, 1000);
    osMutexRelease(i2cMutexHandle);
}

/**
  * @brief Sets a pin to a given state
  * @param device_addr: I2C device address, 7-bit
  * @param pin: pin number to write to
  * @param bit_state: The state to set pin to, 0 or 1
  * @retval None
  */
void PCA8575_WritePin(uint8_t device_addr, uint8_t pin, uint8_t bit_state){
    uint16_t data = PCA8575_Read(device_addr);
    uint16_t mask = 1 << pin;

    if (bit_state == 1) {
        // Set the bit to 1
        data |= mask;
    } else if (bit_state == 0) {
        // Set the bit to 0
        data &= ~mask;
    } else {
        // how did we get neither a 0 or 1?
    }

    uint8_t payload[2] = {data | 0xFF, (data | 0xFF00) >> 8};

    osMutexAcquire(i2cMutexHandle, osWaitForever);
    HAL_I2C_Master_Transmit(&hi2c1, device_addr, payload, 2, 1000);
    osMutexRelease(i2cMutexHandle);
}

/**
  * @brief Reads a 16-bit value from the pins
  * @param device_addr: I2C device address, 7-bit
  * @retval 16-bit value read from the pins
  */
uint16_t PCA8575_Read(uint8_t device_addr){
    uint8_t buffer[2];

    osMutexAcquire(i2cMutexHandle, osWaitForever);
    HAL_I2C_Master_Receive(&hi2c1, device_addr, buffer, 2, 1000);
    osMutexRelease(i2cMutexHandle);

    uint16_t data = buffer[0] | (buffer[1] << 8);

    return data;
}

/**
  * @brief Reads a 16-bit value from the pins
  * @param device_addr: I2C device address, 7-bit
  * @param pin: Pin to read
  * @retval 1-bit value read from the specified pin
  */
uint8_t PCA8575_ReadPin(uint8_t device_addr, uint8_t pin) {
    uint8_t data = PCA8575_Read();
    return (data >> pin) & 0x01;
}

/**
  * @brief Sets the direction of the pins
  * @param direction: 16-bit value to set the directions pins
  * @retval None
  */
void PCA8575_SetDirection(uint8_t device_addr, uint16_t direction) {
    PCA8575_Write(device_addr, direction);
}