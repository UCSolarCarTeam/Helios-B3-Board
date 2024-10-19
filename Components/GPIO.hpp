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
}

#endif /* HELIOS_INCLUDE_SC_CORE_GPIO_H */
