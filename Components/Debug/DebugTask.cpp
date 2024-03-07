/**
  ******************************************************************************
  * File Name          : DebugTask.cpp
  * Description        : Task for controlling debug input
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "DebugTask.hpp"
#include "Command.hpp"
#include "CubeUtils.hpp"
#include <cstring>

// External Tasks (to send debug commands to)

/* Macros --------------------------------------------------------------------*/

/* Structs -------------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/
constexpr uint8_t DEBUG_TASK_PERIOD = 100;

/* Variables -----------------------------------------------------------------*/

/* Prototypes ----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**
 * @brief Constructor, sets all member variables
 */
DebugTask::DebugTask() : Task(TASK_DEBUG_QUEUE_DEPTH_OBJS), kUart_(UART::Debug)
{
    memset(debugBuffer, 0, sizeof(debugBuffer));
    debugMsgIdx = 0;
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

        // Wait forever for a command
        qEvtQueue->ReceiveWait(cm);

        // Process the command
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
    else if (strncmp(msg, "iecho ", 6) == 0) {
        // Int echo the message (echo an int parameter)
        int32_t val = Utils::ExtractIntParameter(msg, 6);
        if (val != ERRVAL && val > 0 && val < UINT16_MAX) {
            CUBE_PRINT("\n%d", val);
        }
    }

    //-- SYSTEM / CHAR COMMANDS -- (Must be last)
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
