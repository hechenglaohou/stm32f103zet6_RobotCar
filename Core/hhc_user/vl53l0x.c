#include "vl53l0x.h"
#include "i2c.h"   // HAL ?? I2C
#include "i2c_hhc.h"   // HAL ?? I2C
#include "main.h"
#include <stdint.h> // ???????????
#include "cmsis_os.h"  // ??????????????
#include <stdio.h>
#include "sys.h"
#define VL53L0X_ADDR	0x29
// XSHUT
#define XSHUT_PIN       GPIO_PIN_4
#define XSHUT_PORT      GPIOC
// ????IIC
#define IIC_SCL_PIN     GPIO_PIN_12
#define IIC_SCL_PORT    GPIOB
#define IIC_SDA_PIN     GPIO_PIN_13
#define IIC_SDA_PORT    GPIOB

// =============================== ???? IIC ===============================
void IIC_Start(void)
{
  HAL_GPIO_WritePin(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_SET);
  delay_us(1);
  HAL_GPIO_WritePin(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_PIN_RESET);
  delay_us(1);
  HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_RESET);
}

void IIC_Stop(void)
{
  HAL_GPIO_WritePin(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_SET);
  delay_us(1);
  HAL_GPIO_WritePin(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_PIN_SET);
  delay_us(1);
}

// 移除应答检查的IIC写字节函数
void IIC_WriteByte(uint8_t data)
{
    uint8_t i;
    for(i=0;i<8;i++)
    {
        HAL_GPIO_WritePin(IIC_SDA_PORT, IIC_SDA_PIN, (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        data <<= 1;
        delay_us(2);  // ????????????VL53L0X
        HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_SET);
        delay_us(2);
        HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_RESET);
        delay_us(1);
    }
    // 跳过应答检查，直接释放SDA
    HAL_GPIO_WritePin(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_PIN_SET);
    delay_us(2);
    HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_SET);
    delay_us(2);
    HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_RESET);
    delay_us(1);
}

// 移除应答控制的IIC读字节函数
uint8_t IIC_ReadByte(void)
{
    uint8_t dat = 0;
    uint8_t i;

    HAL_GPIO_WritePin(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_PIN_SET);
    for(i=0;i<8;i++)
    {
        dat <<= 1;
        HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_SET);
        delay_us(2);
        if(HAL_GPIO_ReadPin(IIC_SDA_PORT, IIC_SDA_PIN)) dat |= 0x01;
        HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_RESET);
        delay_us(2);
    }
    // 固定发送非应答（不再区分ack参数）
    HAL_GPIO_WritePin(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_PIN_SET);
    delay_us(2);
    HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_SET);
    delay_us(2);
    HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_PIN_SET);
    return dat;
}

// 移除应答检查的VL53L0X写函数
void VL_Write(uint8_t reg, uint8_t val)
{
    IIC_Start();
    IIC_WriteByte(VL53L0X_ADDR << 1); // 不检查应答
    IIC_WriteByte(reg);
    IIC_WriteByte(val);
    IIC_Stop();
    delay_us(10); // ???д???????
}

// 移除应答检查的VL53L0X读函数
// 替换原VL_Read函数（vl53l0x.c 第86-99行）
uint8_t VL_Read(uint8_t reg, uint8_t *dat)  // 新增dat指针参数
{
    if(dat == NULL) return 1; // 空指针保护
    
    IIC_Start();
    IIC_WriteByte(VL53L0X_ADDR << 1); // 写地址
    IIC_WriteByte(reg);                // 写寄存器地址
    
    IIC_Start();
    IIC_WriteByte((VL53L0X_ADDR << 1) | 1); // 读地址
    *dat = IIC_ReadByte(); // 将读取值存入指针指向的变量
    IIC_Stop();
    return 0; // 读取成功返回0
}

// =============================== VL53L0X ????? ===============================

void VL53L0X_Init(void)
{
    uint8_t temp;

    // 1. 硬件复位（保留原有逻辑）
    HAL_GPIO_WritePin(XSHUT_PORT, XSHUT_PIN, GPIO_PIN_RESET);
    HAL_Delay(100); 
    HAL_GPIO_WritePin(XSHUT_PORT, XSHUT_PIN, GPIO_PIN_SET);
    HAL_Delay(500); // 延长复位延时，确保传感器启动

    // 2. 读取设备ID（验证通信）
    if(VL_Read(0x00, &temp)) // 现在参数数量匹配
    {
        printf("VL53L0X Comm Error!\r\n");
        return;
    }
    if(temp != 0xEE)
    {
        printf("VL53L0X ID Error! Read=0x%02X\r\n", temp);
        return;
    }
    printf("VL53L0X ID OK: 0x%02X\r\n", temp);

    // 3. 原有初始化逻辑（保留）
    VL_Write(0x80, 0x01);
    VL_Write(0xFF, 0x01);
    VL_Write(0x00, 0x00);
    VL_Write(0xFF, 0x00);
    VL_Write(0x80, 0x00);

    VL_Write(0x01, 0x02); 
    VL_Write(0x02, 0x00); 
    VL_Write(0x07, 0x01); 
    HAL_Delay(100); 
    
    printf("VL53L0X Init Finish!\r\n");
}

// =============================== ??????? ===============================
uint16_t VL53L0X_ReadDistance(void)
{
    uint8_t status, h, l;
    uint16_t distance = 0;

    // 1. 读取状态寄存器（0x00），确认数据就绪
    if(VL_Read(0x00, &status)) return 0;
    if((status & 0x07) != 0x04) // 0x04表示测距数据就绪
    {
        return 0;
    }

    // 2. 读取距离值（低字节0x13，高字节0x12）
    if(VL_Read(0x13, &l)) return 0;
    if(VL_Read(0x12, &h)) return 0;
    distance = (h << 8) | l;

    // 3. 数据过滤（有效范围20~2000mm）
    if(distance < 20 || distance > 2000)
    {
        return 0;
    }
    return distance;
}
