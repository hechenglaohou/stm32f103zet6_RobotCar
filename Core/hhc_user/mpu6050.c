#include "mpu6050.h"
#include "i2c.h"  // STM32Cube 自动生成的 hi2c1
#include "main.h"
#include <math.h>

// 全局变量：偏航角 + Z轴陀螺仪
float yaw = 0.0f;
float gyro_z = 0.0f;

// MPU6050 I2C 地址（HAL 库使用 7 位地址）
#define MPU6050_ADDR    0x68

// MPU6050 寄存器定义
#define PWR_MGMT_1      0x6B
#define GYRO_CONFIG     0x1B
#define GYRO_Z_H        0x47
#define WHO_AM_I        0x75

/**
  * @brief  MPU6050 写单个寄存器
  * @param  reg: 寄存器地址
  * @param  data: 要写入的数据
  */
static void MPU6050_WriteReg(uint8_t reg, uint8_t data)
{
    // HAL_I2C 内存写入函数 (7位地址)
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, reg,
                      I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    HAL_Delay(1);
}

/**
  * @brief  MPU6050 读单个寄存器
  * @param  reg: 寄存器地址
  * @retval 读到的数据
  */
static uint8_t MPU6050_ReadReg(uint8_t reg)
{
    uint8_t data = 0;
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR << 1, reg,
                     I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    return data;
}

/**
  * @brief  MPU6050 初始化
  */
void MPU6050_Init(void)
{
    uint8_t dev_id;
    HAL_Delay(100);

    // 读取设备ID校验
    dev_id = MPU6050_ReadReg(WHO_AM_I);
    if(dev_id != 0x68)
    {
        // 通信失败，可在这里加错误提示
        Error_Handler();
    }

    // 唤醒 MPU6050
    MPU6050_WriteReg(PWR_MGMT_1, 0x00);
    HAL_Delay(10);

    // 陀螺仪量程 ±250°/s
    MPU6050_WriteReg(GYRO_CONFIG, 0x00);
    HAL_Delay(1);

    // 角度清零
    yaw = 0.0f;
    gyro_z = 0.0f;
}

/**
  * @brief  读取Z轴陀螺仪 + 积分计算偏航角 yaw
  * @note   建议 10ms 调用一次
  */
void MPU6050_Read(void)
{
    uint8_t buf[2];
    int16_t gyro_z_raw;

    // 连续读取 Z 轴高低字节
    buf[0] = MPU6050_ReadReg(GYRO_Z_H);
    buf[1] = MPU6050_ReadReg(GYRO_Z_H + 1);

    // 合成 16 位原始数据
    gyro_z_raw = (int16_t)(buf[0] << 8 | buf[1]);

    // 转换为角速度 (°/s)
    gyro_z = (float)gyro_z_raw / 131.0f;

    // 零漂过滤
    if(fabs(gyro_z) < 0.5f)
    {
        gyro_z = 0.0f;
    }

    // 积分计算偏航角（10ms = 0.01s）
    yaw += gyro_z * 0.01f;

    // 角度归一化 -180 ~ 180°
    if(yaw > 180.0f)  yaw -= 360.0f;
    if(yaw < -180.0f) yaw += 360.0f;
}

/**
  * @brief  获取当前偏航角
  */
float MPU6050_GetYaw(void)
{
    return yaw;
}
