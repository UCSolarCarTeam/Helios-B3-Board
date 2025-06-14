/**
 ******************************************************************************
 * File Name          : CANRxTask.cpp
 * Description        : Task for receiving CAN messages
 ******************************************************************************
 */

#include "CanRxTask.hpp"

// Maybe add a header file
CANPeripheral peripheral2 = {
    .CS_PORT = CS_CAN_N_GPIO_Port,
    .CS_PIN = CS_CAN_N_Pin,
    .hspi = SystemHandles::CAN_SPI
};

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


uint32_t vehicleVelocity = 0;
uint32_t motorVelocity = 0;
uint8_t allowCharge = 0;
uint8_t allowDischarge = 0;

/**
 * @brief Constructor for CANTxTask
 */
CANRxTask::CANRxTask() : Task(CAN_TX_TASK_QUEUE_DEPTH_OBJS)
{
}

/**
 * @brief Initialize the CANTxTask
 */
void CANRxTask::InitTask()
{
    // Make sure the task is not already initialized
    CUBE_ASSERT(rtTaskHandle == nullptr, "Cannot initialize CAN RX task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)CANRxTask::RunTask,
                    (const char *)"CANRxTask",
                    (uint16_t)CAN_RX_TASK_STACK_DEPTH_WORDS,
                    (void *)this,
                    (UBaseType_t)CAN_RX_TASK_PRIORITY,
                    (TaskHandle_t *)&rtTaskHandle);

    CUBE_ASSERT(rtValue == pdPASS, "CANRxTask::InitTask() - xTaskCreate() failed");
}

void CANRxTask::Run(void *pvParams)
{
    ConfigureCANSPI(&peripheral2);
    
    while (1)
    {
        /***
         * MCP2510: https://usw.365.altium.com/librarycomponentsapi/api/v1/References/79FC5B82-F317-4482-BD19-9D6F8666BE78
         * Plan:
         * On buffer interrupt add to queue
         * While(1) Task processes Queue
         * Later Optimization: Add Mutex to
         *
         * 1. Detect RX Interrupt RXBUFF
         *      - Set interrupts RXBuff pg 21
         *      - RXBUFF0 and RXBUFF1 .
         * 2. Queue buffer contents
         *      - CAN_RX_QUEUE.SendFromISR()
         *      - Parse data into command
         *            receiveCANMessage()
         * 3. Mutex the SPI Peripheral
         *      Mutex
         * 4. Handle buffer
         */

        //  Wait forever for a command on interrupt
        Command cm;
        qEvtQueue->ReceiveWait(cm);

        // Process the command
        HandleCommand(cm);


        cm.Reset();
    }
}

void CANRxTask::HandleCommand(Command &cm)
{
    uint32_t id = 0;
    uint8_t dlc = 0;
    uint8_t data[8] = {0};

    switch (static_cast<CAN_RX_COMMANDS>(cm.GetTaskCommand()))
    {
    case CAN_INTERRUPT_HAPPENED:
        CUBE_PRINT("Received a CAN RX Message by interrupt\n");
        break;
    case CAN_RX0_INTERRUPT_HAPPENED:
        receiveCANMessage(0, &id, &dlc, data, &peripheral2);
        CUBE_PRINT("CAN RX0 Message:\n");
        CUBE_PRINT_CAN_MESSAGE(id, dlc, data);
        break;
    case CAN_RX1_INTERRUPT_HAPPENED:
        receiveCANMessage(1, &id, &dlc, data, &peripheral2);
        CUBE_PRINT("CAN RX1 Message:\n");
        CUBE_PRINT_CAN_MESSAGE(id, dlc, data);
        break;
    default:
        CUBE_PRINT("CANRXTask - Received unsupported command: %d\n", cm.GetCommand());
        break;
    }
}

//// Handle CAN_INT Callback here ?
#if 0
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
   // NOTE: Can Implement Call back on RXBUF0 and RXBUF1 and decode the message accordingly.
   if (GPIO_Pin == CAN_INT_Pin)
   {
       // Handle or Event Flag into CPP Task
       Command canInterruptHappenedCommandFlag = Command(TASK_SPECIFIC_COMMAND, CAN_INTERRUPT_HAPPENED);
       CANRxTask::Inst()
           .GetCAN_RX_QUEUE()
           ->SendFromISR(canInterruptHappenedCommandFlag);
   }
   else if (GPIO_Pin == CAN_RX0BF_Pin)
   {
       Command canInterruptHappenedCommandFlag = Command(TASK_SPECIFIC_COMMAND, CAN_RX0_INTERRUPT_HAPPENED);
       CANRxTask::Inst()
           .GetCAN_RX_QUEUE()
           ->SendFromISR(canInterruptHappenedCommandFlag);
   }
   else if (GPIO_Pin == CAN_RX1BF_Pin)
   {
       Command canInterruptHappenedCommandFlag = Command(TASK_SPECIFIC_COMMAND, CAN_RX1_INTERRUPT_HAPPENED);
       CANRxTask::Inst()
           .GetCAN_RX_QUEUE()
           ->SendFromISR(canInterruptHappenedCommandFlag);
   }
}
#endif

void CANRxTask::HandleCANMessage(uint32_t id, uint8_t dlc, uint8_t *data){
    switch(id){
        case MBMS_MESSAGE: // MBMS Status ID: CHANGE THIS TO THE ACTUAL DEFINE LATER
            // Bit 6 = nChargeEnable
            // Bit 8 = nDischargeEnable
            allowDischarge = (data[0] & 0x40) ? 0 : 1;  // Byte 0, Bit 6
            allowCharge = (data[1] & 0x1) ? 0 : 1;      // Byte 1, Bit 0
            CUBE_PRINT("MBMS Status: allowCharge = %d, allowDischarge = %d\n", allowCharge, allowDischarge);
            break;

        // TODO: add case motor feedback ID 0x402
        // extract the vehicle velocity input from the data
//        case MOTOR_CONTROLLER_BASE:
//            // IDinfo ID: 0x420
//            // Name         | Bytes |   Bits
//            // ProhelionID	    4	    31-0
//            // SerialNumber	4	    63-32
//
//            // extract the Tritium ID
//            uint32_t prohelionID = 0;
//            for (uint8_t i = 0; i < 4; ++i) {
//                prohelionID |= (data[i] << (i * 8));
//            }
//
//            uint32_t serialNumber = 0;
//            for (uint8_t i = 4; i < 8; ++i) {
//                serialNumber |= (data[i] << ((i - 4) * 8));
//            }
//
//            break;
//
//        case MOTOR_STATUS:
//            // Status ID: 0x421
//            // Name         | Bytes |   Bits
//            // LimitFlags	    2	    15-0
//            // ErrorFlags	    2	    31-16
//            // ActiveMotor	    2	    47-32
//            // TxErrorCount	    1	    55-48
//            // RxErrorCount	    1       63-56
//
//            uint16_t limitFlags = 0;
//            for (uint8_t i = 0; i < 4; ++i) {
//                limitFlags |= (data[i] << (i * 8));
//            }
//
//            uint16_t errorFlags = 0;
//            for (uint8_t i = 2; i < 4; ++i) {
//                errorFlags |= (data[i] << ((i - 2) * 8));
//            }
//
//            uint16_t activeMotor = 0;
//            for (uint8_t i = 4; i < 6; ++i) {
//                activeMotor |= (data[i] << ((i - 4) * 8));
//            }
//            uint8_t txErrorCount = data[6];
//
//            uint8_t rxErrorCount = data[7];
//
//            break;
//
//        case MOTOR_BUS_MEASUREMENT:
//            // Name         | Bytes |   Bits
//            //BusVoltage	    4	    31-0
//            //BusCurrent	    4	    63-32
//
//            uint32_t busVoltage = 0.0f;
//            for(uint8_t i = 0; i < 4; ++i) {
//                busVoltage |= (data[i] << (i * 8));
//            }
//            uint32_t busCurrent = 0.0f;
//            for(uint8_t i = 4; i < 8; ++i) {
//                busCurrent |= (data[i] << ((i - 4) * 8));
//            }
//
//            break;

        case MOTOR_VELOCITY:
            // Name         | Bytes |   Bits
            // MotorVelocity	4	    31-0
            // VehicleVelocity	4	    63-32

            motorVelocity = 0.0f;
            for(uint8_t i = 0; i < 4; ++i) {
                motorVelocity |= (data[i] << (i * 8));
            }

            vehicleVelocity = 0.0f;
            for(uint8_t i = 4; i < 8; ++i) {
                vehicleVelocity |= (data[i] << ((i - 4) * 8));
            }

            break;

        default:
            CUBE_PRINT("CANRxTask - Received unsupported CAN message with ID: 0x%08X\n", id);
            break;        
    }
}

uint32_t CANRxTask::getMotorVehicleVelocityInput()
{
    // This function should return the vehicle velocity input from the motor controller
    return vehicleVelocity; // Placeholder for actual vehicle velocity input
}

uint32_t CANRxTask::getMotorVelocityInput()
{
    // This function should return the motor velocity from the motor controller
    return motorVelocity; // Placeholder for actual motor velocity
}

uint8_t CANRxTask::getAllowCharge()
{
    // This function should return the allow charge status
    return allowCharge; // Placeholder for actual allow charge status
}

uint8_t CANRxTask::getAllowDischarge()
{
    // This function should return the allow discharge status
    return allowDischarge; // Placeholder for actual allow discharge status
}

//Helper function to print CAN message
void CUBE_PRINT_CAN_MESSAGE(uint32_t id, uint8_t dlc, uint8_t *data)
{
    CUBE_PRINT("  ID: 0x%08X\n", id);
    CUBE_PRINT("  DLC: %u\n", dlc);
    CUBE_PRINT("  Data: ");
    for (uint8_t i = 0; i < dlc; ++i)
    {
        CUBE_PRINT("%02X ", data[i]);
    }
}
