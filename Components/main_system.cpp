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
#include "CAN.h"
#include "main.h"

// Tasks
#include "WatchdogTask.hpp"
#include "CubeTask.hpp"
#include "DebugTask.hpp"
#include "SPI/SPI_Task.hpp"
#include "GPIO/GPIOTask.hpp"
#include "CANTx/CANTxTask.hpp"
#include "CANRx/CanRxTask.hpp"
#include "MotorControl/MotorControlTask.hpp"

/* Drivers ------------------------------------------------------------------*/
namespace Driver {
    UARTDriver uart2(USART2);
}

/* CAN peripheral struct object-----------------------------------------------*/
CANPeripheral peripheral0 = {
    .CS_PORT = CS_CAN_N_GPIO_Port,
    .CS_PIN = CS_CAN_N_Pin,
    .hspi = SystemHandles::CAN_SPI
};

/* Interface Functions ------------------------------------------------------------*/
/**
 * @brief Main function interface, called inside main.cpp before os initialization takes place.
*/
void run_main() {
    // Init Tasks
    WatchdogTask::Inst().InitTask();
    CubeTask::Inst().InitTask();
    DebugTask::Inst().InitTask();
    SPI_Task::Inst().InitTask();
    CANRxTask::Inst().InitTask();
    CANTxTask::Inst().InitTask();
    GPIOTask::Inst().InitTask();
    MotorControlTask::Inst().InitTask();

    // Init Peripherals
    ConfigureCANSPI(&peripheral0);

    // Print System Boot Info : Warning, don't queue more than 10 prints before scheduler starts
    CUBE_PRINT("\n-- CUBE SYSTEM --\n");
    CUBE_PRINT("System Reset Reason: [TODO]\n"); //TODO: System reset reason can be implemented via. Flash storage
    CUBE_PRINT("Current System Free Heap: %d Bytes\n", xPortGetFreeHeapSize());
    CUBE_PRINT("Lowest Ever Free Heap: %d Bytes\n\n", xPortGetMinimumEverFreeHeapSize());

    // Start the Scheduler
    // Guidelines:
    // - Be CAREFUL with race conditions after osKernelStart
    // - All uses of new and delete should be closely monitored after this point
    osKernelStart();

    // Should never reach here
    CUBE_ASSERT(false, "osKernelStart() failed");

    while (1)
    {
        osDelay(100);
        HAL_NVIC_SystemReset();
    }
}
