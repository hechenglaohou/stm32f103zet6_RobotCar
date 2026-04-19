/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances (适配电机驱动+舵机控制).
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "tim.h"
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN 0 */
// ==================== 宏定义（对齐标准库） ====================
// 电机TIM3配置（和标准库一致：72MHz/(71+1)/100 = 1kHz PWM）
#define MOTOR_PWM_PERIOD    99    // 周期 = 99 (0~99，对应100级PWM)
#define MOTOR_PWM_PRESCALER 71    // 分频 = 71 (72MHz/(71+1)=1MHz)

// 舵机TIM1配置（50Hz PWM = 20ms周期）
#define SERVO_PWM_PERIOD    19999 // 20ms周期 (1MHz计数: 1*20000=20ms)
#define SERVO_PWM_PRESCALER 71    // 72MHz/(71+1)=1MHz
#define SERVO_MIN_PULSE     500   // 0.5ms (0°)
#define SERVO_MAX_PULSE     2500  // 2.5ms (180°)
#define SERVO_MAX_ANGLE     180   // 最大角度

// ==================== 全局变量 ====================
volatile uint32_t scan_tick = 0;    // 硬件计时（20ms累加）
volatile uint8_t  scan_running = 1; // 扫描运行状态
/* USER CODE END 0 */

TIM_HandleTypeDef htim1;  // 舵机控制+20ms中断
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;  // 电机驱动（4路PWM）

// ==================== TIM1 初始化（舵机50Hz PWM + 20ms中断） ====================
void MX_TIM1_Init(void)
{
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = SERVO_PWM_PRESCALER;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = SERVO_PWM_PERIOD;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

  // 1. 初始化时基
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  // 2. 初始化PWM输出（舵机CH1）
  TIM_OC_InitTypeDef sConfigOC = {0};
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = SERVO_MIN_PULSE; // 默认0°
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  // 3. 开启预装载 + 启动PWM/时基
  HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); // 启动PWM输出
  HAL_TIM_Base_Start_IT(&htim1);            // 启动时基+中断
}

// ==================== TIM3 初始化（电机4路PWM，对齐标准库） ====================
void MX_TIM3_Init(void)
{
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = MOTOR_PWM_PRESCALER;  // 对齐标准库71
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = MOTOR_PWM_PERIOD;        // 对齐标准库99
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; // 开启ARR预装载

  // 1. 初始化PWM时基
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }

  // 2. 配置4路PWM通道（CH1~CH4）
  TIM_OC_InitTypeDef sConfigOC = {0};
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0; // 默认占空比0
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

  // 配置CH1(PA6)/CH2(PA7)/CH3(PB0)/CH4(PB1)
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) Error_Handler();
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) Error_Handler();
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK) Error_Handler();

  // 3. 开启OC预装载（对齐标准库）
//  HAL_TIMEx_EnableOCxPreload(&htim3, TIM_CHANNEL_1);
//  HAL_TIMEx_EnableOCxPreload(&htim3, TIM_CHANNEL_2);
//  HAL_TIMEx_EnableOCxPreload(&htim3, TIM_CHANNEL_3);
//  HAL_TIMEx_EnableOCxPreload(&htim3, TIM_CHANNEL_4);
	
		// 配置 CH1~CH4
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) Error_Handler();
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) Error_Handler();
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK) Error_Handler();



  // 4. 启动4路PWM输出
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
}

// ==================== GPIO/中断底层配置 ====================
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim_base)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (htim_base->Instance == TIM1)
  {
    // TIM1时钟使能
    __HAL_RCC_TIM1_CLK_ENABLE();
    // TIM1_CH1(PA8) GPIO配置（舵机引脚）
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 高速（对齐标准库）
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 中断配置（降低优先级，兼容FreeRTOS）
    HAL_NVIC_SetPriority(TIM1_UP_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
  }
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* tim_pwmHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(tim_pwmHandle->Instance==TIM3)
  {
    // TIM3时钟使能
    __HAL_RCC_TIM3_CLK_ENABLE();
    // GPIO时钟使能（PA+PB）
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // PA6(CH1)/PA7(CH2) 配置
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 50MHz高速（对齐标准库）
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PB0(CH3)/PB1(CH4) 配置
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
  else if(tim_pwmHandle->Instance==TIM2)
  {
    __HAL_RCC_TIM2_CLK_ENABLE();
  }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{
  // 兼容原有代码，无额外配置
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* tim_pwmHandle)
{
  if(tim_pwmHandle->Instance==TIM2)
  {
    __HAL_RCC_TIM2_CLK_DISABLE();
  }
  else if(tim_pwmHandle->Instance==TIM3)
  {
    __HAL_RCC_TIM3_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6|GPIO_PIN_7);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0|GPIO_PIN_1);
  }
}

// ==================== 舵机控制函数（增加边界检查） ====================
void SERVO_SetAngle(uint16_t angle)
{
  // 边界检查，防止角度超限
  if(angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;
  // 计算脉冲值：0.5ms~2.5ms
  uint16_t pulse = SERVO_MIN_PULSE + (angle * (SERVO_MAX_PULSE - SERVO_MIN_PULSE) / SERVO_MAX_ANGLE);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse);
}

// ==================== TIM1中断处理（标准HAL库方式） ====================
void TIM1_UP_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim1);
}

// TIM1周期中断回调函数（替换原硬中断）
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//  if (htim->Instance == TIM1)
//  {
//    // FreeRTOS临界区保护（如果使用RTOS）
//    taskENTER_CRITICAL();
//    if(scan_running)
//    {
//      scan_tick++;  // 20ms自动+1
//    }
//    taskEXIT_CRITICAL();
//  }
//}

// ==================== 兼容原有TIM2初始化（无修改） ====================
void MX_TIM2_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim2);
}

/* USER CODE BEGIN 1 */
// 可选：添加电机控制HAL库封装（对齐标准库Motor_SetSpeed）
void Motor_SetSpeed_HAL(int16_t left, int16_t right)
{
  uint16_t pwm_left = (left<0) ? -left : left;
  uint16_t pwm_right = (right<0) ? -right : right;

  // 限幅0~100
  pwm_left = (pwm_left>100) ? 100 : pwm_left;
  pwm_right = (pwm_right>100) ? 100 : pwm_right;

  // 左电机：CH1(PA6)=正，CH2(PA7)=反
  if(left > 0) {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwm_left);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
  } else if(left < 0) {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pwm_left);
  } else {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
  }

  // 右电机：CH3(PB0)=正，CH4(PB1)=反
  if(right > 0) {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pwm_right);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
  } else if(right < 0) {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, pwm_right);
  } else {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
  }
}
/* USER CODE END 1 */