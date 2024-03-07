/**
 ******************************************************************************
 * File Name          : DefaultTask.hpp
 * Description        :
 ******************************************************************************
*/
#ifndef B3_DEFAULTTASK_HPP_
#define B3_DEFAULTTASK_HPP_

/* Includes ------------------------------------------------------------------*/
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "CubeDefines.hpp"
#include "UARTDriver.hpp"

/* Macros ------------------------------------------------------------------*/

/* Class ------------------------------------------------------------------*/
class DefaultTask : public Task
{
public:
    static DefaultTask& Inst() {
        static DefaultTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { DefaultTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();

    void Run(void* pvParams);    // Main run code

private:
    DefaultTask() : Task(UART_TASK_QUEUE_DEPTH_OBJS) {}    // Private constructor
    DefaultTask(const DefaultTask&);                        // Prevent copy-construction
    DefaultTask& operator=(const DefaultTask&);            // Prevent assignment
};


#endif    // B3_DEFAULTTASK_HPP_
