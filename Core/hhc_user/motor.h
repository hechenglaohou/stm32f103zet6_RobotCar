#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

void Motor_Init(void);
void Motor_SetSpeed(int16_t left, int16_t right);
void Motor_Stop(void);
void Motor_GetSpeed(int16_t *left, int16_t *right);

#endif
