/**
 ******************************************************************************
 * File Name          : main_system.cpp
 * Description        : This file acts as an interface supporting CubeIDE Codegen
    while having a clean interface for development.
 ******************************************************************************
*/
/* Includes -----------------------------------------------------------------*/
//WARNING: THIS FILE WAS CHANGED TO IMPLEMENT MOTORCONTROLTASK INTEAD OF WATCHDOGTASK
#include "SystemDefines.hpp"
#include "UARTDriver.hpp"

// Tasks
//#include "WatchdogTask.hpp"
//#include "CubeTask.hpp"
#include "DebugTask.hpp"
#include "SPI/SPI_Task.hpp"
#include "GPIO/GPIOTask.hpp"
#include "CAN/CANTxTask.hpp"
#include "MotorControlTask.hpp"   // Keep for motor control task
#include "main_system.hpp"

/* Drivers ------------------------------------------------------------------*/
namespace Driver {
    UARTDriver uart2(USART2);
}

/* Interface Functions ------------------------------------------------------------*/
/**
 * @brief Main function interface, called inside main.cpp before os initialization takes place.
*/
void run_main() {
    // Start hardware peripherals
    HAL_ADC_Start_DMA(&hadc, (uint32_t*)dma_adc_buf, ADC_BUF_LEN);
    HAL_UART_Receive_DMA(&huart2, dma_uart_command_buf, UART_BUF_LEN);
    HAL_TIM_Base_Start_IT(&htim7);
    HAL_TIM_Base_Start_IT(&htim6);

    // Init Tasks
    //MotorControlTask::Inst().InitTask();       alternative implementation to run code
    MotorControlTask::RunTask(nullptr);  // Directly create and run the task

    // Start RTOS
    osKernelStart();

    CUBE_ASSERT(false, "osKernelStart() failed");
    while (1) {
        osDelay(100);
        HAL_NVIC_SystemReset();
    }
}
