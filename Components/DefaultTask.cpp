/*
 * DefaultTask.cpp
 */

#include "DefaultTask.hpp"
#include "./Testing/PQueue_Test.hpp"

/**
 * @brief Initializes Default task with the RTOS scheduler
*/
void DefaultTask::InitTask()
{
    // Make sure the task is not already initialized
    CUBE_ASSERT(rtTaskHandle == nullptr, "Cannot initialize Default task twice");

    // Start the task
    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)DefaultTask::RunTask,
            (const char*)"DefaultTask",
            (uint16_t)512,
            (void*)this,
            (UBaseType_t)3,
            (TaskHandle_t*)&rtTaskHandle);

    //Ensure creation succeded
    CUBE_ASSERT(rtValue == pdPASS, "DefaultTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief Instance Run loop for the Default Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
*/
void DefaultTask::Run(void * pvParams)
{ 
    //UART Task loop
    int i = 0;
    while(1) {

        CUBE_PRINT("Hello, world [%d]\n", i);
        test_pqueue_send_receive();
        osDelay(1000);
        ++i;
    }
}



