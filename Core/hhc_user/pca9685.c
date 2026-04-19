#include "pca9685.h"
#include "i2c_hhc.h"
#include "cmsis_os.h"
#include <stdio.h>

static uint8_t pca9685_addr; // PCA9685设备地址（私有化）

// PCA9685初始化（配置PWM频率/模式）
void PCA9685_Init(uint8_t addr)
{
    pca9685_addr = addr;

    // 1. 软复位PCA9685
    I2C_WriteReg(addr, 0x00, 0x80); // MODE1: RESET=1
    osDelay(10);

    // 2. 进入休眠模式（修改频率前必须休眠）
    I2C_WriteReg(addr, 0x00, 0x10); // MODE1: SLEEP=1
    osDelay(1);

    // 3. 计算并设置PWM分频值（50Hz）
    uint8_t prescale_val = (uint8_t)(25000000.0f / (4096 * SG90_PWM_FREQ)) - 1;
    I2C_WriteReg(addr, 0xFE, prescale_val); // PRE_SCALE寄存器
    osDelay(1);

    // 4. 退出休眠+开启自动增量模式
    I2C_WriteReg(addr, 0x00, 0xA0); // MODE1: SLEEP=0, AUTOINCR=1
    osDelay(1);

    // 5. 设置输出模式（推挽+响应ACK）
    I2C_WriteReg(addr, 0x01, 0x04); // MODE2: OUTDRV=1
    osDelay(1);

    printf("PCA9685 Init OK (addr=0x%02X)\r\n", addr);
}

// SG90舵机角度设置（0~180°）
void SG90_SetAngle(uint8_t ch, uint8_t angle)
{
    // 角度边界保护
    if(angle > SG90_MAX_ANGLE) angle = SG90_MAX_ANGLE;
    if(angle < SG90_MIN_ANGLE) angle = SG90_MIN_ANGLE;

    // SG90 PWM换算：0°=1ms, 180°=2ms, 周期20ms
    uint16_t us = 1000 + (angle * 1000) / 180;  // 脉宽（微秒）
    uint16_t pwm = (us * 4096) / 20000;         // 转换为12位PWM值

    // 写入PWM寄存器（ON=0，OFF=pwm）
    uint8_t reg = 6 + 4*ch; // 每个通道占用4个寄存器（0x06~0x09对应通道0）
    I2C_WriteRegs_4(pca9685_addr, reg, 0, pwm);
}

// 舵机通道0初始化（归位到0°）
void SG90_Channel0_Init(void)
{
    SG90_SetAngle(SG90_CHANNEL, 0);
    osDelay(500); // 等待舵机稳定
    printf("SG90 Channel0 Init OK\r\n");
}

