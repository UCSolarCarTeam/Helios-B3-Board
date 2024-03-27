/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"

#include "stm32l1xx_ll_usart.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_cortex.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_dma.h"

#include "stm32l1xx_ll_exti.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define board_int_1_Pin GPIO_PIN_13
#define board_int_1_GPIO_Port GPIOC
#define LED_RED_Pin GPIO_PIN_1
#define LED_RED_GPIO_Port GPIOC
#define LED_BLUE_Pin GPIO_PIN_2
#define LED_BLUE_GPIO_Port GPIOC
#define LED_GREEN_Pin GPIO_PIN_3
#define LED_GREEN_GPIO_Port GPIOC
#define DEBUG_TX_Pin GPIO_PIN_2
#define DEBUG_TX_GPIO_Port GPIOA
#define DEBUG_RX_Pin GPIO_PIN_3
#define DEBUG_RX_GPIO_Port GPIOA
#define SPI_SCK_Data_Pin GPIO_PIN_5
#define SPI_SCK_Data_GPIO_Port GPIOA
#define SPI_MISO_Data_Pin GPIO_PIN_6
#define SPI_MISO_Data_GPIO_Port GPIOA
#define SPI_MOSI_Data_Pin GPIO_PIN_7
#define SPI_MOSI_Data_GPIO_Port GPIOA
#define board_int_4_Pin GPIO_PIN_0
#define board_int_4_GPIO_Port GPIOB
#define I2C_SCL_Expander_Pin GPIO_PIN_10
#define I2C_SCL_Expander_GPIO_Port GPIOB
#define I2C_SDA_Expander_Pin GPIO_PIN_11
#define I2C_SDA_Expander_GPIO_Port GPIOB
#define SPI_SCK_CAN_Pin GPIO_PIN_13
#define SPI_SCK_CAN_GPIO_Port GPIOB
#define SPI_MISO_CAN_Pin GPIO_PIN_14
#define SPI_MISO_CAN_GPIO_Port GPIOB
#define SPI_MOSI_CAN_Pin GPIO_PIN_15
#define SPI_MOSI_CAN_GPIO_Port GPIOB
#define CAN_RX1BF_Pin GPIO_PIN_6
#define CAN_RX1BF_GPIO_Port GPIOC
#define CAN_RX0BF_Pin GPIO_PIN_7
#define CAN_RX0BF_GPIO_Port GPIOC
#define CS_CAN_N_Pin GPIO_PIN_8
#define CS_CAN_N_GPIO_Port GPIOC
#define CAN_INT_Pin GPIO_PIN_9
#define CAN_INT_GPIO_Port GPIOC
#define VBUS_DETECT_Pin GPIO_PIN_8
#define VBUS_DETECT_GPIO_Port GPIOA
#define SPI_Data_CS1_Pin GPIO_PIN_10
#define SPI_Data_CS1_GPIO_Port GPIOC
#define SPI_Data_CS0_Pin GPIO_PIN_11
#define SPI_Data_CS0_GPIO_Port GPIOC
#define Board_SLCT_1_Pin GPIO_PIN_12
#define Board_SLCT_1_GPIO_Port GPIOC
#define Board_SLCT_0_Pin GPIO_PIN_2
#define Board_SLCT_0_GPIO_Port GPIOD
#define I2C_SCL_Data_Pin GPIO_PIN_6
#define I2C_SCL_Data_GPIO_Port GPIOB
#define I2C_SDA_Data_Pin GPIO_PIN_7
#define I2C_SDA_Data_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
