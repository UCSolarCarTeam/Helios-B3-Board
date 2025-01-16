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

/* Enums ------------------------------------------------------------------*/
enum CAN_TX_COMMANDS
{
    LIGHTS_INPUT, // Command for lights input
    DIGITAL_INPUTS,
    ANALOG_INPUTS,     // Command for driver data
    LIGHTS_STATUS_BASE // Command for lights status
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
    int CAN_TX_FREQ = 10;
};

#endif
