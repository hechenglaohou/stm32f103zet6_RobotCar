#ifndef __PCA9685_H
#define __PCA9685_H

#include "stdint.h"

// 新增：定义SG90相关宏（修复未定义问题）
#define SG90_CHANNEL 0       // 舵机占用PCA9685的0通道
#define SG90_MAX_ANGLE 180   // SG90最大角度
#define SG90_MIN_ANGLE 0     // SG90最小角度
#define SG90_PWM_FREQ 50     // 舵机PWM频率50Hz

// 声明I2C底层函数（修复隐式声明警告）
void I2C_Start(void);
void I2C_SendByte(uint8_t byte);
uint8_t I2C_WaitAck(void);
void I2C_Stop(void);
void I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data);

// PCA9685函数声明
void PCA9685_Init(uint8_t addr);
void SG90_SetAngle(uint8_t ch, uint8_t angle);
void SG90_Channel0_Init(void);

#endif

