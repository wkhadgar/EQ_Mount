/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
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
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */


typedef enum {
    AN_V_BAT = 0,
    OUT_RA_DIR,
    OUT_RA_STEP,
    OUT_DEC_DIR,
    OUT_DEC_STEP,
    IN_SELECT,
    IN_ROTARY_CLKW,
    IN_ROTARY_TRIG,
    I2C_SCL,
    I2C_SDA,
    _pin_amount

} pin_t;

bool_t digitalRead(pin_t pin);
void digitalWrite(pin_t pin, bool_t value);
void digitalToggle(pin_t pin);
void digitalPulse(pin_t pin, uint8_t pulse_ms);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

