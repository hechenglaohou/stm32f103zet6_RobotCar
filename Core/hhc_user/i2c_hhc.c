#include "i2c_hhc.h"
#include "i2c.h"
#include "gpio.h"
#include <stdint.h>
#include <stdio.h>

// FreeRTOS I2C总线互斥信号量
#include "FreeRTOS.h"
#include "semphr.h"

SemaphoreHandle_t xI2CMutex = NULL;
#define I2C_TIMEOUT    30000

// 初始化I2C互斥锁（在main中优先调用）
void I2C_InitMutex(void)
{
    if(xI2CMutex == NULL)
        xI2CMutex = xSemaphoreCreateMutex();
}

// 申请I2C总线锁
static uint8_t I2C_Lock(void)
{
    if(xI2CMutex == NULL) return 1;  // 未创建则直接允许
    if(xSemaphoreTake(xI2CMutex, pdMS_TO_TICKS(I2C_TIMEOUT)) == pdTRUE)
        return 1;
    return 0;
}

// 释放I2C总线锁
static void I2C_Unlock(void)
{
    if(xI2CMutex != NULL)
        xSemaphoreGive(xI2CMutex);
}

// I2C总线复位（通信失败时恢复）
void I2C_ResetBus(void)
{
    HAL_I2C_DeInit(&hi2c1);
    HAL_Delay(10);  // ? 改用 HAL 延时，不依赖 RTOS
    MX_I2C1_Init();
    HAL_Delay(10);
}

// 扫描I2C设备（返回1=在线，0=离线）
uint8_t I2C_ScanDevice(uint8_t addr)
{
    if(!I2C_Lock()) return 0;
    uint8_t ret = 0;
    if(HAL_I2C_IsDeviceReady(&hi2c1, addr<<1, 3, I2C_TIMEOUT) == HAL_OK)
        ret = 1;
    I2C_Unlock();
    return ret;
}

// 单字节写寄存器（通用接口）
void I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data)
{
    if(!I2C_Lock()) return;

    uint8_t buf[2] = {reg, data};
    if(HAL_I2C_Master_Transmit(&hi2c1, addr<<1, buf, 2, I2C_TIMEOUT) != HAL_OK)
    {
        //I2C_ResetBus();
        printf("I2C_WriteReg failed: addr=0x%02X reg=0x%02X\r\n", addr, reg);
    }

    I2C_Unlock();

    // ? 只有调度器运行时才使用 vTaskDelay
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
        vTaskDelay(1);
}

// 4字节写寄存器（专用于PCA9685的PWM配置）
void I2C_WriteRegs_4(uint8_t addr, uint8_t reg, uint16_t on, uint16_t off)
{
    if(!I2C_Lock()) return;

    uint8_t buf[5] = {reg, (uint8_t)(on&0xFF), (uint8_t)(on>>8),
                      (uint8_t)(off&0xFF), (uint8_t)(off>>8)};

    if(HAL_I2C_Master_Transmit(&hi2c1, addr << 1, buf, 5, I2C_TIMEOUT) != HAL_OK)
    {
        //I2C_ResetBus();
        printf("I2C_WriteRegs_4 failed: addr=0x%02X reg=0x%02X\r\n", addr, reg);
    }

    I2C_Unlock();

    // ? 只有调度器运行时才使用 vTaskDelay
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
        vTaskDelay(1);
}