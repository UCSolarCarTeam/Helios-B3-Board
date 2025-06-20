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

uint8_t dead_common_heartbeat = 0;
uint8_t dead_motor_heartbeat = 0;
uint8_t dead_array_heartbeat = 0;
uint8_t dead_lv_heartbeat = 0;
uint8_t dead_charge_heartbeat = 0;

uint8_t hard_high_common = 0;
uint8_t hard_high_motor = 0;
uint8_t hard_high_array = 0;
uint8_t hard_high_lv = 0;
uint8_t hard_high_charge = 0;

uint8_t soft_high_common = 0;
uint8_t soft_high_motor = 0;
uint8_t soft_high_array = 0;
uint8_t soft_high_lv = 0;
uint8_t soft_high_charge = 0;

uint8_t close_common = 0;
uint8_t close_motor = 0;
uint8_t close_array = 0;
uint8_t close_lv = 0;
uint8_t close_charge = 0;

uint8_t open_common = 0;
uint8_t open_motor = 0;
uint8_t open_array = 0;
uint8_t open_lv = 0;
uint8_t open_charge = 0;





ts_contactor_state contactor_array[5] = {{0}, {0}, {0}, {0}, {0}};
ts_orion_info orion_info = {0};

void contactorHeartbeatCANPopulate(CANMsg* msg, te_contactor contactor)
{
    contactor_array[contactor].heartbeat++;
    msg->extendedID = 0x200 + contactor;
    msg->DLC = 2;
    msg->data[0] = (contactor_array[contactor].heartbeat >> 0) & 0xFF;
    msg->data[1] = (contactor_array[contactor].heartbeat >> 8) & 0xFF;
}

void contactorDeadHeartbeatCANPopulate(CANMsg* msg, te_contactor contactor)
{
    contactor_array[contactor].heartbeat = 0;
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

void orionPackInfoCANPopulate(CANMsg* msg)
{
//    orion_info.packCurrent = 0;
//    orion_info.packVoltage = 1100;
    msg->extendedID = 0x302;
    msg->DLC = 8;
    msg->data[0] = (orion_info.packCurrent >> 0) & 0xFF;
    msg->data[1] = (orion_info.packCurrent >> 8) & 0xFF;
    msg->data[2] = (orion_info.packVoltage >> 0) & 0xFF;
    msg->data[3] = (orion_info.packVoltage >> 8) & 0xFF;
    msg->data[4] = (orion_info.packStateOfCharge >> 0) & 0xFF;
    msg->data[5] = (orion_info.packAmphours >> 0) & 0xFF;
    msg->data[6] = (orion_info.packAmphours >> 8) & 0xFF;
    msg->data[7] = (orion_info.packDepthOfDischarge >> 0) & 0xFF;
}

void orionTempInfoCANPopulate(CANMsg* msg)
{
//    orion_info.highTemperature = 30;
//    orion_info.lowTemperature = 30;
//    orion_info.AverageTemperature = 30;
//    orion_info.internalTemperature = 30;
    msg->extendedID = 0x304;
    msg->DLC = 8;
    msg->data[0] = orion_info.highTemperature & 0xFF;
    msg->data[1] = orion_info.highThermistorID & 0xFF;
    msg->data[2] = orion_info.lowTemperature & 0xFF;
    msg->data[3] = orion_info.lowThermistorID & 0xFF;
    msg->data[4] = orion_info.AverageTemperature & 0xFF;
    msg->data[5] = orion_info.internalTemperature & 0xFF;
    msg->data[6] = orion_info.fanSpeed & 0xFF;
    msg->data[7] = orion_info.requestedFanSpeed& 0xFF;
}

void orionCellVoltagesCANPopulate(CANMsg* msg)
{
//    orion_info.lowCellVoltage = 39000; /**/
//    orion_info.highCellVoltage = 40000;
//    orion_info.averageCellVoltage = 39500;
    msg->extendedID = 0x305;
    msg->DLC = 8;
    msg->data[0] = (orion_info.lowCellVoltage >> 0) & 0xFF;
    msg->data[1] = (orion_info.lowCellVoltage >> 8) & 0xFF;
    msg->data[2] = (orion_info.lowCellVoltageID >> 0) & 0xFF;
    msg->data[3] = (orion_info.highCellVoltage >> 0) & 0xFF;
    msg->data[4] = (orion_info.highCellVoltage >> 8) & 0xFF;
    msg->data[5] = (orion_info.highCellVoltageID >> 0) & 0xFF;
    msg->data[6] = (orion_info.averageCellVoltage >> 0) & 0xFF;
    msg->data[7] = (orion_info.averageCellVoltage >> 8) & 0xFF;
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
    	contactor_array[COMMON].contactor_closed = 0;
    	close_common = 0;
        contactorStatusCANPopulate(&msg, COMMON);
        CUBE_PRINT("Sent Common Status \n");
        break;
    case MOTOR_BOARD_STATUS:
    	contactor_array[MOTOR].contactor_closed = 0;
    	close_motor = 0;
        contactorStatusCANPopulate(&msg, MOTOR);
        CUBE_PRINT("Sent Motor Status \n");
        break;
    case ARRAY_BOARD_STATUS:
    	contactor_array[ARRAY].contactor_closed = 0;
    	close_array = 0;
        contactorStatusCANPopulate(&msg, ARRAY);
        CUBE_PRINT("Sent Array Status \n");
        break;
    case LV_BOARD_STATUS:
    	contactor_array[LV].contactor_closed = 0;
    	close_lv = 0;
        contactorStatusCANPopulate(&msg, LV);
        CUBE_PRINT("Sent LV Status \n");
        break;
    case CHARGE_BOARD_STATUS:
    	contactor_array[CHARGE].contactor_closed = 0;
    	close_charge = 0;
        contactorStatusCANPopulate(&msg, CHARGE);
        CUBE_PRINT("Sent Charge Status \n");
        break;
    case PACK_INFO:
        orion_info.packCurrent = 0;
        orion_info.packVoltage = 1100;
        orionPackInfoCANPopulate(&msg);
        CUBE_PRINT("Sent Pack Info \n");
        break;
    case TEMPERATURE_INFO:
        orion_info.highTemperature = 30;
        orion_info.lowTemperature = 30;
        orion_info.AverageTemperature = 30;
        orion_info.internalTemperature = 30;
        orionTempInfoCANPopulate(&msg);
        CUBE_PRINT("Sent Temp Info \n");
        break;
    case CELL_VOLTAGES:
        orion_info.lowCellVoltage = 39000; /**/
        orion_info.highCellVoltage = 40000;
        orion_info.averageCellVoltage = 39500;
        orionCellVoltagesCANPopulate(&msg);
        CUBE_PRINT("Sent Cell Voltages \n");
        break;

        /* dead contactor heartbeat */
    case DEAD_COMMON_HEARTBEAT:
    	dead_common_heartbeat = 1;
        contactorDeadHeartbeatCANPopulate(&msg, COMMON);
        CUBE_PRINT("Sent Dead COmmon Heartbeat \n");
        break;
    case DEAD_MOTOR_HEARTBEAT:
    	dead_motor_heartbeat = 1;
        contactorDeadHeartbeatCANPopulate(&msg, MOTOR);
        CUBE_PRINT("Sent Dead Motor Heartbeat \n");
        break;
    case DEAD_ARRAY_HEARTBEAT:
    	dead_array_heartbeat = 1;
        contactorDeadHeartbeatCANPopulate(&msg, ARRAY);
        CUBE_PRINT("Sent Dead Array Heartbeat \n");
        break;
    case DEAD_LV_HEARTBEAT:
    	dead_lv_heartbeat = 1;
        contactorDeadHeartbeatCANPopulate(&msg, LV);
        CUBE_PRINT("Sent Dead LV Heartbeat \n");
        break;
    case DEAD_CHARGE_HEARTBEAT:
    	dead_charge_heartbeat = 1;
        contactorDeadHeartbeatCANPopulate(&msg, CHARGE);
        CUBE_PRINT("Sent Dead Motor Heartbeat \n");
        break;

        /* bad cell voltages */
    case HARD_HIGH_CELL:
        orion_info.lowCellVoltage = 39000; /**/
        orion_info.highCellVoltage = 46000;
        orion_info.averageCellVoltage = 39500;

    	orionCellVoltagesCANPopulate(&msg);
    	CUBE_PRINT("Sent hard high Cell Voltages \n");
    	break;

    case SOFT_HIGH_CELL:
        orion_info.lowCellVoltage = 39000; /**/
        orion_info.highCellVoltage = 43000;
        orion_info.averageCellVoltage = 39500;

    	orionCellVoltagesCANPopulate(&msg);
    	CUBE_PRINT("Sent soft high Cell Voltages \n");
    	break;

    case HARD_LOW_CELL:
		orion_info.lowCellVoltage = 30000; /**/
		orion_info.highCellVoltage = 40000;
		orion_info.averageCellVoltage = 39500;

		orionCellVoltagesCANPopulate(&msg);
		CUBE_PRINT("Sent hard low Cell Voltages \n");
		break;

	case SOFT_LOW_CELL:
		orion_info.lowCellVoltage = 36000; /**/
		orion_info.highCellVoltage = 40000;
		orion_info.averageCellVoltage = 39500;

		orionCellVoltagesCANPopulate(&msg);
		CUBE_PRINT("Sent soft low Cell Voltages \n");
		break;


		/* bad currents */
	case HARD_HIGH_COMMON:
		hard_high_common = 1;
        orion_info.packCurrent = 3200;
        orion_info.packVoltage = 1100;
        orionPackInfoCANPopulate(&msg);
        CUBE_PRINT("Sent hard high common Info \n");
        break;

	case SOFT_HIGH_COMMON:
		soft_high_common = 1;
        orion_info.packCurrent = 2950;
        orion_info.packVoltage = 1100;
        orionPackInfoCANPopulate(&msg);
        CUBE_PRINT("Sent soft high common Info \n");
        break;

	case HARD_HIGH_MOTOR:
		hard_high_motor = 1;
		contactor_array[MOTOR].line_current = 3200;
        contactorStatusCANPopulate(&msg, MOTOR);
        CUBE_PRINT("Sent hard high Motor Status \n");
        break;

	case SOFT_HIGH_MOTOR:
		soft_high_motor = 1;
		contactor_array[MOTOR].line_current = 2950;
        contactorStatusCANPopulate(&msg, MOTOR);
        CUBE_PRINT("Sent soft high Motor Status \n");
        break;

	case HARD_HIGH_ARRAY:
		contactor_array[ARRAY].line_current = 3200;
        contactorStatusCANPopulate(&msg, ARRAY);
        CUBE_PRINT("Sent hard high array Status \n");
        break;

	case SOFT_HIGH_ARRAY:
		contactor_array[ARRAY].line_current = 2950;
        contactorStatusCANPopulate(&msg, ARRAY);
        CUBE_PRINT("Sent soft high Array Status \n");
        break;

	case HARD_HIGH_LV:
		contactor_array[LV].line_current = 3200;
        contactorStatusCANPopulate(&msg, LV);
        CUBE_PRINT("Sent hard high LV Status \n");
        break;

	case SOFT_HIGH_LV:
		contactor_array[LV].line_current = 2950;
        contactorStatusCANPopulate(&msg, LV);
        CUBE_PRINT("Sent soft high LV Status \n");
        break;

	case HARD_HIGH_CHARGE:
		contactor_array[CHARGE].line_current = 3200;
        contactorStatusCANPopulate(&msg, CHARGE);
        CUBE_PRINT("Sent hard high charge Status \n");
        break;

	case SOFT_HIGH_CHARGE:
		contactor_array[CHARGE].line_current = 2950;
        contactorStatusCANPopulate(&msg, CHARGE);
        CUBE_PRINT("Sent soft high Charge Status \n");
        break;


        /* bad temps */
    case HARD_MAX_TEMP:
        orion_info.highTemperature = 46;
        orion_info.lowTemperature = 30;
        orion_info.AverageTemperature = 30;
        orion_info.internalTemperature = 30;
        orionTempInfoCANPopulate(&msg);
        CUBE_PRINT("Sent Hard Max Temp \n");
        break;

    case SOFT_MAX_TEMP:
        orion_info.highTemperature = 42;
        orion_info.lowTemperature = 30;
        orion_info.AverageTemperature = 30;
        orion_info.internalTemperature = 30;
        orionTempInfoCANPopulate(&msg);
        CUBE_PRINT("Sent Soft Max Temp \n");
        break;

    case HARD_MIN_TEMP:
        orion_info.highTemperature = 30;
        orion_info.lowTemperature = -3;
        orion_info.AverageTemperature = 30;
        orion_info.internalTemperature = 30;
        orionTempInfoCANPopulate(&msg);
        CUBE_PRINT("Sent Hard Min Temp \n");
        break;

    case SOFT_MIN_TEMP:
        orion_info.highTemperature =30;
        orion_info.lowTemperature = 4;
        orion_info.AverageTemperature = 30;
        orion_info.internalTemperature = 30;
        orionTempInfoCANPopulate(&msg);
        CUBE_PRINT("Sent Soft Min Temp \n");
        break;

        /* CLOSE CONTACTORS */
    case CLOSE_COMMON:
    	close_common = 1;
    	contactor_array[COMMON].contactor_closed = 1;
        contactorStatusCANPopulate(&msg, COMMON);
        CUBE_PRINT("Sent close common \n");
        break;

    case CLOSE_MOTOR:
    	close_motor = 1;
    	contactor_array[MOTOR].contactor_closed = 1;
        contactorStatusCANPopulate(&msg, MOTOR);
        CUBE_PRINT("Sent close motor \n");
        break;
    case CLOSE_ARRAY:
    	close_array = 1;
    	contactor_array[ARRAY].contactor_closed = 1;
        contactorStatusCANPopulate(&msg, ARRAY);
        CUBE_PRINT("Sent Close array \n");
        break;
    case CLOSE_LV:
    	close_lv = 1;
    	contactor_array[LV].contactor_closed = 1;
        contactorStatusCANPopulate(&msg, LV);
        CUBE_PRINT("Sent close LV \n");
        break;
    case CLOSE_CHARGE:
    	close_charge = 1;
    	contactor_array[CHARGE].contactor_closed = 1;
        contactorStatusCANPopulate(&msg, CHARGE);
        CUBE_PRINT("Sent close charge \n");
        break;

    case OPEN_COMMON:
    	close_common = 0;
    	open_common = 1;
    	contactor_array[COMMON].contactor_closed = 0;
        contactorStatusCANPopulate(&msg, COMMON);
        CUBE_PRINT("Sent open common \n");
        break;

    case OPEN_MOTOR:
    	close_motor = 0;
    	open_motor = 1;
    	contactor_array[MOTOR].contactor_closed = 0;
        contactorStatusCANPopulate(&msg, MOTOR);
        CUBE_PRINT("Sent open motor \n");
        break;
    case OPEN_ARRAY:
    	close_array = 0;
    	open_array = 1;
    	contactor_array[ARRAY].contactor_closed = 0;
        contactorStatusCANPopulate(&msg, ARRAY);
        CUBE_PRINT("Sent open array \n");
        break;
    case OPEN_LV:
    	close_lv = 0;
    	open_lv = 1;
    	contactor_array[LV].contactor_closed = 0;
        contactorStatusCANPopulate(&msg, LV);
        CUBE_PRINT("Sent open LV \n");
        break;
    case OPEN_CHARGE:
    	close_charge = 0;
    	open_charge = 1;
    	contactor_array[CHARGE].contactor_closed = 0;
        contactorStatusCANPopulate(&msg, CHARGE);
        CUBE_PRINT("Sent open charge \n");
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
