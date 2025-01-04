/**
 ******************************************************************************
 * File Name          : CANRxTask.cpp
 * Description        : Task for receiving CAN messages
 ******************************************************************************
 */

#include "main.h"
#include "Queue.hpp"
#include "Mutex.hpp"
#include "CAN.h"
#include "CanRxTask.hpp"

void CANRxTask::Run(void *pvParams)
{
    //ConfigureCANSPI(&peripheral1);
    Queue CAN_RX_QUEUE = Queue();
    // Mutex

    while (1)
    {
        /***
         * Plan:
         * 
         * 1. Detect RX Interrupt RXBUFF
         *      - Set interrupts RXBuff pg 21
         *      - RXBUFF0 and RXBUFF1 . 
         * 2. Queue buffer contents 
         *      - CAN_RX_QUEUE.SendFromISR()
         *      - Parse data into command 
         *            receiveCANMessage()
         * 3. Mutex the SPI Peripheral?
         *      Mutex 
         * 4. Handle buffer
         */
    }
}
