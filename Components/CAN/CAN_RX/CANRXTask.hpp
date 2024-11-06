#pragma once

#include "main_system.hpp"
#include "Task.hpp"
#include "Timer.hpp"
#include "Command.hpp"
#include "CubeUtils.hpp"
#include <cstring>

#include "SystemDefines.hpp"
#include "CubeDefines.hpp"


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

