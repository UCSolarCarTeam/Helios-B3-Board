/**
 ******************************************************************************
 * File Name          : main_system.hpp
 * Description        : Header file for main_system.cpp, acts as an interface between
 *  STM32CubeIDE and our application.
 ******************************************************************************
*/
#ifndef MAIN_SYSTEM_HPP_
#define MAIN_SYSTEM_HPP_

/* Includes  ----------------------------------------------------------------------------*/
#include "Mutex.hpp"
// Board specific includes
#include "stm32l1xx_hal.h"
#include "stm32l1xx_ll_usart.h"
#include "stm32l1xx_hal_rcc.h"
#include "stm32l1xx_ll_dma.h"



/* Interface Functions ------------------------------------------------------------------*/
/* These functions act as our program's 'main' and any functions inside CubeIDE's main --*/
void run_main();
void run_StartDefaultTask();

/* Global Functions ------------------------------------------------------------------*/

/* Global Variable Interfaces ------------------------------------------------------------------*/
/* All must be extern from main_system.cpp -------------------------------------------------*/


/* Globally Accessible Drivers ------------------------------------------------------------------*/
// UART Driver
class UARTDriver;
namespace Driver {
    extern UARTDriver uart2;
}
namespace UART {
    constexpr UARTDriver* Debug = &Driver::uart2;
}


/* System Handles ------------------------------------------------------------------*/
extern CRC_HandleTypeDef hcrc;       // CRC - Hardware CRC System Handle

namespace SystemHandles {
    constexpr CRC_HandleTypeDef* CRC_Handle = &hcrc;
}



#endif /* MAIN_SYSTEM_HPP_ */
