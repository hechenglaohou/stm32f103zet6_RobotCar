#include "motor.h"
#include "tim.h"  // HAL 定时器句柄 htim3
#include "main.h"
#include "gpio.h"

// ==========================
// TIM3 通道（与原来完全一致）
// PA6 = CH1
// PA7 = CH2
// PB0 = CH3
// PB1 = CH4
// ==========================

// 电机速度范围 -100 ~ 100
static int16_t motor_left = 0;
static int16_t motor_right = 0;

// DRV8833 唤醒引脚 PB12
static void DRV8833_Init(void)
{
  // PB12 已经在 CubeMX 配置为推挽输出，这里直接置 1
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

// 电机初始化（HAL 版本）
void Motor_Init(void)
{
  DRV8833_Init();

  // 启动 TIM3 所有 4 路 PWM 输出
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

  Motor_Stop();
}

// ------------------------------
// 左电机：PA6=CH1, PA7=CH2
// ------------------------------
static void Motor_SetLeft(int16_t speed)
{
  uint16_t pwm = (speed < 0) ? -speed : speed;
  if (pwm > 100) pwm = 100;

  if (speed > 0)
  {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwm);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
  }
  else if (speed < 0)
  {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pwm);
  }
  else
  {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
  }
  motor_left = speed;
}

// ------------------------------
// 右电机：PB0=CH3, PB1=CH4
// ------------------------------
static void Motor_SetRight(int16_t speed)
{
  uint16_t pwm = (speed < 0) ? -speed : speed;
  if (pwm > 100) pwm = 100;

  if (speed > 0)
  {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pwm);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
  }
  else if (speed < 0)
  {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, pwm);
  }
  else
  {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
  }
  motor_right = speed;
}

// 设置左右电机速度
void Motor_SetSpeed(int16_t left, int16_t right)
{
  Motor_SetLeft(left);
  Motor_SetRight(right);
}

// 停止电机
void Motor_Stop(void)
{
  Motor_SetSpeed(0, 0);
}

// 获取当前速度
void Motor_GetSpeed(int16_t *left, int16_t *right)
{
  if (left)  *left  = motor_left;
  if (right) *right = motor_right;
}
