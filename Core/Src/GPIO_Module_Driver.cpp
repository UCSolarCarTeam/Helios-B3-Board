/*
 * I2C_Drivers.c
 *
 *  Created on: Feb 7, 2024
 *      Author: dominic
 * 
 *  Modified on: June, 11, 2024
 *      By: Alend Maci
 */
#include <GPIO_Module_Driver.hpp>
#include <stm32l152xe.h>

extern I2C_HandleTypeDef hi2c1;
uint16_t pins;
Mutex* i2cMutex;

/**
 * @brief Initializes the I2C1 peripheral
*/
void PCA8575_Init(Mutex* mutex) {
	i2cMutex = mutex;
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    // Checks if the I2C1 peripheral is already initialized
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        CUBE_PRINT("[!] Error initializing I2C1!\n");
    }
}

/**
  * @brief Writes a 16-bit value to the pins
  * @param device_addr: I2C device address, 7-bit
  * @param data: 16-bit value to write
  * @retval None
  */
void PCA8575_Write(uint8_t device_addr, uint16_t data){
    pins = data;
    uint8_t payload[2] = {(uint8_t)(pins | 0xFF), (uint8_t)((pins | 0xFF00) >> 8)};
    i2cMutex->Lock();
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, device_addr, payload, 2, 1000);
    if (status != HAL_OK) {
        CUBE_PRINT("[!] Error writing to address %s\n", device_addr);
    }
    i2cMutex->Unlock();
}

/**
  * @brief Sets a pin to a given state
  * @param device_addr: I2C device address, 7-bit
  * @param pin: pin number to write to
  * @param bit_state: The state to set pin to, 0 or 1
  * @retval None
  */
void PCA8575_WritePin(uint8_t device_addr, uint16_t pin, uint8_t bit_state){
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

    uint8_t payload[2] = {(uint8_t)(data | 0xFF), (uint8_t)((data | 0xFF00) >> 8)};

    i2cMutex->Lock();
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, device_addr, payload, 2, 1000);
    if (status != HAL_OK) {
        CUBE_PRINT("[!] Error setting pin %s\n", device_addr);
    }
    i2cMutex->Unlock();
}

/**
  * @brief Reads a 16-bit value from the pins
  * @param device_addr: I2C device address, 7-bit
  * @retval 16-bit value read from the pins
  */
uint16_t PCA8575_Read(uint8_t device_addr){
    uint8_t buffer[2];

    i2cMutex->Lock();
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive(&hi2c1, device_addr, buffer, 2, 1000);
    if (status != HAL_OK) {
        CUBE_PRINT("[!] Error reading from address %s\n", device_addr);
    }

    i2cMutex->Unlock();

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
    uint8_t data = PCA8575_Read(device_addr);
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


//-----------------------Test Methods----------------------------------

/**
  * @brief Tests the PCA8575 by writing and reading a 16-bit value
  * @param device_addr: I2C device address, 7-bit
  * @retval 16-bit value read from the pins
  */
uint16_t PCA8575_DataTest(uint8_t device_addr) {
    CUBE_PRINT("Setting up test...\n");
    PCA8575_Write(device_addr, 0x0000); // Clear the pins

    CUBE_PRINT("-Testing Data Read/Write-\n");
    uint16_t data_to_write = 0xA5A5; // test data
    for(int i = 0; i < 16; i++) {
      PCA8575_Write(device_addr, data_to_write); // Write data
      uint16_t data_read = PCA8575_Read(device_addr); // Read data
      if (data_read != data_to_write) {
        CUBE_PRINT("[!] Data read: %d doesn't match data sent: %d\n", data_read, data_to_write);
      }else{
        CUBE_PRINT("[✓] Data written to PCA8575: %d\n", data_to_write); // Print data written
        CUBE_PRINT("[✓] Data read from PCA8575: %d\n", data_read); // Print data read
      }
      data_to_write = data_to_write << 1; // Shift data to test reading
      osDelay(1000);
    }
    return 0;
}

/**
  * @brief Tests the PCA8575 by writing and reading a 16-bit value
  * @param device_addr: I2C device address, 7-bit
  * @retval 1-bit value read from the specified pin
  */
uint16_t PCA8575_PinTest(uint8_t device_addr) {
    CUBE_PRINT("Setting Up Test...\n");
    PCA8575_Write(device_addr, 0x0000); // Clear the pins

    CUBE_PRINT("-Testing Pin Read/Write-\n");
    for (int test_pin = 0; test_pin < 16; test_pin++) {
        PCA8575_WritePin(device_addr, test_pin, 1); // Write the bit
        uint8_t bit_read = PCA8575_ReadPin(device_addr, test_pin); // Read the bit
        if(bit_read != test_pin){
          CUBE_PRINT("[!] Pin %d didn't read as %d\n", test_pin, bit_read);
        }else{
          CUBE_PRINT("[✓]Pin %d read as %d\n", test_pin, bit_read); // Print the bit
        }
        osDelay(1000);
    }

    CUBE_PRINT("All pins should be 1: %d\n", PCA8575_Read(device_addr)); // Print the data
    return 0;
}

/**
  * @brief Tests the PCA8575 by writing and reading a 16-bit value
  * @param device_addr: I2C device address, 7-bit
  * @retval 1-bit value read from the specified pin
  */
uint16_t PCA8575_PinWaitTest(uint8_t device_addr) {
    CUBE_PRINT("Setting Up Test...\n");
    PCA8575_Write(device_addr, 0x0000); // Clear the pins
    uint8_t test_pin = 2;

    CUBE_PRINT("Testing Pin Read\n");
    uint8_t bit_read = PCA8575_ReadPin(device_addr, test_pin); // Read the bit
    CUBE_PRINT("Pin %d read as %d\n", test_pin, bit_read); // Print the bit

    while(PCA8575_ReadPin(device_addr, test_pin) == bit_read) {
      osDelay(250);
    }

    CUBE_PRINT("Pin %d read as %d\n", test_pin, bit_read); // Print the bit
    osDelay(1000);

    return 0;
}
