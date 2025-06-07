/**
 ******************************************************************************
 * File Name          : CANTxTask.hpp
 * Description        :
 ******************************************************************************
*/
#ifndef CANRXTASK_HPP_
#define CANRXTASK_HPP_

#include "Task.hpp"
#include "main.h"
#include "Queue.hpp"
#include "Mutex.hpp"
#include "CAN.h"

class CANRxTask : public Task
{
public:
    static CANRxTask& Inst() {
        static CANRxTask inst;
        return inst;
    }

    void InitTask();
    Queue* GetCAN_RX_QUEUE() const { return qEvtQueue; }
    
    uint32_t getMotorVehicleVelocityInput();
    uint32_t getMotorVelocityInput();
    uint8_t getAllowCharge();
    uint8_t getAllowDischarge();

protected:
    static void RunTask(void* pvParams) { CANRxTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); // Main run code
    void HandleCommand(Command& cm);
    void HandleCANMessage(uint32_t id, uint8_t dlc, uint8_t *data);

private:
    CANRxTask();        // Private constructor
    CANRxTask(const CANRxTask&);                        // Prevent copy-construction
    CANRxTask& operator=(const CANRxTask&);            // Prevent assignment
};

//Helper function to print CAN message
void CUBE_PRINT_CAN_MESSAGE(uint32_t id, uint8_t dlc, uint8_t *data);

#endif
