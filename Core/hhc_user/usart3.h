#ifndef __USART3_H
#define __USART3_H

#include "main.h"

extern uint8_t USART3_RX_BUF[128];
extern uint32_t USART3_RX_LEN;
extern uint8_t USART3_RX_FLAG;

//void USART3_Init(void);
void USART3_SendByte(uint8_t data);
void USART3_SendString(uint8_t* str);
void USART3_ClearBuf(void);
void USART3_CheckRx(void);
void MX_USART3_UART_Init(void);

#endif

