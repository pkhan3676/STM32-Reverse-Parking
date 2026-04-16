/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "ssd1306.h"
#include "ssd1306_fonts.h"
/* USER CODE END Includes */

CAN_HandleTypeDef hcan1;
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint32_t IC1_1=0, IC1_2=0;
uint8_t cap1=0;
float dist1=0;

uint32_t IC2_1=0, IC2_2=0;
uint8_t cap2=0;
float dist2=0;

uint32_t IC3_1=0, IC3_2=0;
uint8_t cap3=0;
float dist3=0;

// Averaging Filter - 3 readings
#define AVG_SIZE 3
float dist1_buf[AVG_SIZE] = {0};
float dist2_buf[AVG_SIZE] = {0};
float dist3_buf[AVG_SIZE] = {0};
uint8_t avg_idx = 0;
float dist1_avg = 0;
float dist2_avg = 0;
float dist3_avg = 0;

CAN_TxHeaderTypeDef TxHeader;
uint32_t TxMailbox;
uint8_t TxData[8];
char msg[100];
/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_I2C1_Init(void);

/* USER CODE BEGIN 0 */
void Trigger_S1(void)
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
}

void Trigger_S2(void)
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
}

void Trigger_S3(void)
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
}

void LED_Update(uint16_t min_dist)
{
  if(min_dist <= 5)
  {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
  }
  else if(min_dist <= 10)
  {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
  }
  else
  {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);
  }
}
/* USER CODE END 0 */

int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_CAN1_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_I2C1_Init();

  /* USER CODE BEGIN 2 */
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);
  HAL_CAN_Start(&hcan1);

  TxHeader.DLC = 6;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.StdId = 0x123;
  TxHeader.TransmitGlobalTime = DISABLE;

  // OLED Init
  ssd1306_Init();
  HAL_Delay(100);

  // Startup Screen
  ssd1306_Fill(Black);
  ssd1306_SetCursor(15, 0);
  ssd1306_WriteString("CAR PARKING", Font_7x10, White);
  ssd1306_SetCursor(10, 15);
  ssd1306_WriteString("SENSOR SYSTEM", Font_7x10, White);
  ssd1306_SetCursor(25, 35);
  ssd1306_WriteString("3 Sensors", Font_7x10, White);
  ssd1306_SetCursor(20, 50);
  ssd1306_WriteString("Starting...", Font_7x10, White);
  ssd1306_UpdateScreen();
  HAL_Delay(2000);
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
    // Trigger Sensors
    Trigger_S1();
    HAL_Delay(100);

    Trigger_S2();
    HAL_Delay(100);

    Trigger_S3();
    HAL_Delay(100);

    // Store in buffer
    dist1_buf[avg_idx] = dist1;
    dist2_buf[avg_idx] = dist2;
    dist3_buf[avg_idx] = dist3;
    avg_idx = (avg_idx + 1) % AVG_SIZE;

    // Calculate 3-reading average
    float sum1=0, sum2=0, sum3=0;
    for(int i=0; i<AVG_SIZE; i++)
    {
      sum1 += dist1_buf[i];
      sum2 += dist2_buf[i];
      sum3 += dist3_buf[i];
    }
    dist1_avg = sum1 / AVG_SIZE;
    dist2_avg = sum2 / AVG_SIZE;
    dist3_avg = sum3 / AVG_SIZE;

    // Limit max distance to 200cm
    if(dist1_avg > 200) dist1_avg = 200;
    if(dist2_avg > 200) dist2_avg = 200;
    if(dist3_avg > 200) dist3_avg = 200;

    uint16_t d1 = (uint16_t)dist1_avg;
    uint16_t d2 = (uint16_t)dist2_avg;
    uint16_t d3 = (uint16_t)dist3_avg;

    // Min Distance
    uint16_t min_dist = d1;
    if(d2 < min_dist) min_dist = d2;
    if(d3 < min_dist) min_dist = d3;

    // LED Update
    LED_Update(min_dist);

    // UART Debug
    sprintf(msg, "D1:%d D2:%d D3:%d Min:%d\r\n",
            d1, d2, d3, min_dist);
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);

    // CAN Send
    TxData[0] = d1 >> 8;
    TxData[1] = d1 & 0xFF;
    TxData[2] = d2 >> 8;
    TxData[3] = d2 & 0xFF;
    TxData[4] = d3 >> 8;
    TxData[5] = d3 & 0xFF;

    if(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0)
    {
      HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
    }

    // OLED Update
    ssd1306_Fill(Black);

    // Title
    ssd1306_SetCursor(15, 0);
    ssd1306_WriteString("PARKING SENSOR", Font_7x10, White);

    // Line
    for(int x=0; x<128; x++)
      ssd1306_DrawPixel(x, 11, White);

    // S1
    ssd1306_SetCursor(0, 14);
    sprintf(msg, "S1:%3dcm", d1);
    ssd1306_WriteString(msg, Font_7x10, White);
    uint8_t bar1 = (d1 >= 50) ? 60 : (d1*60)/50;
    for(int x=68; x<128; x++)
    {
      ssd1306_DrawPixel(x, 13, White);
      ssd1306_DrawPixel(x, 22, White);
    }
    for(int x=68; x<68+bar1; x++)
      for(int y=14; y<22; y++)
        ssd1306_DrawPixel(x, y, White);

    // S2
    ssd1306_SetCursor(0, 25);
    sprintf(msg, "S2:%3dcm", d2);
    ssd1306_WriteString(msg, Font_7x10, White);
    uint8_t bar2 = (d2 >= 50) ? 60 : (d2*60)/50;
    for(int x=68; x<128; x++)
    {
      ssd1306_DrawPixel(x, 24, White);
      ssd1306_DrawPixel(x, 33, White);
    }
    for(int x=68; x<68+bar2; x++)
      for(int y=25; y<33; y++)
        ssd1306_DrawPixel(x, y, White);

    // S3
    ssd1306_SetCursor(0, 36);
    sprintf(msg, "S3:%3dcm", d3);
    ssd1306_WriteString(msg, Font_7x10, White);
    uint8_t bar3 = (d3 >= 50) ? 60 : (d3*60)/50;
    for(int x=68; x<128; x++)
    {
      ssd1306_DrawPixel(x, 35, White);
      ssd1306_DrawPixel(x, 44, White);
    }
    for(int x=68; x<68+bar3; x++)
      for(int y=36; y<44; y++)
        ssd1306_DrawPixel(x, y, White);

    // Line
    for(int x=0; x<128; x++)
      ssd1306_DrawPixel(x, 52, White);

    // Status -😄
    ssd1306_SetCursor(0, 54);
    if(min_dist <= 5)
      ssd1306_WriteString(" RUK JAO MAALIK !! ", Font_7x10, White);
    else if(min_dist <= 10)
      ssd1306_WriteString(" DHEERE CHALO ", Font_7x10, White);
    else
      ssd1306_WriteString("  BINDAAS CHALO ! ", Font_7x10, White);

    ssd1306_UpdateScreen();

    HAL_Delay(100);
  }
  /* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  // Sensor 1 - TIM2 (32-bit)
  if(htim->Instance == TIM2)
  {
    if(!cap1)
    {
      IC1_1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      cap1 = 1;
      __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1,
                                     TIM_INPUTCHANNELPOLARITY_FALLING);
    }
    else
    {
      IC1_2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      uint32_t diff1;
      if(IC1_2 > IC1_1)
        diff1 = IC1_2 - IC1_1;
      else
        diff1 = (0xFFFFFFFF - IC1_1) + IC1_2;
      dist1 = diff1 * 0.034f / 2.0f;
      cap1 = 0;
      __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1,
                                     TIM_INPUTCHANNELPOLARITY_RISING);
    }
  }

  // Sensor 2 - TIM3 (16-bit) - Overflow Fix!
  if(htim->Instance == TIM3)
  {
    if(!cap2)
    {
      IC2_1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      cap2 = 1;
      __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1,
                                     TIM_INPUTCHANNELPOLARITY_FALLING);
    }
    else
    {
      IC2_2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      uint32_t diff2;
      if(IC2_2 > IC2_1)
        diff2 = IC2_2 - IC2_1;
      else
        diff2 = (65535 - IC2_1) + IC2_2; // Overflow fix
      dist2 = diff2 * 0.034f / 2.0f;
      cap2 = 0;
      __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1,
                                     TIM_INPUTCHANNELPOLARITY_RISING);
    }
  }

  // Sensor 3 - TIM4 (16-bit) - Overflow Fix!
  if(htim->Instance == TIM4)
  {
    if(!cap3)
    {
      IC3_1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      cap3 = 1;
      __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1,
                                     TIM_INPUTCHANNELPOLARITY_FALLING);
    }
    else
    {
      IC3_2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      uint32_t diff3;
      if(IC3_2 > IC3_1)
        diff3 = IC3_2 - IC3_1;
      else
        diff3 = (65535 - IC3_1) + IC3_2; // Overflow fix
      dist3 = diff3 * 0.034f / 2.0f;
      cap3 = 0;
      __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1,
                                     TIM_INPUTCHANNELPOLARITY_RISING);
    }
  }
}
/* USER CODE END 4 */

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    Error_Handler();
}

static void MX_CAN1_Init(void)
{
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 6;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_11TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if(HAL_CAN_Init(&hcan1) != HAL_OK)
    Error_Handler();
}

static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if(HAL_I2C_Init(&hi2c1) != HAL_OK)
    Error_Handler();
}

static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 84-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if(HAL_TIM_Base_Init(&htim2) != HAL_OK) Error_Handler();
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if(HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    Error_Handler();
  if(HAL_TIM_IC_Init(&htim2) != HAL_OK) Error_Handler();
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if(HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    Error_Handler();
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if(HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    Error_Handler();
}

static void MX_TIM3_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 84-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if(HAL_TIM_Base_Init(&htim3) != HAL_OK) Error_Handler();
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if(HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
    Error_Handler();
  if(HAL_TIM_IC_Init(&htim3) != HAL_OK) Error_Handler();
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if(HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    Error_Handler();
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if(HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    Error_Handler();
}

static void MX_TIM4_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 84-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if(HAL_TIM_Base_Init(&htim4) != HAL_OK) Error_Handler();
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if(HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
    Error_Handler();
  if(HAL_TIM_IC_Init(&htim4) != HAL_OK) Error_Handler();
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if(HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
    Error_Handler();
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if(HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    Error_Handler();
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if(HAL_UART_Init(&huart2) != HAL_OK)
    Error_Handler();
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5|GPIO_PIN_6|
                    GPIO_PIN_7|GPIO_PIN_8, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|
                    GPIO_PIN_2, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|
                        GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while(1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif
