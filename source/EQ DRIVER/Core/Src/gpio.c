/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
        * Free pins are configured automatically as Analog (this feature is enabled through
        * the Code Generation settings)
*/
void MX_GPIO_Init(void) {

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(VDD_SIGNAL_GPIO_Port, VDD_SIGNAL_Pin, GPIO_PIN_SET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, GND_SIGNAL_Pin | M1_ENABLE_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, M1_DIR_Pin | M2_DIR_Pin | M2_ENABLE_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pins : PC13 PC14 PC15 */
    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pins : PA2 PA3 PA8 PA9
                             PA10 PA11 PA12 PA15 */
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_8 | GPIO_PIN_9
                          | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pins : PAPin PAPin */
    GPIO_InitStruct.Pin = VDD_SIGNAL_Pin | M1_ENABLE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = GND_SIGNAL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GND_SIGNAL_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : PBPin PBPin PBPin */
    GPIO_InitStruct.Pin = M1_DIR_Pin | M2_DIR_Pin | M2_ENABLE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : PB11 PB15 PB3 PB4
                             PB5 PB8 PB9 */
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_15 | GPIO_PIN_3 | GPIO_PIN_4
                          | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : PBPin PBPin */
    GPIO_InitStruct.Pin = SELECT_Pin | ROTARY_TRIG_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = ROTARY_CLKW_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(ROTARY_CLKW_GPIO_Port, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */
struct peripheral_t {
    GPIO_TypeDef *GPIOx;
    uint16_t port;
};

static struct {
    struct peripheral_t gpio[_pin_amount];
} self = {
        .gpio = {
                {V_BATTERY_GPIO_Port,   V_BATTERY_Pin},
                {M1_DIR_GPIO_Port,      M1_DIR_Pin},
                {M1_STEP_GPIO_Port,     M1_STEP_Pin},
                {M2_DIR_GPIO_Port,      M2_DIR_Pin},
                {M2_STEP_GPIO_Port,     M2_STEP_Pin},
                {SELECT_GPIO_Port,      SELECT_Pin},
                {ROTARY_CLKW_GPIO_Port, ROTARY_CLKW_Pin},
                {ROTARY_TRIG_GPIO_Port, ROTARY_TRIG_Pin},
                {SCL_GPIO_Port,         SCL_Pin},
                {SDA_GPIO_Port,         SDA_Pin},
                {},
        },
};


bool digitalRead(pin_t pin) {
    bool state = (bool) HAL_GPIO_ReadPin(self.gpio[pin].GPIOx, self.gpio[pin].port);
    return !state;
}

void digitalWrite(pin_t pin, bool value) {
    HAL_GPIO_WritePin(self.gpio[pin].GPIOx, self.gpio[pin].port, (GPIO_PinState) value);
    return;
}

void digitalToggle(pin_t pin) {
    HAL_GPIO_TogglePin(self.gpio[pin].GPIOx, self.gpio[pin].port);
    return;
}

void digitalPulse(pin_t pin, uint8_t pulse_ms) { //nao usar em isr!!!
    HAL_GPIO_TogglePin(self.gpio[pin].GPIOx, self.gpio[pin].port); //start pulse
    HAL_Delay(pulse_ms);
    HAL_GPIO_TogglePin(self.gpio[pin].GPIOx, self.gpio[pin].port); //end pulse
    return;
}
/* USER CODE END 2 */
