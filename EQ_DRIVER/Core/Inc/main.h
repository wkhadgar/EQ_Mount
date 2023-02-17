/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
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
#define FINE_ADJUST_Pin GPIO_PIN_0
#define FINE_ADJUST_GPIO_Port GPIOA
#define M1_STEP_Pin GPIO_PIN_6
#define M1_STEP_GPIO_Port GPIOA
#define M1_ENABLE_Pin GPIO_PIN_7
#define M1_ENABLE_GPIO_Port GPIOA
#define M1_DIR_Pin GPIO_PIN_0
#define M1_DIR_GPIO_Port GPIOB
#define M2_DIR_Pin GPIO_PIN_1
#define M2_DIR_GPIO_Port GPIOB
#define M2_ENABLE_Pin GPIO_PIN_10
#define M2_ENABLE_GPIO_Port GPIOB
#define M2_STEP_Pin GPIO_PIN_11
#define M2_STEP_GPIO_Port GPIOB
#define LED_GND_Pin GPIO_PIN_13
#define LED_GND_GPIO_Port GPIOB
#define NRF_CSN_Pin GPIO_PIN_14
#define NRF_CSN_GPIO_Port GPIOB
#define NRF_IRQ_Pin GPIO_PIN_15
#define NRF_IRQ_GPIO_Port GPIOB
#define NRF_IRQ_EXTI_IRQn EXTI15_10_IRQn
#define GPS_TX_Pin GPIO_PIN_9
#define GPS_TX_GPIO_Port GPIOA
#define GPS_RX_Pin GPIO_PIN_10
#define GPS_RX_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define SPI_CLK_Pin GPIO_PIN_3
#define SPI_CLK_GPIO_Port GPIOB
#define SPI_MISO_Pin GPIO_PIN_4
#define SPI_MISO_GPIO_Port GPIOB
#define SPI_MOSI_Pin GPIO_PIN_5
#define SPI_MOSI_GPIO_Port GPIOB
#define LED_PWM_Pin GPIO_PIN_8
#define LED_PWM_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
