///*
// * CANTransmit.cpp
// *
// *  Created on: Nov 1, 2024
// *      Author: Omar Hassan
// */
//
//
//#include "cantransmit.hpp"
//#include "can.h"           // For low-level CAN transmission functions
//#include "FreeRTOS.h"       // FreeRTOS headers for queue management
//
//// Constructor - Initializes the queue for CAN transmission messages
//CanTransmitTask::CanTransmitTask() : Task(CAN_TX_TASK_QUEUE_DEPTH)
//{
//    txQueue = xQueueCreate(CAN_TX_TASK_QUEUE_DEPTH, sizeof(CANMsg));
//    CUBE_ASSERT(txQueue != nullptr, "Failed to create CAN transmit queue");
//}
//
//// Initialize the CAN Transmit Task
//void CanTransmitTask::InitTask()
//{
//    // Ensure the task is not already initialized
//    CUBE_ASSERT(rtTaskHandle == nullptr, "Cannot initialize CAN transmit task twice");
//
//    // Create the FreeRTOS task
//    BaseType_t rtValue = xTaskCreate(
//        (TaskFunction_t)CanTransmitTask::RunTask,
//        (const char*)"CANTransmitTask",
//        (uint16_t)CAN_TX_TASK_STACK_DEPTH,
//        (void*)this,
//        (UBaseType_t)CAN_TX_TASK_PRIORITY,
//        (TaskHandle_t*)&rtTaskHandle
//    );
//
//    // Confirm the task creation succeeded
//    CUBE_ASSERT(rtValue == pdPASS, "CANTransmitTask::InitTask() - xTaskCreate() failed");
//}
//
//// Queue a message for transmission
//bool CanTransmitTask::SendMessage(const CANMsg& msg)
//{
//    if (xQueueSend(txQueue, &msg, portMAX_DELAY) == pdPASS) {
//        return true;
//    } else {
//        CUBE_PRINT("CANTransmitTask - Failed to queue CAN message\n");
//        return false;
//    }
//}
//
//// The main task loop, which runs in the RTOS environment
//void CanTransmitTask::Run(void* pvParams)
//{
//    CANMsg msg;
//    while (1) {
//        // Wait for a CAN message in the transmit queue
//        if (xQueueReceive(txQueue, &msg, portMAX_DELAY) == pdPASS) {
//            // Call the low-level function from can.c to send the message
//            sendExtendedCANMessage(&msg, &can1);
//        }
//    }
//}
//
//
