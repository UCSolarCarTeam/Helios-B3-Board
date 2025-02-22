/**
 ******************************************************************************
 * File Name          : CANTxTask.hpp
 * Description        :
 ******************************************************************************
*/
#include "Task.hpp"
#include "main.h"
#include "Queue.hpp"
#include "Mutex.hpp"
#include "CAN.h"

enum CAN_RX_COMMANDS {
    CAN_INTERRUPT_HAPPENED, //Task specific command queued on CAN_INT ISR
};
class CANRxTask : public Task
{
public:
    static CANRxTask& Inst() {
        static CANRxTask inst;
        return inst;
    }

    void InitTask();
    Queue* GetCAN_RX_QUEUE() const { return qEvtQueue; }

protected:
    static void RunTask(void* pvParams) { CANRxTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); // Main run code
    void HandleCommand(Command& cm);

private:
    CANRxTask();        // Private constructor
    CANRxTask(const CANRxTask&);                        // Prevent copy-construction
    CANRxTask& operator=(const CANRxTask&);            // Prevent assignment
};