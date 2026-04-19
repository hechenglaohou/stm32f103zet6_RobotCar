#include "encoder.h"
#include "tim.h"  // HAL 库必须包含（CubeMX 生成）

// 定义使用的定时器句柄（HAL 库用句柄，不是直接操作寄存器）
#define ENCODER_LEFT_TIM     htim2
#define ENCODER_RIGHT_TIM    htim3

volatile int32_t encoder_left = 0;
volatile int32_t encoder_right = 0;

// 初始化：因为 CubeMX 已经配置好了，这里只需要启动编码器 + 清零
void Encoder_Init(void)
{
    // HAL 库已经完成 GPIO / 时钟 / 编码器模式配置
    // 只需要启动编码器
    HAL_TIM_Encoder_Start(&ENCODER_LEFT_TIM, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&ENCODER_RIGHT_TIM, TIM_CHANNEL_ALL);

    // 清零计数值
    __HAL_TIM_SET_COUNTER(&ENCODER_LEFT_TIM, 0);
    __HAL_TIM_SET_COUNTER(&ENCODER_RIGHT_TIM, 0);

    encoder_left = 0;
    encoder_right = 0;
}

// 更新编码器值（核心逻辑不变，改成 HAL 写法）
void Encoder_Update(void)
{
    int16_t p;

    p = (int16_t)__HAL_TIM_GET_COUNTER(&ENCODER_LEFT_TIM);
    encoder_left += p;
    __HAL_TIM_SET_COUNTER(&ENCODER_LEFT_TIM, 0);

    p = (int16_t)__HAL_TIM_GET_COUNTER(&ENCODER_RIGHT_TIM);
    encoder_right += p;
    __HAL_TIM_SET_COUNTER(&ENCODER_RIGHT_TIM, 0);
}

// 以下完全保持你 main.c 调用的函数名不变
int32_t Encoder_GetLeft(void)  { return encoder_left; }
int32_t Encoder_GetRight(void) { return encoder_right; }
float Encoder_GetSpeedLeft(void)  { return 0; }
float Encoder_GetSpeedRight(void) { return 0; }

void Encoder_Clear(void)
{
    encoder_left = 0;
    encoder_right = 0;
    __HAL_TIM_SET_COUNTER(&ENCODER_LEFT_TIM, 0);
    __HAL_TIM_SET_COUNTER(&ENCODER_RIGHT_TIM, 0);
}

//#include "encoder.h"
//

//// 左编码器：PA0 PA1 → TIM2
//// 右编码器：PA2 PA3 → TIM4

//#define ENCODER_LEFT_TIM     TIM2
//#define ENCODER_RIGHT_TIM    TIM4

//volatile int32_t encoder_left = 0;
//volatile int32_t encoder_right = 0;

//void Encoder_Init(void)
//{
//    GPIO_InitTypeDef GPIO_InitS;
//    TIM_TimeBaseInitTypeDef TIM_InitS;

//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM4, ENABLE);

//    // PA0~3 上拉输入
//    GPIO_InitS.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
//    GPIO_InitS.GPIO_Mode = GPIO_Mode_IPU;
//    GPIO_Init(GPIOA, &GPIO_InitS);

//    // TIM2 编码器
//    TIM_TimeBaseStructInit(&TIM_InitS);
//    TIM_InitS.TIM_Period = 0xFFFF;
//    TIM_TimeBaseInit(TIM2, &TIM_InitS);
//    TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
//    TIM_SetCounter(TIM2, 0);
//    TIM_Cmd(TIM2, ENABLE);

//    // TIM4 编码器
//    TIM_TimeBaseInit(TIM4, &TIM_InitS);
//    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
//    TIM_SetCounter(TIM4, 0);
//    TIM_Cmd(TIM4, ENABLE);

//    encoder_left = 0;
//    encoder_right = 0;
//}

//void Encoder_Update(void)
//{
//    int16_t p;

//    p = (int16_t)TIM_GetCounter(TIM2);
//    encoder_left += p;
//    TIM_SetCounter(TIM2, 0);

//    p = (int16_t)TIM_GetCounter(TIM4);
//    encoder_right += p;
//    TIM_SetCounter(TIM4, 0);
//}

//// 以下函数名 100% 匹配你 main.c 调用
//int32_t Encoder_GetLeft(void)  { return encoder_left; }
//int32_t Encoder_GetRight(void) { return encoder_right; }
//float Encoder_GetSpeedLeft(void) { return 0; }   // 临时占位，不报错
//float Encoder_GetSpeedRight(void) { return 0; } // 临时占位，不报错

//void Encoder_Clear(void)
//{
//    encoder_left=0;
//    encoder_right=0;
//    TIM_SetCounter(TIM2,0);
//    TIM_SetCounter(TIM4,0);
//}
