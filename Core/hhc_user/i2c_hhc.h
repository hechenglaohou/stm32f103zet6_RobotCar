#ifndef __I2C_H
#define __I2C_H
#include "main.h" 


void I2C1_Init(void);
uint8_t I2C_WaitEvent(uint32_t event);
uint8_t I2C_ScanDevice(uint8_t addr);
void I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data);
void I2C_InitMutex(void);
void I2C_WriteRegs_4(uint8_t addr, uint8_t reg, uint16_t on, uint16_t off);

#endif




