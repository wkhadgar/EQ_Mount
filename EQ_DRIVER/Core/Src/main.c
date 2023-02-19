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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "variables.h"
#include "steppers.h"
#include "astro_conv.h"
#include "astro_targets.h"
#include "PA6H.h"
#include "nrf24l01p.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/**
 * @brief menu options enum
 */
enum menu_options {
    DEC_ = 0,
    RA,
    hemisphere,
    automatic_mode,
    manual_mode,
    MENU_SIZE, //must be the last value
};

struct {
    int16_t DEC;
    uint16_t RA;

    bool hemisphere;
    bool automatic_mode;
    bool manual_mode;
} menu_values = {0};

uint8_t received_payload[32];
GNSS_data_t* GNSS;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
bool get_data(uint8_t* nRF24_payload, uint8_t payload_length);

void nRF24_RX_ESB_setup(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    if (huart == &huart1) {
        if (GNSS_UART_CallBack(GNSS)) {
            astro_update_LMST();
        }
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
    GNSS = astro_get_gnss_pointer();
    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_ADC1_Init();
    MX_TIM3_Init();
    MX_USART1_UART_Init();
    MX_SPI1_Init();
    MX_TIM4_Init();
    /* USER CODE BEGIN 2 */

    HAL_Delay(500);
    GNSS_init();
    astro_init();
    nRF24_RX_ESB_setup();

    led_start_blink();


    /** Ensure NRF24L01 connection. */
    if (!nRF24_Check()) {
        led_set_fast_blink();
    }
    led_set_slow_blink();
    while (!get_flag(NRF_IRQ)) {
        /** Wait for controller connection. */
    }
    led_set_always_on();

    uint8_t init_payload[10] = "INIT";
    while (!get_data(received_payload, 10)) {
        for (uint8_t i = 0; i < 5; i++) {
            if (received_payload[i] != init_payload[i]) {
                reset_flag(NRF_OK);
                break;
            }
            set_flag(NRF_OK);
            led_set_slow_fast_blink();

        }
    }
    reset_flag(NRF_IRQ);

    bool flag = true;

    astro_release();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

        astro_update_raw_fine_adjusts();

        if (flag) {
            astro_go_home();
        }

        while (astro_get_gnss_pointer()->is_valid != VALID_DATA) {
            /** Wait for GNSS fix. */
        }

        if (flag) {
            astro_update_target(astro_target[JUPITER]);
            astro_goto_target();
            flag = false;
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
void nRF24_RX_ESB_setup(void) {
    // This is simple receiver with Enhanced ShockBurst:
//   - RX address: 'ESB'
//   - payload: 10 bytes
//   - RF channel: 40 (2440MHz)
//   - data rate: 250kbps
//   - CRC scheme: 2 byte

// The transmitter sends a 10-byte packets to the address 'ESB' with Auto-ACK (ShockBurst enabled)

// Set RF channel
    nRF24_SetRFChannel(40);

// Set data rate
    nRF24_SetDataRate(nRF24_DR_2Mbps);

// Set CRC scheme
    nRF24_SetCRCScheme(nRF24_CRC_2byte);

// Set address width, it's common for all pipes (RX and TX)
    nRF24_SetAddrWidth(3);

// Configure RX PIPE
    static const uint8_t nRF24_ADDR[] = {'E', 'S', 'B'};

    // program address for pipe
    nRF24_SetAddr(nRF24_PIPE1, nRF24_ADDR);
    // Auto-ACK: enabled, payload length: 10 bytes
    nRF24_SetRXPipe(nRF24_PIPE1, nRF24_AA_ON, 10);

// Set TX power for Auto-ACK (maximum, to ensure that transmitter will hear ACK reply)
    nRF24_SetTXPower(nRF24_TXPWR_0dBm);

// Set operational mode (PRX == receiver)
    nRF24_SetOperationalMode(nRF24_MODE_RX);

// Clear any pending IRQ flags
    nRF24_ClearIRQFlags();

// Wake the transceiver
    nRF24_SetPowerMode(nRF24_PWR_UP);

}


bool get_data(uint8_t* nRF24_payload, uint8_t payload_length) {

    if (nRF24_GetStatus_RXFIFO() != nRF24_STATUS_RXFIFO_EMPTY) {
        // Get a payload from the transceiver
        nRF24_ReadPayload(nRF24_payload, &payload_length);

        // Clear all pending IRQ flags
        nRF24_ClearIRQFlags();
        return true;
    }

    return false;
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
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
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
    while (1);
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