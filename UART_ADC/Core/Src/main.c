/* USER CODE BEGIN Header */
#include "stm32f4xx_hal.h"
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "eth.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
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
int counter = 0;
static uint32_t before_tick = 0;
char uart_buf[100];
int adc_value;
char led_value[] = "Green(LD1), Blue(LD2), Red(LD3)";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
static void displayDigit(uint8_t digit);
static void GPIO_Config(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
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
  MX_ETH_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_ADC1_Init();
  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_IT(&hadc1);
  GPIO_Config();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // Check the state of the Up button
	  if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == GPIO_PIN_RESET)
	  {
		if (HAL_GetTick() - before_tick >= 300)
		{
			before_tick = HAL_GetTick();

			counter++;

			if (counter > 9)
			{
			  counter = 0;
			}

			displayDigit(counter);
		}
	  }

	  // Check the state of the Down button
	  else if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_RESET)
	  {
		if (HAL_GetTick() - before_tick >= 300)
		{
			before_tick = HAL_GetTick();

			counter--;

			if (counter < 0)
			{
			  counter = 9;
			}

			displayDigit(counter);
		}
	  }

	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1, 10);
	  adc_value = HAL_ADC_GetValue(&hadc1);

	  if (adc_value >= 0 && adc_value <= 1000)
	  {
		  HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOB, LD2_Pin|LD3_Pin, GPIO_PIN_RESET);
		  strcpy(led_value, "Green(LD1)");
	  }
	  else if (adc_value >= 1001 && adc_value <= 2000)
	  {
		  HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin, GPIO_PIN_RESET);
		  strcpy(led_value, "Blue(LD2)");
	  }
	  else if (adc_value >= 2001 && adc_value <= 4000)
	  {
		  HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD2_Pin, GPIO_PIN_RESET);
		  strcpy(led_value, "Red(LD3)");
	  }

	  HAL_ADC_Stop(&hadc1);

	  memset(uart_buf, 0, sizeof(uart_buf));
	  sprintf(uart_buf, "ADC Value: %d, LED State: %s, Up/Down Value: %d \r\n", adc_value, led_value, counter);
	  HAL_UART_Transmit(&huart3, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);
	  HAL_Delay(250);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* USART3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* EXTI15_10_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* USER CODE BEGIN 4 */
void displayDigit(uint8_t digit)
{
  // Define the 7-segment display patterns for digits 0 to 9
  const uint8_t segmentPatterns[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };

  // Output the pattern to the 7-segment display
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, (segmentPatterns[digit] & 0x01) ? GPIO_PIN_RESET : GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, (segmentPatterns[digit] & 0x02) ? GPIO_PIN_RESET : GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, (segmentPatterns[digit] & 0x04) ? GPIO_PIN_RESET : GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, (segmentPatterns[digit] & 0x08) ? GPIO_PIN_RESET : GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, (segmentPatterns[digit] & 0x10) ? GPIO_PIN_RESET : GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5, (segmentPatterns[digit] & 0x20) ? GPIO_PIN_RESET : GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_6, (segmentPatterns[digit] & 0x40) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void GPIO_Config(void)
{
	  GPIOB -> MODER |= (1<<14);

	  GPIOB -> OTYPER = 0;
	  GPIOB -> OSPEEDR = 0;
	  GPIOB -> PUPDR &= ~((1<<10) | (1<<11));
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
