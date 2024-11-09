/*
 * CANTask.h
 *
 *  Created on: Oct 26, 2024
 *      Author: Omar Hassan
 */

#ifndef CAN_CANTRANSMIT_HPP_
#define CAN_CANTRANSMIT_HPP_



#include "Task.hpp"              // Base class for tasks
#include "SystemDefines.hpp"     // For CANPeripheral, CANMsg, etc.
#include "CanRegisters.h"        // Include CAN register definitions
#include "FreeRTOS.h"
#include "queue.h"               // RTOS Queue for message handling

// Class definition for CAN transmit functionality
class CanTransmitTask : public Task {
public:
    static CanTransmitTask& Inst();  // Singleton instance method
    void InitTask();                  // Task initialization

protected:
    static void RunTask(void* pvParams); // Static task interface
    void Run(void* pvParams);         // Main run method

private:
    CanTransmitTask();                // Private constructor
    CanTransmitTask(const CanTransmitTask&) = delete; // Prevent copy-construction
    CanTransmitTask& operator=(const CanTransmitTask&) = delete; // Prevent assignment

    // Queue for CAN messages
    QueueHandle_t txQueue;           // Handle for CAN transmission queue
};



#endif /* CAN_CANTRANSMIT_HPP_ */
