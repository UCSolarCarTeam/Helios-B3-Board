/**
  ******************************************************************************
  * File Name          : DebugTask.cpp
  * Description        : Task for controlling debug input
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "main_system.hpp"
#include "DebugTask.hpp"
#include "Command.hpp"
#include "CubeUtils.hpp"
#include <cstring>

#include "IOExpander.hpp"
#include "SPI/SPI_Task.hpp"

#include "CANTx/CANTxTask.hpp"
#include "CAN.h"

// External Tasks (to send debug commands to)

/* Macros --------------------------------------------------------------------*/

/* Structs -------------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/
constexpr uint8_t DEBUG_TASK_PERIOD = 100;

/* Variables -----------------------------------------------------------------*/
extern ts_contactor_state contactor_array[5];
extern ts_orion_info orion_info;
static IOExpander ioExpander(SystemHandles::I2C_Expander, IOExpander::CalculateAddress(1,0,0));


/* Prototypes ----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**
 * @brief Constructor, sets all member variables
 */
DebugTask::DebugTask() : Task(TASK_DEBUG_QUEUE_DEPTH_OBJS), kUart_(UART::Debug)
{
    memset(debugBuffer, 0, sizeof(debugBuffer));
    debugMsgIdx = 0;
    debugRxChar = 0;
    isDebugMsgReady = false;
}

/**
 * @brief Init task for RTOS
 */
void DebugTask::InitTask()
{
    // Make sure the task is not already initialized
    CUBE_ASSERT(rtTaskHandle == nullptr, "Cannot initialize Debug task twice");

    // Start the task
    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)DebugTask::RunTask,
            (const char*)"DebugTask",
            (uint16_t)TASK_DEBUG_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_DEBUG_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    // Ensure creation succeded
    CUBE_ASSERT(rtValue == pdPASS, "DebugTask::InitTask - xTaskCreate() failed");
}

// TODO: Only run thread when appropriate GPIO pin pulled HIGH (or by define)
/**
 *    @brief Runcode for the DebugTask
 */
void DebugTask::Run(void * pvParams)
{
    // Arm the interrupt
    ReceiveData();

    while (1) {
        Command cm;

        //Wait forever for a command
        qEvtQueue->ReceiveWait(cm);

        //Process the command
        if(cm.GetCommand() == DATA_COMMAND && cm.GetTaskCommand() == EVENT_DEBUG_RX_COMPLETE) {
            HandleDebugMessage((const char*)debugBuffer);
        }

        cm.Reset();
    }
}

/**
 * @brief Handles debug messages, assumes msg is null terminated
 * @param msg Message to read, must be null termianted
 */
void DebugTask::HandleDebugMessage(const char* msg)
{
    //-- PARAMETRIZED COMMANDS -- (Must be first)
    if (strncmp(msg, "echo ", 5) == 0) {
        // Echo the message (without the 'echo')
        CUBE_PRINT("\n%s", &msg[5]);
    }
    else if (strncmp(msg, "readAccel ", strlen("readAccel ")) == 0) {
        // test_SPI_Pedals.readAccelerationPedal();
    }
    else if (strncmp(msg, "readBrake ", strlen("readBrake ")) == 0) {
        // test_SPI_Pedals.readBrakingPedal();
    }
    else if (strncmp(msg, "iecho ", 6) == 0) {
        // Int echo the message (echo an int parameter)
        int32_t val = Utils::ExtractIntParameter(msg, 6);
        if (val != ERRVAL && val > 0 && val < UINT16_MAX) {
            CUBE_PRINT("\n%d", val);
        }
    }
    else if (strncmp(msg, "iox_hi ", 7) == 0) {
        // Set IO Expander pin high
        int32_t val = Utils::ExtractIntParameter(msg, 7);
        if (val != ERRVAL) {
            bool res = ioExpander.SetPinNow(static_cast<IOPin>(val), IOState::HIGH);
            if (res == false) {
                CUBE_PRINT("\nIO Expander Set Pin Failed");
            }
        }
    }
    else if (strncmp(msg, "iox_lo ", 7) == 0) {
        // Set IO Expander pin low
        int32_t val = Utils::ExtractIntParameter(msg, 7);
        if (val != ERRVAL) {
            bool res = ioExpander.SetPinNow(static_cast<IOPin>(val), IOState::LOW);
            if (res == false) {
                CUBE_PRINT("\nIO Expander Set Pin Failed");
            }
        }
    }
    else if (strncmp(msg, "iox_tog ", 8) == 0) {
        // Toggle IO Expander pin
        int32_t val = Utils::ExtractIntParameter(msg, 8);
        if (val != ERRVAL) {
            bool res = ioExpander.TogglePinNow(static_cast<IOPin>(val));
            if (res == false) {
                CUBE_PRINT("\nIO Expander Toggle Pin Failed");
            }
        }
    }
    else if (strncmp(msg, "iox_rd ", 7) == 0) {
        // Read IO Expander pin
        int32_t val = Utils::ExtractIntParameter(msg, 7);
        if (val != ERRVAL) {
            IOState state = ioExpander.GetPinState(static_cast<IOPin>(val));
            CUBE_PRINT("\nIO Expander Pin %d State: %d", val, state);
        }
    }

    //-- CAN Commands --
    else if (strncmp(msg, "can_lights_input ", strlen("can_lights_input ")) == 0) {
        Command cmd(DATA_COMMAND, LIGHTS_INPUT);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent CAN Lights INPUT Command");
        }

    }
    else if (strncmp(msg, "can_digital_input ", strlen("can_digital_input ")) == 0) {
        Command cmd(DATA_COMMAND, DIGITAL_INPUTS);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent CAN Digital INPUT Command");
        }
    }
    else if (strncmp(msg, "light_status_base ", strlen("light_status_base ")) == 0) {
        Command cmd(DATA_COMMAND, LIGHTS_STATUS_BASE);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent CAN Lights Status Command");
        }
    }

    //
    else if (strncmp(msg, "dead_common_heartbeat ", strlen("dead_common_heartbeat ")) == 0) {
        Command cmd(DATA_COMMAND, DEAD_COMMON_HEARTBEAT);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent dead common heartbeat Command");
        }
    }
    else if (strncmp(msg, "dead_motor_heartbeat ", strlen("dead_motor_heartbeat ")) == 0) {
        Command cmd(DATA_COMMAND, DEAD_MOTOR_HEARTBEAT);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent dead motor heartbeat Command");
        }
    }
    else if (strncmp(msg, "dead_array_heartbeat ", strlen("dead_array_heartbeat ")) == 0) {
        Command cmd(DATA_COMMAND, DEAD_ARRAY_HEARTBEAT);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent dead array heartbeat Command");
        }
    }
    else if (strncmp(msg, "dead_lv_heartbeat ", strlen("dead_lv_heartbeat ")) == 0) {
        Command cmd(DATA_COMMAND, DEAD_LV_HEARTBEAT);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent dead lv heartbeat Command");
        }
    }
    else if (strncmp(msg, "dead_charge_heartbeat ", strlen("dead_charge_heartbeat ")) == 0) {
        Command cmd(DATA_COMMAND, DEAD_CHARGE_HEARTBEAT);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent dead charge heartbeat Command");
        }
    }

    else if (strncmp(msg, "hard_high_cell ", strlen("hard_high_cell ")) == 0) {
        Command cmd(DATA_COMMAND, HARD_HIGH_CELL);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent hard high cell voltage Command");
        }
    }

    else if (strncmp(msg, "soft_high_cell ", strlen("soft_high_cell ")) == 0) {
        Command cmd(DATA_COMMAND, SOFT_HIGH_CELL);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent soft high cell voltage Command");
        }
    }

    else if (strncmp(msg, "hard_low_cell ", strlen("hard_low_cell ")) == 0) {
        Command cmd(DATA_COMMAND, HARD_LOW_CELL);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent hard low cell voltage Command");
        }
    }

    else if (strncmp(msg, "soft_low_cell ", strlen("soft_low_cell ")) == 0) {
            Command cmd(DATA_COMMAND, SOFT_LOW_CELL);
            Queue* evtQ = CANTxTask::Inst().GetEventQueue();
            bool res = evtQ->Send(cmd);
            if(res){
               CUBE_PRINT("Sent soft low cell voltage Command");
            }
        }
    /* bad currents */

    else if (strncmp(msg, "hard_high_common ", strlen("hard_high_common ")) == 0) {
        Command cmd(DATA_COMMAND, HARD_HIGH_COMMON);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent hard high common Command");
        }
    }

    else if (strncmp(msg, "soft_high_common ", strlen("soft_high_common ")) == 0) {
        Command cmd(DATA_COMMAND, SOFT_HIGH_COMMON);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent soft high common Command");
        }
    }

    else if (strncmp(msg, "hard_high_motor ", strlen("hard_high_motor ")) == 0) {
        Command cmd(DATA_COMMAND, HARD_HIGH_MOTOR);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent hard high motor Command");
        }
    }

    else if (strncmp(msg, "soft_high_motor ", strlen("soft_high_motor ")) == 0) {
        Command cmd(DATA_COMMAND, SOFT_HIGH_MOTOR);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent soft high motor Command");
        }
    }

    else if (strncmp(msg, "hard_high_array ", strlen("hard_high_array ")) == 0) {
        Command cmd(DATA_COMMAND, HARD_HIGH_ARRAY);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent hard high array Command");
        }
    }

    else if (strncmp(msg, "soft_high_array ", strlen("soft_high_array ")) == 0) {
        Command cmd(DATA_COMMAND, SOFT_HIGH_ARRAY);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent soft high array Command");
        }
    }

    else if (strncmp(msg, "hard_high_lv ", strlen("hard_high_lv ")) == 0) {
        Command cmd(DATA_COMMAND, HARD_HIGH_LV);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent hard high lv Command");
        }
    }


    else if (strncmp(msg, "soft_high_lv ", strlen("soft_high_lv ")) == 0) {
        Command cmd(DATA_COMMAND, SOFT_HIGH_LV);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent soft high lv Command");
        }
    }


    else if (strncmp(msg, "hard_high_charge ", strlen("hard_high_charge ")) == 0) {
        Command cmd(DATA_COMMAND, HARD_HIGH_CHARGE);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent hard high charge Command");
        }
    }

    else if (strncmp(msg, "soft_high_charge ", strlen("soft_high_charge ")) == 0) {
        Command cmd(DATA_COMMAND, SOFT_HIGH_CHARGE);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent soft high charge Command");
        }
    }

    /* bad temperatures */

    else if (strncmp(msg, "hard_max_temp ", strlen("hard_max_temp ")) == 0) {
        Command cmd(DATA_COMMAND, HARD_MAX_TEMP);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent hard max temp Command");
        }
    }
    else if (strncmp(msg, "soft_max_temp ", strlen("soft_max_temp ")) == 0) {
        Command cmd(DATA_COMMAND, SOFT_MAX_TEMP);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent soft max temp Command");
        }
    }
    else if (strncmp(msg, "hard_min_temp ", strlen("hard_min_temp ")) == 0) {
        Command cmd(DATA_COMMAND, HARD_MIN_TEMP);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent hard min temp Command");
        }
    }
    else if (strncmp(msg, "soft_min_temp ", strlen("soft_min_temp ")) == 0) {
        Command cmd(DATA_COMMAND, SOFT_MIN_TEMP);
        Queue* evtQ = CANTxTask::Inst().GetEventQueue();
        bool res = evtQ->Send(cmd);
        if(res){
           CUBE_PRINT("Sent soft min temp Command");
        }
    }
    //-- SYSTEM / CHAR COMMANDS -- (Must be last)
    else if (strncmp(msg, "iox_upd", 7) == 0) {
        // Update IO Expander
        ioExpander.Update();
    }
    else if (strncmp(msg, "iox_com", 7) == 0) {
        // Commit IO Expander
        ioExpander.Commit();
    }
    else if (strcmp(msg, "sysreset") == 0) {
        // Reset the system
        CUBE_ASSERT(false, "System reset requested");
    }
    else if (strcmp(msg, "sysinfo") == 0) {
        // Print message
        CUBE_PRINT("\n\n-- CUBE SYSTEM --\n");
        CUBE_PRINT("Current System Free Heap: %d Bytes\n", xPortGetFreeHeapSize());
        CUBE_PRINT("Lowest Ever Free Heap: %d Bytes\n", xPortGetMinimumEverFreeHeapSize());
        CUBE_PRINT("Debug Task Runtime  \t: %d ms\n\n", TICKS_TO_MS(xTaskGetTickCount()));
    }
    else {
        // Single character command, or unknown command
        switch (msg[0]) {
        default:
            CUBE_PRINT("Debug, unknown command: %s\n", msg);
            break;
        }
    }

    //We've read the data, clear the buffer
    debugMsgIdx = 0;
    isDebugMsgReady = false;
}

/**
 * @brief Receive data, currently receives by arming interrupt
 */
bool DebugTask::ReceiveData()
{
    return kUart_->ReceiveIT(&debugRxChar, this);
}

/**
 * @brief Receive data to the buffer
 * @return Whether the debugBuffer is ready or not
 */
void DebugTask::InterruptRxData(uint8_t errors)
{
    // If we already have an unprocessed debug message, ignore this byte
    if (!isDebugMsgReady) {
        // Check byte for end of message - note if using termite you must turn on append CR
        if (debugRxChar == '\r' || debugMsgIdx == DEBUG_RX_BUFFER_SZ_BYTES) {
            // Null terminate and process
            debugBuffer[debugMsgIdx++] = '\0';
            isDebugMsgReady = true;

            // Notify the debug task
            Command cm(DATA_COMMAND, EVENT_DEBUG_RX_COMPLETE);
            bool res = qEvtQueue->SendFromISR(cm);

            // If we failed to send the event, we should reset the buffer, that way DebugTask doesn't stall
            if (res == false) {
                debugMsgIdx = 0;
                isDebugMsgReady = false;
            }
        }
        else {
            debugBuffer[debugMsgIdx++] = debugRxChar;
        }
    }

    //Re-arm the interrupt
    ReceiveData();
}
