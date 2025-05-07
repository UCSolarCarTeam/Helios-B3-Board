/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32l1xx_it.h
  * @brief   This file contains the headers of the interrupt handlers.
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
#ifndef __STM32L1xx_IT_H
#define __STM32L1xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

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
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);
void TIM2_IRQHandler(void);
void USART2_IRQHandler(void);
//added the below functions for motorsafety task
void DMA1_Channel1_IRQHandler(void);        // ADC DMA interrupt
void DMA1_Channel4_IRQHandler(void);        // USART1 TX DMA interrupt
void DMA1_Channel6_IRQHandler(void);        // USART2 RX DMA interrupt
void ADC1_IRQHandler(void);                 // ADC interrupt
void USART1_IRQHandler(void);               // USART1 interrupt
void USART2_IRQHandler(void);               // USART2 interrupt
void TIM2_IRQHandler(void);                 // TIM2 interrupt
void TIM6_IRQHandler(void);                 // TIM6 interrupt
void TIM7_IRQHandler(void);                 // TIM7 interrupt
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __STM32L1xx_IT_H */
