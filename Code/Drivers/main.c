/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx.h"
//#include "stm32f0xx_hal.h"
//#include "stm32f0xx_hal_def.h"
//#include "system_stm32f0xx.h"
#include "stdbool.h"
#include <stdbool.h>
//#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
ADC_HandleTypeDef hadc;

TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
//unsigned long currentMillis = millis();
const unsigned long delayTime = 10;
unsigned long previousMillis = 0;
unsigned long currentMillis;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM14_Init(void);
int myTimer1(long delayTime, long currentMillis);
void int_to_string(int value, char* str, int size);
void strcopy(char* destination, char* source, int begin, int end);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void int_to_string(int value, char* str, int size) {
	for(int i = size-1; i >= 0; i--) {
		str[i] ='0' + value%10;
		value /= 10;
	}
}

void strcopy(char* destination, char* source, int begin, int end) {
	for(int i = begin; i <= end; i++) {
		destination[i] = source[i-begin];
	}
}

int HAL_TIM_PeriodElapsedCallBack(TIM_HandleTypeDef *htim14){
	if(currentMillis - previousMillis >= delayTime){
		previousMillis = currentMillis;
		return 1;
	}else{
		return 0;
	}
}

// First event timer
int myTimer1(long delayTime, long currentMillis){
  if(currentMillis - previousMillis >= delayTime){
	  previousMillis = currentMillis;
	  return 1;
  }else{
	  return 0;
  }
}


/*
 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void){
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
  MX_ADC_Init();
  MX_USART1_UART_Init();
  MX_TIM14_Init();

  /* USER CODE BEGIN 2 */

  //Variables for Calculate BPM
  // Up 2072 Low 1957 ValorCris -> Up 2000 Low 1357
  int UpperThreshold = 2000;
  int LowerThreshold = 1357;
  int Signal = 0;
  float BPM = 0.0;
  bool IgnoreReading = false;
  bool FirstPulseDetected = false;
  unsigned long FirstPulseTime = 0;
  unsigned long SecondPulseTime = 0;
  unsigned long PulseInterval = 0;

  //Variables for UART Communication
  char msg[50];
  char valueStr[4];
  HAL_StatusTypeDef x;
  HAL_StatusTypeDef y;
  uint8_t buf1[100];
  memset(buf1,0,100);

  //Message to thingspeak
  strcpy(msg,"GET /update?api_key=DY2N954X12ZN5Q6K&field1=0000\r\n");
  //ESP answers OK
  x = HAL_UART_Transmit(&huart1, (uint8_t *)"AT\r\n", 4, 2000);
  y = HAL_UART_Receive(&huart1, buf1, 100, 3000);
  HAL_Delay(300);
  //Sets the Wi-Fi mode as station mode (client)
  HAL_UART_Transmit(&huart1, (uint8_t *)"AT+CWMODE=1\r\n", 13, 2000);
  HAL_UART_Receive(&huart1, buf1, 100, 3000);
  HAL_Delay(300);
  //Name and password of the Wi-Fi
  HAL_UART_Transmit(&huart1, (uint8_t *)"AT+CWJAP=\"REDE_WIFI\",\"SENHA404\"\r\n", 50, 2000);
  HAL_UART_Receive(&huart1, buf1, 100, 10000);
  HAL_Delay(300);
  //Sets the communication mode as single mode
  HAL_UART_Transmit(&huart1, (uint8_t *)"AT+CIPMUX=0\r\n", 13, 2000);
  HAL_UART_Receive(&huart1, buf1, 100, 2000);

  /* USER CODE END 2 */
  HAL_TIM_Base_Start_IT(&htim14);
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1){

	  HAL_ADC_Start(&hadc);
	  Signal = HAL_ADC_GetValue(&hadc);


      // Heart beat leading edge detected.
      if(Signal < UpperThreshold && IgnoreReading == false){
    	  if(FirstPulseDetected == false){
    		  FirstPulseTime = HAL_GetTick();
    		  FirstPulseDetected = true;
    	  }else{
    		  SecondPulseTime = HAL_GetTick();
    		  PulseInterval = SecondPulseTime - FirstPulseTime;
    		  FirstPulseTime = SecondPulseTime;
    	  }
    	  IgnoreReading = true;
      }

      // Heart beat trailing edge detected.
      if(Signal > LowerThreshold){
    	  IgnoreReading = false;
      }

      BPM = (2*(1.0/Signal)) * 60.0 * 1000;
	  HAL_Delay(1000);

	  //Add this value to the message
	  int_to_string(BPM, valueStr, 4);
	  strcopy(msg, valueStr, 44, 47);
	  //Starts the communication
	  HAL_UART_Transmit(&huart1, (uint8_t *)"AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n", 70, 2000);
	  HAL_UART_Receive(&huart1, buf1, 100, 5000);
	  HAL_Delay(1000);
	  //Inform the number of bytes will be sended
	  HAL_UART_Transmit(&huart1, (uint8_t *)"AT+CIPSEND=50\r\n", 15, 2000);
	  HAL_UART_Receive(&huart1, buf1, 100, 5000);
	  HAL_Delay(500);
	  //Transmit the message to thinkspeak through the ESP
	  HAL_UART_Transmit(&huart1, msg, 50, 2000);
	  HAL_UART_Receive(&huart1, buf1, 100, 5000);
	  HAL_Delay(20000);

	  /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void){
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK){
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void){

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  if (HAL_ADC_Init(&hadc) != HAL_OK){
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted. 
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK){
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void){

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 7999;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 99;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK){
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void){

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK){
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void){
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void){
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
void assert_failed(char *file, uint32_t line){
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
