#pragma once

#include "main_system.hpp"
#include "Task.hpp"
#include "Timer.hpp"
#include "Command.hpp"
#include "CubeUtils.hpp"
#include <cstring>

#include "SystemDefines.hpp"
#include "CubeDefines.hpp"

//enum CANRX_COMMANDS {
//    CANRX_NONE = 0,
 //   CANRX_COMMAND_1,
 //   CANRX_COMMAND_2,
    // Add other relevant commands here
//};
// Enums for CAN message commands related to lights and signals
enum CANRX_COMMANDS {
    LIGHTS_INPUT_BASE,    // Command for lights input
    DRIVER_BASE,          // Command for driver data
    LIGHTS_STATUS_BASE    // Command for lights status
};



class CANRXTask : public Task
{
public:
    static CANRXTask& Inst() {
        static CANRXTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { CANRXTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); // Main run code

    void HandleCommand(Command& cm);

private:
    // Private Functions
    CANRXTask();        // Private constructor
    CANRXTask(const CANRXTask&);                        // Prevent copy-construction
    CANRXTask& operator=(const CANRXTask&);            // Prevent assignment
};
