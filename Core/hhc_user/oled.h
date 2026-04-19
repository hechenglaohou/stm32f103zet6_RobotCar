#ifndef __OLED_H
#define __OLED_H

#include "main.h"  // HAL 库标准头文件，包含 uint8_t 等类型

void OLED_Init(void);
void OLED_WriteCmd(uint8_t cmd);
void OLED_WriteData(uint8_t data);
void OLED_ShowBlock(uint8_t x, uint8_t y);

#endif
