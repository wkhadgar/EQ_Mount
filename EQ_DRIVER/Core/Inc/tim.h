/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
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
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern TIM_HandleTypeDef htim2;

extern TIM_HandleTypeDef htim3;

extern TIM_HandleTypeDef htim4;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void MX_TIM4_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN Prototypes */

/**
 * @brief Inicia o led em modo ligado.
 */
void led_set_always_on(void);

/**
 * @brief Inicia o led em modo de piscadas lentas.
 */
void led_set_slow_blink(void);

/**
 * @brief Inicia o led em modo de uma piscada rápida seguida de um intervalo longo.
 */
void led_set_slow_fast_blink(void);

/**
 * @brief Inicia o led em modo de piscadas rápidas.
 */
void led_set_fast_blink(void);

/**
 * @brief Inicia o blink do led.
 */
void led_start_blink(void);

/**
 * @brief Para o estado atual do led, e o desliga.
 */
void led_stop_blink(void);



/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

