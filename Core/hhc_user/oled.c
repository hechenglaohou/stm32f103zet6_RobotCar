#include "oled.h"
#include "i2c.h"
#include "main.h"
#include <string.h>
#include "sys.h"

#define OLED_ADDR    0x3C  // 7位地址

// ===================== 配置你的I2C句柄（和MPU6050保持同一个！） =====================
#define OLED_I2C_HANDLER    &hi2c1
// ==================================================================================

// 写命令
void OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd}; // 0x00 = 命令标志
    HAL_I2C_Master_Transmit(OLED_I2C_HANDLER, OLED_ADDR << 1, buf, 2, 100);
    delay_us(10);
}

// 写数据
void OLED_WriteData(uint8_t data)
{
    uint8_t buf[2] = {0x40, data}; // 0x40 = 数据标志
    HAL_I2C_Master_Transmit(OLED_I2C_HANDLER, OLED_ADDR << 1, buf, 2, 100);
    delay_us(10);
}

// OLED 初始化
void OLED_Init(void)
{
    uint8_t i, j;
    osDelay(100);

    OLED_WriteCmd(0xAE);
    OLED_WriteCmd(0xD5); OLED_WriteCmd(0x80);
    OLED_WriteCmd(0xA8); OLED_WriteCmd(0x3F);
    OLED_WriteCmd(0xD3); OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0x8D); OLED_WriteCmd(0x14);
    OLED_WriteCmd(0x20); OLED_WriteCmd(0x00);
    OLED_WriteCmd(0xA1);
    OLED_WriteCmd(0xC8);
    OLED_WriteCmd(0xDA); OLED_WriteCmd(0x12);
    OLED_WriteCmd(0x81); OLED_WriteCmd(0xCF);
    OLED_WriteCmd(0xD9); OLED_WriteCmd(0xF1);
    OLED_WriteCmd(0xDB); OLED_WriteCmd(0x30);
    OLED_WriteCmd(0xA4);
    OLED_WriteCmd(0xA6);
    OLED_WriteCmd(0xAF);

    // 清屏
    for(i=0; i<8; i++){
        OLED_WriteCmd(0xB0 + i);
        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0x10);
        for(j=0; j<128; j++){
            OLED_WriteData(0x00);
        }
    }
}

// 显示块
void OLED_ShowBlock(uint8_t x, uint8_t y)
{
    uint8_t i;
    OLED_WriteCmd(0xB0 + y);
    OLED_WriteCmd(0x00 + (x & 0x0F));
    OLED_WriteCmd(0x10 + (x >> 4));
    for(i=0; i<8; i++){
        OLED_WriteData(0xFF);
    }
}
