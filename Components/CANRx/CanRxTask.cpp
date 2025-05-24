/**
 ******************************************************************************
 * File Name          : CANRxTask.cpp
 * Description        : Task for receiving CAN messages
 ******************************************************************************
 */

#include "CanRxTask.hpp"

// Maybe add a header file
CANPeripheral peripheral2 = {
    .CS_PORT = CS_CAN_N_GPIO_Port,
    .CS_PIN = CS_CAN_N_Pin,
    .hspi = SystemHandles::CAN_SPI};

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
    ConfigureCANSPI(&peripheral2);

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
         * 3. Mutex the SPI Peripheral
         *      Mutex
         * 4. Handle buffer
         */

        //  Wait forever for a command on interrupt
        Command cm;
        qEvtQueue->ReceiveWait(cm);

        // Process the command
        HandleCommand(cm);

        cm.Reset();
    }
}

void CANRxTask::HandleCommand(Command &cm)
{
    uint32_t id = 0;
    uint8_t dlc = 0;
    uint8_t data[8] = {0};

    switch (static_cast<CAN_RX_COMMANDS>(cm.GetTaskCommand()))
    {
    case CAN_INTERRUPT_HAPPENED:
        CUBE_PRINT("Received a CAN RX Message by interrupt\n");
        break;
    case CAN_RX0_INTERRUPT_HAPPENED:
        receiveCANMessage(0, &id, &dlc, data, &peripheral2);
        CUBE_PRINT("CAN RX0 Message:\n");
        CUBE_PRINT_CAN_MESSAGE(id, dlc, data);
        break;
    case CAN_RX1_INTERRUPT_HAPPENED:
        receiveCANMessage(1, &id, &dlc, data, &peripheral2);
        CUBE_PRINT("CAN RX1 Message:\n");
        CUBE_PRINT_CAN_MESSAGE(id, dlc, data);
        break;
    default:
        CUBE_PRINT("CANRXTask - Received unsupported command: %d\n", cm.GetCommand());
        break;
    }
}

// Handle CAN_INT Callback here ?
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//    // NOTE: Can Implement Call back on RXBUF0 and RXBUF1 and decode the message accordingly.
//    if (GPIO_Pin == CAN_INT_Pin)
//    {
//        // Handle or Event Flag into CPP Task
//        Command canInterruptHappenedCommandFlag = Command(TASK_SPECIFIC_COMMAND, CAN_INTERRUPT_HAPPENED);
//        CANRxTask::Inst()
//            .GetCAN_RX_QUEUE()
//            ->SendFromISR(canInterruptHappenedCommandFlag);
//    }
//    else if (GPIO_Pin == CAN_RX0BF_Pin)
//    {
//        Command canInterruptHappenedCommandFlag = Command(TASK_SPECIFIC_COMMAND, CAN_RX0_INTERRUPT_HAPPENED);
//        CANRxTask::Inst()
//            .GetCAN_RX_QUEUE()
//            ->SendFromISR(canInterruptHappenedCommandFlag);
//    }
//    else if (GPIO_Pin == CAN_RX1BF_Pin)
//    {
//        Command canInterruptHappenedCommandFlag = Command(TASK_SPECIFIC_COMMAND, CAN_RX1_INTERRUPT_HAPPENED);
//        CANRxTask::Inst()
//            .GetCAN_RX_QUEUE()
//            ->SendFromISR(canInterruptHappenedCommandFlag);
//    }
//}

//Helper function to print CAN message
void CUBE_PRINT_CAN_MESSAGE(uint32_t id, uint8_t dlc, uint8_t *data)
{
    CUBE_PRINT("  ID: 0x%08X\n", id);
    CUBE_PRINT("  DLC: %u\n", dlc);
    CUBE_PRINT("  Data: ");
    for (uint8_t i = 0; i < dlc; ++i)
    {
        CUBE_PRINT("%02X ", data[i]);
    }
}
