/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "horse_running.h"
#include "bitmaps.h"
#include "sh1106.h"
#include "variables.h"
#include "bkp_regs.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ROT_DEBOUNCE_DELAY_MS 10 /** < Delay to accept another rotary move */
#define PUSH_DEBOUNCE_DELAY_MS 300 /** < Delay to accept another button press */
#define DEFAULT_FONT fnt5x7
#define V_BAT_MIN 32 /** < 3.2v * 10 */
#define V_BAT_MAX 42 /** < 4.2v * 10 */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define TICKS_NOW HAL_GetTick() /** < Current sys tick value */
#define SCROLL_DOWN_MENU(menu_top) ((menu_top) < (MENU_SIZE - SCREEN_ROWS)) ? (menu_top)++ : false /** < Increases menu head in-bound */
#define SCROLL_UP_MENU(menu_top) ((menu_top) > 0) ? (menu_top)-- : false /** < Decreases menu head in-bound */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//menu options (limited to given size)
enum menu_option {
    DEC_ = 0,
    RA,
    hemisphere,
    automatic_mode,
    manual_mode,
    brilho_tela,
    tempo_tela,
    save_configs,
    MENU_SIZE //must be the last value
};
const char *menu_str[] = {
        "DEC",
        "R.A",
        "Hemisferio",
        "Modo automatico",
        "Modo manual",
        "Contraste",
        "Luz da tela",
        "Salvar configs"
};
uint16_t menu_op_value[MENU_SIZE] = {0};

const uint8_t SCREEN_ROWS = 5;

uint32_t last_move_ticks = 0; // to track time passed in ms with HAL_GetTick()
uint32_t ra_last_tick, bat_ticks_update = 0;


uint8_t menu_head = 0;
uint8_t menu_selection = 0;
uint8_t battery_charge;
uint8_t frame = 0;

uint16_t value_preview = 0;
uint16_t lock_value = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

#define CURRENT_SELECTION (selection_row+current_menu_top_row);


/**
 * @brief Calculates the attached battery to the A_V_BAT pin charge.
 *
 * @retval uint8_t Battery charge in % value.
 */
uint8_t get_bat_percentage(void);

/**
 * @brief Deals with the rotary encoder changes
 *
 * @retval -1 if the rotary was moved counter-clockwise, 1 if clockwise, 0 if no change happened.
 */
int8_t handle_rotary_events(void);

/**
 * @brief Manage the changes in the menu, updating position variables and value previews.
 *
 * @param value [out] Value to be updated when not in menu
 * @param current_menu_top [out] Value to be updated of the menu head
 * @param arrow_row  [out] Value of the selection arrow on menu screen
 * @param increase boolean telling whether to increase or decrease the current screen parameters
 */
void handle_menu_changes(uint8_t *current_menu_top, uint8_t *arrow_row, uint16_t *op_value, bool increase);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == ROTARY_TRIG_Pin) {
        set_flag(rotary_triggered);
        if (ROTARY_CLKW_GPIO_Port->IDR & ROTARY_CLKW_Pin) {
            set_flag(ccw);
        }
    } else if (GPIO_Pin == SELECT_Pin) {
        set_flag(selected);
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_ADC2_Init();
    MX_TIM2_Init();
    /* USER CODE BEGIN 2 */

    SH1106_cleanInit();

    /** data recover */
    battery_charge = get_bat_percentage();
    menu_op_value[brilho_tela] = BKP_read(0);
    menu_op_value[hemisphere] = BKP_read(1);
    menu_op_value[tempo_tela] = BKP_read(2);

    /**start logo display */
    SH1106_drawBitmapFullscreen(eqmount_logo);
    SH1106_flush();
    SH1106_clear();
    HAL_Delay(2500);

    /** transition to menu fake load */
    SH1106_printStr(16, 15, "Carregando Menu", fnt5x7);
    for (uint8_t s = 0; s <= 100; s += 2) {
        SH1106_drawRoundRectFill(s, 7, 32, 110, 8);
        SH1106_flush();
        HAL_Delay(1);
    }
    SH1106_clear();

    last_move_ticks = TICKS_NOW; /** < start time reference */
    ra_last_tick = last_move_ticks; /** < RA tracking time reference */
    bat_ticks_update = last_move_ticks; /** < time tracking to help make periodic battery checks */

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

        { /** horse running easter egg */
            if (get_flag(toggle_horse)) {
                if (frame > 14) frame = 0;
                SH1106_drawBitmapFullscreen(horse_running[frame++]);
                SH1106_flush();
            }
        }

        SH1106_clear(); /** < clears buffer, to construct new one and flush it later */

        { /** battery info */
            if (TICKS_NOW - bat_ticks_update > 10000) {
                battery_charge = get_bat_percentage();
                if (battery_charge < 20) set_flag(low_battery);
                bat_ticks_update = TICKS_NOW;
            }
            SH1106_drawRoundRectFill(battery_charge, 105, 2, 20, 5);
            if (get_flag(low_battery)) SH1106_drawBitmap(96, 1, 5, 8, alert);
        }

        handle_menu_changes(&menu_head, &menu_selection, &value_preview, handle_rotary_events());

        if (get_flag(update_display)) { /** < checks if changes to the buffer happened, and if so, flush them */
            reset_flag(update_display);
            SH1106_flush();
        }
    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */
uint8_t get_bat_percentage(void) {
    HAL_ADC_Start(&hadc2);

    uint16_t read_voltage = voltage_read(5 * 10); //5v * 10 of reference on board
    uint8_t percent = ((read_voltage >= V_BAT_MIN ? read_voltage : V_BAT_MIN) - V_BAT_MIN) *
                      (100 / (V_BAT_MAX - V_BAT_MIN));    // converting [bat_min, bat_max] to [0, 100]

    HAL_ADC_Stop(&hadc2);

    return percent;
}


int8_t handle_rotary_events(void) {
    if (get_flag(rotary_triggered)) { // rotary encoder triggered
        reset_flag(rotary_triggered);
        set_flag(wake);

        if ((TICKS_NOW - last_move_ticks) >= ROT_DEBOUNCE_DELAY_MS) {

            last_move_ticks = TICKS_NOW;

            if (get_flag(ccw)) { //counter-clockwise rotation

                reset_flag(ccw);
                return -1;
            } else { // clockwise rotation
                return 1;
            }
        }
    } else if (get_flag(selected)) { // rotary encoder pressed
        reset_flag(selected);
        set_flag(wake);

        if ((TICKS_NOW - last_move_ticks) >= PUSH_DEBOUNCE_DELAY_MS) {

            last_move_ticks = TICKS_NOW;
            set_flag(wake);

            if (get_flag(on_menu)) reset_flag(on_menu);
            else set_flag(on_menu);

        }
    }
    return 0;
}

void handle_menu_changes(uint8_t *current_menu_top, uint8_t *arrow_row, uint16_t *op_value, bool increase) {


    if (increase) {
        if (get_flag(on_menu)) {
            if (*arrow_row < SCREEN_ROWS) {
                *arrow_row++;
            } else {
                SCROLL_DOWN_MENU(*current_menu_top);
            }
        } else {
            *op_value++;
        }
    } else {
        if (get_flag(on_menu)) {
            if (*arrow_row > 0) {
                *arrow_row--;
            } else {
                SCROLL_UP_MENU(*current_menu_top);
            }
        } else {
            *op_value--;
        }
    }

}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    /* USER CODE BEGIN Callback 0 */

    /* USER CODE END Callback 0 */
    if (htim->Instance == TIM1) {
        HAL_IncTick();
    }
    /* USER CODE BEGIN Callback 1 */

    /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */