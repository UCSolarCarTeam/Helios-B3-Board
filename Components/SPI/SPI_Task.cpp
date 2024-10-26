/**
 ******************************************************************************
 * File Name          : SPI_Task.cpp
 * Description        : Primary SPI task, reads data from pedals.
 ******************************************************************************
*/
#include "SystemDefines.hpp"
#include "SPI_Task.hpp"


/**
 * @brief Constructor for SPI Task
 */
SPI_Task::SPI_Task() : Task(SPI_TAK_QUEUE_DEPTH_OBJS)
{
}

/**
 * @brief Initialize the SPI Task
 */
void SPI_Task::InitTask()
{
    CUBE_ASSERT(rtTaskHandle == nullptr, "Cannot initialize SPI task twice");

    BaseType_t rtValue = 
        xTaskCreate((TaskFunction_t)SPI_Task::RunTask,
                    (const char*)"SPI Task",
                    (uint16_t)SPI_TASK_STACK_DEPTH_WORDS,
                    (void*)this,
                    (UBaseType_t)SPI_TASK_PRIORITY,
                    (TaskHandle_t)&rtTaskHandle);

    CUBE_ASSERT(rtValue == pdPASS, "SPI_Task::InitTask() - xTaskCreate() failed");
}

/**
 * @brief Handles a command. 
 * @param cm Command reference to handle
 */
void SPI_Task::HandleCommand(Command& cm){

    switch (cm.GetCommand()) {
    case TASK_SPECIFIC_COMMAND: {
        break;
    }
    default:
        CUBE_PRINT("SPI Task - Received Unsupported Command {%d}\n", cm.GetCommand());
        break;
    }

    //No matter what we happens, we must reset allocated data
    cm.Reset();
}

/**
 * @brief Instance Run loop for the SPI Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance (from InitTask), should not be used
 */
void SPI_Task::Run(void * pvParams){


    /** 
     * TODO: Logic
     * 1. First decoder selects Y0 as low to enable SPI Select on 2nd decoder
     *  -> Board Select 0 and 1 = LOW
     * 
     * 2. SPI Chip Select to select which analog to receive
     * For Acceleration -> CSb00 -> SPI CS0 = 0 && SPI CS1 = 0
     * For Braking -> CSb01 -> SPI SPI CS0 = 0 && SPI CS1 = 1
     * 
     * 3. Read the ADC output on SPI MISO
    */

}
