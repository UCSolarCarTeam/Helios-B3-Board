/**
 ******************************************************************************
 * File Name          : CANTxTask.hpp
 * Description        :
 ******************************************************************************
 */
#ifndef CUBE_SYSTEM_CAN_TX_TASK_HPP_
#define CUBE_SYSTEM_CAN_TX_TASK_HPP_

/* Includes ------------------------------------------------------------------*/
#include "main_system.hpp"
#include "Task.hpp"
#include "Command.hpp"
#include "CubeUtils.hpp"
#include <cstring>

#include "SystemDefines.hpp"
#include "CubeDefines.hpp"

typedef enum
{
    COMMON = 0,
    MOTOR,
    ARRAY,
    LV,
    CHARGE
} te_contactor;

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

typedef struct {
    uint16_t packCurrent; /* 0.1A */
    uint16_t packVoltage; /* 0.1V */
    uint8_t packStateOfCharge; /* 0.5% */
    uint16_t packAmphours; /* 0.1Ah */
    uint8_t packDepthOfDischarge; /* 0.5% */
    uint8_t highTemperature; /* 1C */
    uint8_t highThermistorID; /* # */
    uint8_t lowTemperature; /* 1C */
    uint8_t lowThermistorID; /* # */
    uint8_t AverageTemperature; /* 1C */
    uint8_t internalTemperature; /* 1C */
    uint8_t fanSpeed; /* # */
    uint8_t requestedFanSpeed; /* # */
    uint16_t lowCellVoltage; /* 0.1mV */
    uint16_t lowCellVoltageID; /* # */
    uint16_t highCellVoltage; /* 0.1mV */
    uint16_t highCellVoltageID; /* # */
    uint16_t averageCellVoltage; /* 0.1mV */
} ts_orion_info;

/* Enums ------------------------------------------------------------------*/
enum CAN_TX_COMMANDS
{
    LIGHTS_INPUT, // Command for lights input
    DIGITAL_INPUTS,
    ANALOG_INPUTS,     // Command for driver data
    LIGHTS_STATUS_BASE, // Command for lights status
    HEARTBEAT,
    COMMON_BOARD_HEARTBEAT,
    MOTOR_BOARD_HEARTBEAT,
    ARRAY_BOARD_HEARTBEAT,
    LV_BOARD_HEARTBEAT,
    CHARGE_BOARD_HEARTBEAT,
    COMMON_BOARD_STATUS,
    MOTOR_BOARD_STATUS,
    ARRAY_BOARD_STATUS,
    LV_BOARD_STATUS,
    CHARGE_BOARD_STATUS,
    PACK_INFO,
    TEMPERATURE_INFO,
    CELL_VOLTAGES,

	DEAD_COMMON_HEARTBEAT,
	DEAD_MOTOR_HEARTBEAT,

	HARD_MAX_TEMP,
	SOFT_MAX_TEMP,
	HARD_MIN_TEMP,
	SOFT_MIN_TEMP

};

/* Macros ------------------------------------------------------------------*/

/* Class ------------------------------------------------------------------*/
class CANTxTask : public Task
{
public:
    static CANTxTask &Inst()
    {
        static CANTxTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void *pvParams) { CANTxTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void *pvParams);                                                // Main run code
    void HandleCommand(Command &cm);

private:
    // Private Functions
    CANTxTask();                             // Private constructor
    CANTxTask(const CANTxTask &);            // Prevent copy-construction
    CANTxTask &operator=(const CANTxTask &); // Prevent assignment
};

#endif
