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

typedef struct {
    uint16_t heartbeat;
    uint8_t precharger_closed;
    uint8_t precharger_closing;
    uint8_t precharger_error;
    uint8_t contactor_closed;
    uint8_t contactor_closing;
    uint8_t contactor_error;
    uint16_t line_current;
    uint16_t charge_current;
} ts_contactor_state;

ts_contactor_state contactor_array[5] = {{0}, {0}, {0}, {0}, {0}};

void contactorHeartbeatCANPopulate(CANMsg* msg, te_contactor contactor)
{
    contactor_array->heartbeat++;
    msg->extendedID = 0x200 + contactor;
    msg->DLC = 2;
    msg->data[0] = (contactor_array[contactor].heartbeat >> 0) & 0xFF;
    msg->data[1] = (contactor_array[contactor].heartbeat >> 8) & 0xFF;
}

void contactorStatusCANPopulate(CANMsg* msg, te_contactor contactor)
{
    msg->extendedID = 0x210 + contactor;
    msg->DLC = 4;
    msg->data[0]   = contactor_array[contactor].precharger_closed  ? 0x01 : 0x0;
    msg->data[0]  |= contactor_array[contactor].precharger_closing ? 0x02 : 0x0;
    msg->data[0]  |= contactor_array[contactor].precharger_error   ? 0x04 : 0x0;
    msg->data[0]  |= contactor_array[contactor].contactor_closed   ? 0x08 : 0x0;
    msg->data[0]  |= contactor_array[contactor].contactor_closing  ? 0x10 : 0x0;
    msg->data[0]  |= contactor_array[contactor].contactor_error    ? 0x20 : 0x0;
    msg->data[0]  |= (contactor_array[contactor].line_current & 0x003) << 6;
    msg->data[1]  =  (contactor_array[contactor].line_current & 0x3FC) >> 2;
    msg->data[2]  =  (contactor_array[contactor].line_current & 0xC00) >> 10;
    msg->data[2]  |= (contactor_array[contactor].charge_current & 0x03F) >> 0;
    msg->data[3]  = (contactor_array[contactor].charge_current & 0xFC0) >> 6;
}  

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

    uint8_t u8_data = 0;
    uint16_t u16_data = 0;
    uint16_t u16_acceleration = 0;
    uint16_t u16_braking = 0;
    uint32_t u32_data = 0;

    // Handle command based on address/type
    switch (static_cast<CAN_TX_COMMANDS>(cm.GetTaskCommand()))
    {
    case LIGHTS_INPUT:
        msg.extendedID = 0x610;
        msg.DLC = 1;
        u8_data = GPIOTask::Inst().LightsInputs();
        msg.data[0] = u8_data;
        CUBE_PRINT("Sent Lights Input command\n");
        break;

    case DIGITAL_INPUTS:
        msg.extendedID = 0x611;
        msg.DLC = 2;

        u16_data = GPIOTask::Inst().DigitalInputs();

        msg.data[1] = (u16_data >> 8) & 0x01; // High byte, only 9 bits
        msg.data[0] = u16_data & 0xFF;        // Low byte, 8 bits

        CUBE_PRINT("Sent DIGITAL INPUTS command\n");
        break;

    case ANALOG_INPUTS:
        msg.extendedID = 0x612;
        msg.DLC = 3;

        u16_acceleration = SPI_Task::Inst().getAccelerationReading_P() & 0x0FFF; // Mask to ensure 12 bits
        u16_braking = SPI_Task::Inst().getBrakingReading_P() & 0x0FFF;           // Mask to ensure 12 bits

        // Pack the 12-bit acceleration and 12-bit braking into a 24-bit structure
        u32_data = (u16_acceleration & 0x0FFF) | ((u16_braking & 0x0FFF) << 12);

        // Split u32_data into bytes and assign to msg.data[]
        msg.data[0] = static_cast<uint8_t>(u32_data & 0xFF);         // Extract the first 8 bits (bits 0-7)
        msg.data[1] = static_cast<uint8_t>((u32_data >> 8) & 0xFF);  // Extract the next 8 bits (bits 8-15)
        msg.data[2] = static_cast<uint8_t>((u32_data >> 16) & 0xFF); // Extract the next 8 bits (bits 16-23)

        CUBE_PRINT("Sent Analog Inputs command\n");
        break;

    case LIGHTS_STATUS_BASE:
        msg.extendedID = 0x620;
        msg.DLC = 1;
        u8_data = GPIOTask::Inst().LightStatus();
        msg.data[0] = u8_data;
        CUBE_PRINT("Sent Lights Status command\n");
        break;

    case HEARTBEAT:
        msg.extendedID = 0x600;
        msg.DLC = 1;
        msg.data[0] = 1;
        CUBE_PRINT("Sent Heartbeat \n");
        break;
    case COMMON_BOARD_HEARTBEAT:
        contactorHeartbeatCANPopulate(&msg, COMMON);
        CUBE_PRINT("Sent Common Heartbeat \n");
        break;
    case MOTOR_BOARD_HEARTBEAT:
        contactorHeartbeatCANPopulate(&msg, MOTOR);
        CUBE_PRINT("Sent Motor Heartbeat \n");
        break;
    case ARRAY_BOARD_HEARTBEAT:
        contactorHeartbeatCANPopulate(&msg, ARRAY);
        CUBE_PRINT("Sent Array Heartbeat \n");
        break;
    case LV_BOARD_HEARTBEAT:
        contactorHeartbeatCANPopulate(&msg, LV);
        CUBE_PRINT("Sent LV Heartbeat \n");
        break;
    case CHARGE_BOARD_HEARTBEAT:
        contactorHeartbeatCANPopulate(&msg, CHARGE);
        CUBE_PRINT("Sent Charge Heartbeat \n");
        break;
    case COMMON_BOARD_STATUS:
        contactorStatusCANPopulate(&msg, COMMON);
        CUBE_PRINT("Sent Common Status \n");
        break;
    case MOTOR_BOARD_STATUS:
        contactorStatusCANPopulate(&msg, MOTOR);
        CUBE_PRINT("Sent Motor Status \n");
        break;
    case ARRAY_BOARD_STATUS:
        contactorStatusCANPopulate(&msg, ARRAY);
        CUBE_PRINT("Sent Array Status \n");
        break;
    case LV_BOARD_STATUS:
        contactorStatusCANPopulate(&msg, LV);
        CUBE_PRINT("Sent LV Status \n");
        break;
    case CHARGE_BOARD_STATUS:
        contactorStatusCANPopulate(&msg, CHARGE);
        CUBE_PRINT("Sent Charge Status \n");
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
