/**
 ******************************************************************************
 * File Name          : WatchdogTask.hpp
 * Description        : Primary Watchdog task, default task for the system.
 ******************************************************************************
*/
#ifndef HELIOS_WATCHDOGTASK_HPP_
#define HELIOS_WATCHDOGTASK_HPP_
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "Timer.hpp"

/* Macros/Enums ------------------------------------------------------------*/
enum HEARTBEAT_COMMANDS  {
    RADIOHB_NONE = 0,
    RADIOHB_REQUEST,        // Heartbeat countdown timer is reset when HEARTBEAT_COMMAND is sent
    HB_STATUS_SEND,         // Sends the status of the heartbeat
    RADIOHB_DISABLED
};

class WatchdogTask : public Task
{
public:
    static WatchdogTask& Inst() {
        static WatchdogTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { WatchdogTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); // Main run code

    void HandleCommand(Command& cm);

private:
    // Private Functions
    WatchdogTask();        // Private constructor
    WatchdogTask(const WatchdogTask&);                        // Prevent copy-construction
    WatchdogTask& operator=(const WatchdogTask&);            // Prevent assignment
};

#endif    // HELIOS_WATCHDOGTASK_HPP_
