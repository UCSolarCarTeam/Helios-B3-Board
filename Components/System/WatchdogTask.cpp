/**
 ******************************************************************************
 * File Name          : WatchdogTask.cpp
 * Description        : Primary Watchdog task, default task for the system.
 ******************************************************************************
*/
#include "GPIO.hpp"
#include "SystemDefines.hpp"
#include "WatchdogTask.hpp"

/* Macros/Enums ------------------------------------------------------------*/

/**
 * @brief Constructor for WatchdogTask
 */
WatchdogTask::WatchdogTask() : Task(WATCHDOG_TASK_QUEUE_DEPTH_OBJS)
{
}

/**
 * @brief Initialize the WatchdogTask
 */
void WatchdogTask::InitTask()
{
    // Make sure the task is not already initialized
    CUBE_ASSERT(rtTaskHandle == nullptr, "Cannot initialize watchdog task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)WatchdogTask::RunTask,
            (const char*)"WatchdogTask",
            (uint16_t)WATCHDOG_TASK_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)WATCHDOG_TASK_RTOS_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    CUBE_ASSERT(rtValue == pdPASS, "WatchdogTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief Handles a command
 * @param cm Command reference to handle
 */
void WatchdogTask::HandleCommand(Command& cm)
{
    switch (cm.GetCommand()) {
    case TASK_SPECIFIC_COMMAND: {
        break;
    }
    default:
        CUBE_PRINT("WatchdogTask - Received Unsupported Command {%d}\n", cm.GetCommand());
        break;
    }

    //No matter what we happens, we must reset allocated data
    cm.Reset();
}

/**
 * @brief Instance Run loop for the Watchdog Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void WatchdogTask::Run(void * pvParams)
{
    uint32_t tempSecondCounter = 0;

    while (1) {
        GPIO::LED_BLUE::On();
        osDelay(500);
        GPIO::LED_BLUE::Off();
        osDelay(500);
        //GPIO::LED_GREEN::Toggle(); // This does not work right now (perhaps wrong pin)
        GPIO::LED_RED::Toggle();

        Command cm;

        CUBE_PRINT(">> Run [%d] s", tempSecondCounter++);

        // Ingest the command queue, up to 5 commands
        uint8_t proced = 0;
        while (qEvtQueue->Receive(cm) && (proced < 5)) {
            HandleCommand(cm);
            ++proced;
        }

        cm.Reset();

    }
}
