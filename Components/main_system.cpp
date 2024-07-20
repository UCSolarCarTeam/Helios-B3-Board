/**
 ******************************************************************************
 * File Name          : main_system.cpp
 * Description        : This file acts as an interface supporting CubeIDE Codegen
    while having a clean interface for development.
 ******************************************************************************
*/
/* Includes -----------------------------------------------------------------*/
#include "GPIO_Module_Driver.hpp"
#include "SystemDefines.hpp"
#include "UARTDriver.hpp"
#include "Mutex.hpp"

// Tasks
#include "CubeTask.hpp"

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

    Mutex* i2cMutex = new Mutex();
    PCA8575_Init(i2cMutex);
    while (1)
    {
        uint16_t device_addr = 0x21; // Device address, 0x20 for the PCA85

        // Test Data Read/Write
        PCA8575_DataTest(device_addr);

        // Test Pin Read/Write
        PCA8575_PinTest(device_addr);

        osDelay(1000);
        //HAL_NVIC_SystemReset();
    }
}
