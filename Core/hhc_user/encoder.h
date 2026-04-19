#ifndef __ENCODER_H
#define __ENCODER_H
#include "main.h"  // HAL 库标准头文件，包含 uint8_t 等类型


void Encoder_Init(void);
void Encoder_Update(void);
int32_t Encoder_GetLeft(void);
int32_t Encoder_GetRight(void);
float Encoder_GetSpeedLeft(void);
float Encoder_GetSpeedRight(void);
void Encoder_Clear(void);

#endif
