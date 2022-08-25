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
#include "gpio.h"
#include "i2c.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "horse_running.h"
#include "sh1106.h"
#include "variables.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ROT_DEBOUNCE_DELAY_MS 10
#define PUSH_DEBOUNCE_DELAY_MS 300
#define DEFAULT_FONT fnt5x7
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define TICKS_NOW HAL_GetTick()
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == ROTARY_TRIG_Pin) {
    set_flag(rotary_triggered);
    if (ROTARY_CLKW_GPIO_Port->IDR & ROTARY_CLKW_Pin)
      set_flag(ccw);
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

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  //menu options (limited to given size)
  enum menu_option{
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
  const char* menu_str[] = {
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

  uint32_t lock_value;

  uint32_t last_move_ticks = 0; // to track time passed in ms with HAL_GetTick()
  uint32_t ra_last_tick = 0;

  uint8_t actual_menu_top_row = 0;
  uint8_t arrow_row = 0;
  uint8_t current_selection;

  bool_t wake_flag     = False;
  bool_t select_pressed = 0;
  bool_t sleeping = False;
  bool_t on_menu = 1;
  bool_t low_battery_flag = False;
  bool_t toggle_horse = False;

  uint8_t frame = 0;
  uint16_t rot_val = 0;


  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_ADC2_Init();
  /* USER CODE BEGIN 2 */

  SH1106_cleanInit();
  uint16_t data_readed = 0;


  //enabling bkp write
  //BKP_TypeDef * DRx = BKP;
  //RCC_TypeDef* RCCAPB = RCC;
  //PWR_TypeDef* PWR_C = PWR;
  //RCCAPB->APB1ENR &= ~(0b11 << 27);
  //PWR->CR &= ~(1 << 8);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

	//data_readed = (DRx->DR1 << 16) >> 16;
    /* USER CODE BEGIN 3 */
    rot_val = incremented_var(rot_value, 0);

    if (toggle_horse) {
      if (frame > 14)
        frame = 0;
      SH1106_drawBitmapFullscreen(horse_running[frame++]);
      SH1106_flush();
      HAL_Delay(2);
    }

    SH1106_clear();

    if (get_flag(rotary_triggered)) { // rotary encoder triggered
      reset_flag(rotary_triggered);
      if ((TICKS_NOW - last_move_ticks) >= ROT_DEBOUNCE_DELAY_MS) {

        set_flag(update_display);
        // digitalToggle(OUT_RA_DIR);

        if (get_flag(ccw)) {
          reset_flag(ccw);

          if (rot_val > 0) {
            incremented_var(rot_value, -1);
          }
        } else { // clockwise rotation
          incremented_var(rot_value, 1);
        }
        SH1106_drawCircle(63, 31, rot_val);

        last_move_ticks = TICKS_NOW;
      }
    }

    else if (get_flag(selected)) { // rotary encoder trigged
      reset_flag(selected);
      if ((TICKS_NOW - last_move_ticks) >= PUSH_DEBOUNCE_DELAY_MS) {

        SH1106_setContrast(255);
        set_flag(update_display);
        toggle_horse = !toggle_horse;

        //DRx->DR1 &= ~0b1010;

        last_move_ticks = TICKS_NOW;
      }
    }

    if (get_flag(update_display)) {
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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
