/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "stdio.h"
#include "main.h"
#include "cmsis_os.h"

/* USER CODE BEGIN 0 */

#define EN_USART1_RX 1
#define USART_REC_LEN 200

uint8_t USART_RX_BUF[USART_REC_LEN];
uint16_t USART_RX_STA = 0;

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* USART1 init function */
void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

  /* 使能接收中断 —— 完全对应标准库 uart_init 逻辑 */
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* NVIC 配置 —— 完全对应标准库 uart_init 逻辑 */
    HAL_NVIC_SetPriority(USART1_IRQn, 3, 3);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  }
  else if(uartHandle->Instance==USART3)
  {
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
  if(uartHandle->Instance==USART1)
  {
    __HAL_RCC_USART1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  }
  else if(uartHandle->Instance==USART3)
  {
    __HAL_RCC_USART3_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_11);
  }
}

/* USER CODE BEGIN 1 */

void _sys_exit(int x) {x = x; }

int fputc(int ch, FILE *f)
{
    while((USART1->SR & UART_FLAG_TXE) == 0);
    USART1->DR = (uint8_t)ch;
    return ch;
}

void USART1_IRQHandler(void)
{
  uint8_t Res;

  if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET)
  {
    Res = (uint8_t)(USART1->DR & 0x00FF);

    if((USART_RX_STA&0x8000)==0)
    {
      if(USART_RX_STA&0x4000)
      {
        if(Res!=0x0a)USART_RX_STA=0;
        else USART_RX_STA|=0x8000;
      }
      else
      {
        if(Res==0x0d)USART_RX_STA|=0x4000;
        else
        {
          USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
          USART_RX_STA++;
          if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;
        }
      }
    }
  }

  HAL_UART_IRQHandler(&huart1);
}

/* USER CODE END 1 */
