/**
 ******************************************************************************
 * File Name          : GPIO.hpp
 * Description        :
 *
 *    GPIO contains all GPIO pins wrapped in a namespace and corresponding functions
 *
 *    All GPIO pins should be controlled through this abstraction layer to ensure readable control.
 *
 ******************************************************************************
*/
#ifndef HELIOS_INCLUDE_SC_CORE_GPIO_H
#define HELIOS_INCLUDE_SC_CORE_GPIO_H
#include "SystemDefines.hpp"
#include "main.h"
#include "stm32l1xx_hal.h"

namespace GPIO
{

    namespace LED_RED
    {
        inline void On() { HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET); }
        inline void Off() { HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET); }
        inline void Toggle() { HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin); }

        inline bool IsOn() { return HAL_GPIO_ReadPin(LED_RED_GPIO_Port, LED_RED_Pin) == GPIO_PIN_SET; }
    }

    namespace LED_BLUE
    {
        inline void On() { HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET); }
        inline void Off() { HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET); }
        inline void Toggle() { HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin); }

        inline bool IsOn() { return HAL_GPIO_ReadPin(LED_BLUE_GPIO_Port, LED_BLUE_Pin) == GPIO_PIN_SET; }
    }

    namespace LED_GREEN
    {
        inline void On() { HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET); }
        inline void Off() { HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET); }
        inline void Toggle() { HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin); }

        inline bool IsOn() { return HAL_GPIO_ReadPin(LED_GREEN_GPIO_Port, LED_GREEN_Pin) == GPIO_PIN_SET; }
    }
    namespace BOARD_SELECT_0
    {
        inline void On() {HAL_GPIO_WritePin(Board_SLCT_0_GPIO_Port, Board_SLCT_0_Pin, GPIO_PIN_SET); }
        inline void Off() {HAL_GPIO_WritePin(Board_SLCT_0_GPIO_Port, Board_SLCT_0_Pin, GPIO_PIN_RESET); }
    }
    namespace BOARD_SELECT_1
    {
        inline void On() {HAL_GPIO_WritePin(Board_SLCT_1_GPIO_Port, Board_SLCT_1_Pin, GPIO_PIN_SET); }
        inline void Off() {HAL_GPIO_WritePin(Board_SLCT_1_GPIO_Port, Board_SLCT_1_Pin, GPIO_PIN_RESET); }
    }
    namespace SPI_DATA_CS0
    {
        inline void On() {HAL_GPIO_WritePin(SPI_Data_CS0_GPIO_Port, SPI_Data_CS0_Pin, GPIO_PIN_SET); }
        inline void Off() {HAL_GPIO_WritePin(SPI_Data_CS0_GPIO_Port, SPI_Data_CS0_Pin, GPIO_PIN_RESET); }
    }
    namespace SPI_DATA_CS1
    {
        inline void On() {HAL_GPIO_WritePin(SPI_Data_CS1_GPIO_Port, SPI_Data_CS1_Pin, GPIO_PIN_SET); }
        inline void Off() {HAL_GPIO_WritePin(SPI_Data_CS1_GPIO_Port, SPI_Data_CS1_Pin, GPIO_PIN_RESET); }
    }
}

#endif /* HELIOS_INCLUDE_SC_CORE_GPIO_H */
