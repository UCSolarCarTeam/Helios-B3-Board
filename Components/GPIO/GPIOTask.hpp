/**
 ******************************************************************************
 * File Name          : GPIOTask.hpp
 * Description        : Polls and updates I2C GPIO expander in B3.
 ******************************************************************************
*/
#ifndef HELIOS_GPIOTASK_HPP_
#define HELIOS_GPIOTASK_HPP_

#include "Task.hpp"
#include "SystemDefines.hpp"
#include "Timer.hpp"

/*---------------------------------- Macros/Enums ----------------------------------*/
enum GPIO_COMMANDS {
    GPIO_STATE = 0,
    GPIO_RESET
};

/*---------------------------------- Task Implementation ----------------------------------*/
class GPIOTask : public Task 
{
public:
    static GPIOTask& Inst() {
        static GPIOTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { GPIOTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams);                                              // Main run code

private:
    // Private Functions
    GPIOTask();                                 // Private constructor
    GPIOTask(const GPIOTask&);                  // Prevent copy-construction
    GPIOTask& operator=(const GPIOTask&);       // Prevent assignment
};

#endif
