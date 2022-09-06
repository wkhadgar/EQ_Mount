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
#include "stm32f1xx_hal_pwr.h"
#include "stm32f103xb.h"
#include <stdint-gcc.h>
#include <stdbool.h>

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
#define V_BATTERY_Pin GPIO_PIN_0
#define V_BATTERY_GPIO_Port GPIOA
#define M1_DIR_Pin GPIO_PIN_5
#define M1_DIR_GPIO_Port GPIOA
#define M1_STEP_Pin GPIO_PIN_6
#define M1_STEP_GPIO_Port GPIOA
#define M2_DIR_Pin GPIO_PIN_7
#define M2_DIR_GPIO_Port GPIOA
#define M2_STEP_Pin GPIO_PIN_0
#define M2_STEP_GPIO_Port GPIOB
#define SELECT_Pin GPIO_PIN_12
#define SELECT_GPIO_Port GPIOB
#define SELECT_EXTI_IRQn EXTI15_10_IRQn
#define ROTARY_CLKW_Pin GPIO_PIN_13
#define ROTARY_CLKW_GPIO_Port GPIOB
#define ROTARY_TRIG_Pin GPIO_PIN_14
#define ROTARY_TRIG_GPIO_Port GPIOB
#define ROTARY_TRIG_EXTI_IRQn EXTI15_10_IRQn
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define SCL_Pin GPIO_PIN_6
#define SCL_GPIO_Port GPIOB
#define SDA_Pin GPIO_PIN_7
#define SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */