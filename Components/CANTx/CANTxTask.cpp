/**
 ******************************************************************************
 * File Name          : CANTxTask.cpp
 * Description        : Task for transmitting CAN messages
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>

#include "CANTxTask.hpp"

#include "CAN.h"
#include "CANRegisters.h"

#include "GPIO/GPIOTask.hpp"
#include "SPI/SPI_Task.hpp"

CANPeripheral peripheral1 = {
    .CS_PORT = CS_CAN_N_GPIO_Port,
    .CS_PIN = CS_CAN_N_Pin,
    .hspi = SystemHandles::CAN_SPI};

/**
 * @brief Constructor for CANTxTask
 */
CANTxTask::CANTxTask() : Task(CAN_TX_TASK_QUEUE_DEPTH_OBJS)
{
}

/**
 * @brief Initialize the CANTxTask
 */
void CANTxTask::InitTask()
{
    // Make sure the task is not already initialized
    CUBE_ASSERT(rtTaskHandle == nullptr, "Cannot initialize CAN Tx task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)CANTxTask::RunTask,
                    (const char *)"CANTxTask",
                    (uint16_t)CAN_TX_TASK_STACK_DEPTH_WORDS,
                    (void *)this,
                    (UBaseType_t)CAN_TX_TASK_PRIORITY,
                    (TaskHandle_t *)&rtTaskHandle);

    CUBE_ASSERT(rtValue == pdPASS, "CANTxTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief Instance Run loop for the CAN_TX Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void CANTxTask::Run(void *pvParams)
{
    ConfigureCANSPI(&peripheral1);
    while (1)
    {

        // Wait forever for a command
        Command cm;
        qEvtQueue->ReceiveWait(cm);

        // Process the command
        HandleCommand(cm);

        cm.Reset();
    }
}

/**
 * @brief Handle a command
 * @param cm Command to handle
 */
void CANTxTask::HandleCommand(Command &cm)
{
    CANMsg msg;
    msg.ID = 0;
    msg.DLC = 1; // Assuming DLC of 1 for each command; adjust as needed.
    // msg.data[0] = 1; // Example data; set according to command specifics.

    uint8_t u8_data = 0;
    uint16_t u16_acceleration = 0;
    uint16_t u16_braking = 0;
    uint32_t u32_data = 0;
    // Handle command based on address/type
    switch (static_cast<CAN_TX_COMMANDS>(cm.GetTaskCommand()))
    {
    case LIGHTS_INPUT_BASE:
        msg.extendedID = 0x701;
        msg.DLC = 1;
        u8_data = GPIOTask::Inst().LightsInputsBase();
        msg.data[0] = u8_data;
        CUBE_PRINT("Sent Lights Input command\n");
        break;

    case DRIVER_BASE:
        msg.extendedID = 0x703;
        msg.DLC = 4;

        // ASSUMPTION: Returning only the P of the pedal readings. 12 bits returned even though SPI IC returns 10, following the comm sheet
        u16_acceleration = SPI_Task::Inst().getAccelerationReading_P() & 0x0FFF; // Mask to ensure 12 bits
        u16_braking = SPI_Task::Inst().getBrakingReading_P() & 0x0FFF;           // Mask to ensure 12 bits

        // Combine the two 12-bit values into a 24-bit variable
        u8_data = GPIOTask::Inst().DriverBase();

        // Combine all values into a single 32-bit variable
        u32_data = (static_cast<uint32_t>(u16_braking) << 12) | // Shift braking to bits 12–23
                   (static_cast<uint32_t>(u8_data) << 24) |     // Shift u8_data to bits 24–31
                   u16_acceleration;                            // Place acceleration in bits 0–11

        // Split u32_data into bytes and assign to msg.data[]
        msg.data[0] = static_cast<uint8_t>(u32_data & 0xFF);         // Extract the first 8 bits (bits 0-7)
        msg.data[1] = static_cast<uint8_t>((u32_data >> 8) & 0xFF);  // Extract the next 8 bits (bits 8-15)
        msg.data[2] = static_cast<uint8_t>((u32_data >> 16) & 0xFF); // Extract the next 8 bits (bits 16-23)
        msg.data[3] = static_cast<uint8_t>((u32_data >> 24) & 0xFF); // Extract the last 8 bits (bits 24-31)

        CUBE_PRINT("Sent Driver command\n");
        break;

    case LIGHTS_STATUS_BASE:
        msg.extendedID = 0x711;
        msg.DLC = 1;
        u8_data = GPIOTask::Inst().LightStatus();
        msg.data[0] = u8_data;
        CUBE_PRINT("Sent Lights Status command\n");
        break;

    default:
        CUBE_PRINT("CANRXTask - Received unsupported command: %d\n", cm.GetCommand());
        break;
    }

    if (msg.extendedID != 0)
    {
        sendExtendedCANMessage(&msg, &peripheral1);
    }

    cm.Reset(); // Clear command data after processing
}
