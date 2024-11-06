#include "CANRXTask.hpp"
#include "CAN/CAN.h"
#include "CAN/CANRegisters.h"
#include "main_system.hpp"
#include <stdint.h>

/**
 * @brief Constructor for CANRXTask
 */
CANRXTask::CANRXTask() : Task(CAN_RX_TASK_QUEUE_DEPTH_OBJS) {}

/**
 * @brief Init task for RTOS
 */
void CANRXTask::InitTask()
{
    // Make sure the task is not already initialized
    CUBE_ASSERT(rtTaskHandle == nullptr, "Cannot initialize CAN RX task twice");

    // Start the task
    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)CANRXTask::RunTask,
            (const char*)"CANRXTask",
            (uint16_t)CAN_RX_TASK_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)CAN_RX_TASK_RTOS_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    // Ensure creation succeded
    CUBE_ASSERT(rtValue == pdPASS, "CANRXTask::InitTask - xTaskCreate() failed");
}

/**
 * @brief Main run code for CANRXTask
 * @param pvParams Parameters passed to the task
 */
void CANRXTask::Run(void * pvParams)
{
    uint8_t CANRxFlag_0 = 0;
    uint8_t CANRxFlag_1 = 0;

    uint8_t CANINTF_STATUS = 0;
    uint8_t EFLG_STATUS = 0;
    uint8_t CAN_STATUS = 0;

    uint8_t TXB0CTRL_STATUS = 0;
    uint8_t TXB1CTRL_STATUS = 0;
    uint8_t TXB2CTRL_STATUS = 0;

    CANPeripheral peripheral1 = {
    	.CS_PORT = CS_CAN_N_GPIO_Port,
		.CS_PIN = CS_CAN_N_Pin,
		.hspi = SystemHandles::CAN_SPI_Handler
    };

	uint32_t ID = 0;
	uint8_t DLC = 0;
	uint8_t data[8] = {0};

	CANMsg msg1 = {
		.ID = 0,
		.extendedID = 0xCFCFCFC,
		.DLC = 1,
		.data = {0xCC}
	};

//	CANMsg msg2 = {
//		.ID = 0,
//		.extendedID = 0xAFAFAFA,
//		.DLC = 1,
//		.data = {0xAA}
//	};

	ConfigureCANSPI(&peripheral1);

    while (1)
    {
        CANINTF_STATUS = 0;
        EFLG_STATUS = 0;
        CAN_STATUS = 0;

        // Read CANINTF register
        CAN_IC_READ_REGISTER(CANINTF, &CANINTF_STATUS, &peripheral1);
        CAN_IC_READ_REGISTER(EFLG, &EFLG_STATUS, &peripheral1);

        // clear interrupts and error flags in case
        CAN_IC_WRITE_REGISTER(CANINTF, 0x00, &peripheral1);
        CAN_IC_WRITE_REGISTER(EFLG, 0x00, &peripheral1);

        CUBE_PRINT("CANINTF 1: %u\n", CANINTF_STATUS);
        CUBE_PRINT("EFLG 1: %u\n", EFLG_STATUS);

        CAN_IC_READ_STATUS(&CAN_STATUS, &peripheral1);

        // Send Message Here
        // sendCANMessage(&msg1, &peripheral1);
        sendExtendedCANMessage(&msg1, &peripheral1);
        // sendCANMessage(&msg2, &peripheral1);
        // sendExtendedCANMessage(&msg2, &peripheral1);

        CAN_IC_READ_REGISTER(TXB0CTRL, &TXB0CTRL_STATUS, &peripheral1);
        CAN_IC_READ_REGISTER(TXB1CTRL, &TXB1CTRL_STATUS, &peripheral1);
        CAN_IC_READ_REGISTER(TXB2CTRL, &TXB2CTRL_STATUS, &peripheral1);

        // Read interrupt status again
        CANINTF_STATUS = 0;
        EFLG_STATUS = 0;
        CAN_STATUS = 0;
        CAN_IC_READ_REGISTER(CANINTF, &CANINTF_STATUS, &peripheral1);
        CAN_IC_READ_REGISTER(EFLG, &EFLG_STATUS, &peripheral1);
        CAN_IC_READ_STATUS(&CAN_STATUS, &peripheral1);

        CUBE_PRINT("CANINTF 2: %u\n", CANINTF_STATUS);
        CUBE_PRINT("EFLG 2: %u\n", EFLG_STATUS);

        if (CANINTF_STATUS & 0x01) {
        	CANRxFlag_0 = 1;
        }
        if (CANINTF_STATUS & 0x02) {
        	CANRxFlag_1 = 1;
        }

        // Receive CAN_Messages
        if (CANRxFlag_0 == 1) {
        	CUBE_PRINT("RX_BUFFER RECV 0\n");
            receiveCANMessage(0, &ID, &DLC, data, &peripheral1);
        }
        if (CANRxFlag_1 == 1) {
        	CUBE_PRINT("RX_BUFFER RECV 1\n");
        	receiveCANMessage(1, &ID, &DLC, data, &peripheral1);
        }

        CAN_IC_READ_STATUS(&CAN_STATUS, &peripheral1);
        CANRxFlag_0 = 0;
        CANRxFlag_1 = 0;

        osDelay(500);
    }
}


/**
 * @brief Handle a command
 * @param cm Command to handle
 */
void CANRXTask::HandleCommand(Command& cm){
//    // Handle the command
//    switch (cm.GetTaskCommand()) {
//    case EVENT_DEBUG_RX_COMPLETE:
//        // Handle the debug message
//        CUBE_PRINT("CAN RX Task received debug message: %s\n", cm.GetDataPointer());
//        break;
//    default:
//        CUBE_PRINT("CAN RX Task received unknown command: %d\n", cm.GetTaskCommand());
//        break;
//    }
}
