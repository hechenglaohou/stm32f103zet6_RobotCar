#ifndef __VL53L0X_H
#define __VL53L0X_H

#include "main.h" 

//uint8_t VL53L0X_Init(void);
void VL53L0X_Init(void);
float VL53L0X_Read(void);
uint16_t VL53L0X_ReadDistance(void);
#endif
