#pragma once
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "Timer.hpp"

class CANTXTask : public Task
{
public:
    static CANTXTask& Inst() {
        static CANTXTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { CANTXTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); // Main run code

    void HandleCommand(Command& cm);

private:
    // Private Functions
    CANTXTask();        // Private constructor
    CANTXTask(const CANTXTask&);                        // Prevent copy-construction
    CANTXTask& operator=(const CANTXTask&);            // Prevent assignment
};