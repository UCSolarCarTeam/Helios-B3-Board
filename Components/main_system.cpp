/**
 ******************************************************************************
 * File Name          : main_system.cpp
 * Description        : This file acts as an interface supporting CubeIDE Codegen
    while having a clean interface for development.
 ******************************************************************************
*/
/* Includes -----------------------------------------------------------------*/
#include "SystemDefines.hpp"
#include "UARTDriver.hpp"

// Tasks
#include "CubeTask.hpp"
#include "I2C_Drivers.h"

/* Drivers ------------------------------------------------------------------*/
namespace Driver {
    UARTDriver uart2(USART2);
}

/* Interface Functions ------------------------------------------------------------*/
/**
 * @brief Main function interface, called inside main.cpp before os initialization takes place.
*/
void run_main() {
    // Init Tasks
    CubeTask::Inst().InitTask();

    // Print System Boot Info : Warning, don't queue more than 10 prints before scheduler starts
    CUBE_PRINT("\n-- CUBE SYSTEM --\n");
    CUBE_PRINT("System Reset Reason: [TODO]\n"); //TODO: System reset reason can be implemented via. Flash storage
    CUBE_PRINT("Current System Heap Use: %d Bytes\n", xPortGetFreeHeapSize());
    CUBE_PRINT("Lowest Ever Heap Size: %d Bytes\n\n", xPortGetMinimumEverFreeHeapSize());

    // Start the Scheduler
    // Guidelines:
    // - Be CAREFUL with race conditions after osKernelStart
    // - All uses of new and delete should be closely monitored after this point
    //osKernelStart();

    // Should never reach here
    //CUBE_ASSERT(false, "osKernelStart() failed");

    while (1)
    {
        uint8_t device_addr = 0x20; // Device address
        uint16_t data_to_write = 0xABCD; // Example data to write

        // Write data to PCA8575
        PCA8575_Write(device_addr, data_to_write);

        // Example of setting a pin state
        uint8_t pin_to_write = 2; // Example pin number
        uint8_t bit_state = 1; // Example bit state (0 or 1)
        PCA8575_WritePin(device_addr, pin_to_write, bit_state);

        // Example of reading data from PCA8575
        uint16_t data_read = PCA8575_Read(device_addr);

        // Example of reading a pin state
        uint8_t pin_to_read = 3; // Example pin number
        uint8_t pin_state = PCA8575_ReadPin(device_addr, pin_to_read);

        osDelay(1000);
        //HAL_NVIC_SystemReset();
    }
}
