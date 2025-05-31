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
