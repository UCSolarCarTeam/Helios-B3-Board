/**
 ******************************************************************************
 * File Name          : CANTxTask.hpp
 * Description        :
 ******************************************************************************
*/
#include "Task.hpp"
#include "main.h"

class CANRxTask : public Task
{
public:
    static CANRxTask& Inst() {
        static CANRxTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { CANRxTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); // Main run code

private:
    CANRxTask();        // Private constructor
    CANRxTask(const CANRxTask&);                        // Prevent copy-construction
    CANRxTask& operator=(const CANRxTask&);            // Prevent assignment
};