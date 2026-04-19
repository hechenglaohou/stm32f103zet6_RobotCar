///* USER CODE BEGIN Header */
///**
//  ******************************************************************************
//  * @file    usart.h
//  * @brief   This file contains all the function prototypes for
//  *          the usart.c file
//  ******************************************************************************
//  * @attention
//  *
//  * Copyright (c) 2026 STMicroelectronics.
//  * All rights reserved.
//  *
//  * This software is licensed under terms that can be found in the LICENSE file
//  * in the root directory of this software component.
//  * If no LICENSE file comes with this software, it is provided AS-IS.
//  *
//  ******************************************************************************
//  */
///* USER CODE END Header */
///* Define to prevent recursive inclusion -------------------------------------*/
//#ifndef __USART_H__
//#define __USART_H__

//#ifdef __cplusplus
//extern "C" {
//#endif

///* Includes ------------------------------------------------------------------*/
//#include "main.h"
//#include <stdio.h>
///* USER CODE BEGIN Includes */

///* USER CODE END Includes */

//extern UART_HandleTypeDef huart1;

//extern UART_HandleTypeDef huart3;

///* USER CODE BEGIN Private defines */

///* USER CODE END Private defines */

//void MX_USART1_UART_Init(void);


///* USER CODE BEGIN Prototypes */
//void _sys_exit(int x);
//int fputc(int ch, FILE *f);
///* USER CODE END Prototypes */

//#ifdef __cplusplus
//}
//#endif

//#endif /* __USART_H__ */




#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 

//STM32F1核心板
//串口1初始化		   

//********************************************************************************

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
	  	
extern uint8_t  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern uint16_t USART_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义
void MX_USART1_UART_Init(void);
#endif
