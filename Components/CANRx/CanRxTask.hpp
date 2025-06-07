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
    CAN_RX0_INTERRUPT_HAPPENED, //Task specific command queued on CAN_RX0BF ISR
    CAN_RX1_INTERRUPT_HAPPENED, //Task specific command queued on CAN_RX1BF ISR
};

enum CAN_RX_ADDRESSES{
    // BMS
    MBMS_MESSAGE = 0x102,

    // Motor Controller Addresses
    MOTOR_CONTROLLER_BASE = 0x420,
    MOTOR_STATUS = 0x421,
    MOTOR_BUS_MEASUREMENT= 0x422,
    MOTOR_VELOCITY = 0x423,
    MOTOR_PHASE_CURRENT = 0x424,
    MOTOR_VOLTAGE_VECTOR = 0x425,
    MOTOR_CURRENT_VECTOR = 0x426,
    MOTOR_BACK_EMF_PRED = 0x427,
    MOTOR_RAIL_15V = 0x428,
    MOTOR_RAIL_3V3_V9 = 0x429,
    MOTOR_HEATSINK_TEMP = 0x42B,
    MOTOR_BOARD_DSP_TEMP = 0x42C,
    MOTOR_ODOMETER_BUS = 0x42E,
    MOTOR_SLIP_SPEED = 0x437,
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
    void handleCANMessage(uint32_t id, uint8_t dlc, uint8_t *data);

private:
    CANRxTask();        // Private constructor
    CANRxTask(const CANRxTask&);                        // Prevent copy-construction
    CANRxTask& operator=(const CANRxTask&);            // Prevent assignment
};

//Helper function to print CAN message
void CUBE_PRINT_CAN_MESSAGE(uint32_t id, uint8_t dlc, uint8_t *data);