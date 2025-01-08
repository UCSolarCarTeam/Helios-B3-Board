/**
 ******************************************************************************
 * File Name          : CANRxTask.cpp
 * Description        : Task for receiving CAN messages
 ******************************************************************************
 */

#include "CanRxTask.hpp"

/**
 * @brief Constructor for CANTxTask
 */
CANRxTask::CANRxTask() : Task(CAN_TX_TASK_QUEUE_DEPTH_OBJS)
{
}

/**
 * @brief Initialize the CANTxTask
 */
void CANRxTask::InitTask()
{
    // Make sure the task is not already initialized
    CUBE_ASSERT(rtTaskHandle == nullptr, "Cannot initialize CAN RX task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)CANRxTask::RunTask,
                    (const char *)"CANRxTask",
                    (uint16_t)CAN_RX_TASK_STACK_DEPTH_WORDS,
                    (void *)this,
                    (UBaseType_t)CAN_RX_TASK_PRIORITY,
                    (TaskHandle_t *)&rtTaskHandle);

    CUBE_ASSERT(rtValue == pdPASS, "CANRxTask::InitTask() - xTaskCreate() failed");
}

void CANRxTask::Run(void *pvParams)
{
    // ConfigureCANSPI(&peripheral1);

    while (1)
    {
        /***
         * MCP2510: https://usw.365.altium.com/librarycomponentsapi/api/v1/References/79FC5B82-F317-4482-BD19-9D6F8666BE78 
         * Plan:
         * On buffer interrupt add to queue
         * While(1) Task processes Queue
         * Later Optimization: Add Mutex to
         *
         * 1. Detect RX Interrupt RXBUFF
         *      - Set interrupts RXBuff pg 21
         *      - RXBUFF0 and RXBUFF1 .
         * 2. Queue buffer contents
         *      - CAN_RX_QUEUE.SendFromISR()
         *      - Parse data into command
         *            receiveCANMessage()
         * 3. Mutex the SPI Peripheral?
         *      Mutex
         * 4. Handle buffer
         */

        //TODO: Update TX with new Addresses 

    }
}

// Handle CAN_INT Callback here ?
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == CAN_INT_Pin)
    {
        // Handle or Event Flag into CPP Task
        Command canInterruptHappenedCommandFlag = Command(TASK_SPECIFIC_COMMAND, CAN_INTERRUPT_HAPPENED);
            CANRxTask::Inst()
                .GetCAN_RX_QUEUE()
                ->SendFromISR(canInterruptHappenedCommandFlag);    
    }
}
