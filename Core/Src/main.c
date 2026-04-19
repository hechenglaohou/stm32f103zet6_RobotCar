/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body for STM32+FreeRTOS+Robot
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "i2c_hhc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
#include "usart3.h"
#include "esp8266.h"
#include "sys.h"
#include "motor.h"
#include "encoder.h"
#include "pid.h"
#include "mpu6050.h"
#include "vl53l0x.h"
#include "oled.h"
#include "pca9685.h"
#include "fast_ros.h"
#include "kinematics.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* 宏定义 */
#define M_PI                3.14159265f
#define PID_KP              625.0f
#define PID_KI              125.0f
#define PID_KD              0.0f
#define PID_OUT_LIMIT       100
#define ENCODER_PPR         1942        // 编码器每转脉冲数
#define WHEEL_DIAMETER      0.065f      // 车轮直径(m)
#define WHEEL_BASE          0.175f      // 轮距(m)
#define LASER_POINTS        120         // 激光扫描点数
#define MOTOR_MIN_PWM       20          // 电机最小PWM（防止堵转）
#define PCA9685_ADDR        0x40        // PCA9685默认I2C地址
#define SG90_MAX_ANGLE      180
#define SG90_CHANNEL        0

/* 全局变量 */
float laser_buf0[LASER_POINTS];         // 激光扫描缓存1
float laser_buf1[LASER_POINTS];         // 激光扫描缓存2
volatile uint8_t laser_ready = 0;       // 激光数据就绪标志
float laser_pub_buf[LASER_POINTS];      // 激光发布缓存

PID_TypeDef pid_left, pid_right;        // 左右轮PID控制器
float target_vx = 0, target_wz = 0;     // 目标线速度/角速度
float target_left = 0, target_right = 0;// 左右轮目标速度
int scan_angle = 0, scan_dir = 0;       // 扫描角度/方向
float odom_x=0, odom_y=0, yaw_angle=0;  // 里程计位姿
float pwmL=0, pwmR=0;                   // 电机PWM输出
float dist = 0;                         // 激光测距值
nav_msgs__Odometry odom_msg;            // ROS里程计消息
sensor_msgs__LaserScan scan_msg;        // ROS激光扫描消息

static int last_left_tick=0, last_right_tick=0; // 编码器上一次计数值

/* FreeRTOS任务句柄 */
osThreadId laserTaskHandle;
osThreadId motorPidTaskHandle;
osThreadId odomPubTaskHandle;
osThreadId scanPubTaskHandle;
osThreadId cmdRecvTaskHandle;
osThreadId sysInitTaskHandle;

/* FreeRTOS互斥锁 */
osMutexId laserBufMutexHandle;
osMutexId odomDataMutexHandle;
osMutexDef(laserBufMutex);
osMutexDef(odomDataMutex);


/* 函数声明 */
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
void laserTask(void const * argument);
void motorPidTask(void const * argument);
void odomPubTask(void const * argument);
void scanPubTask(void const * argument);
void cmdRecvTask(void const * argument);
void sysInitTask(void const * argument);
void laser_step(void);
void update_odom(void);

/* 激光扫描步进函数 */
void laser_step(void)
{
    // 获取激光测距值（转换为米）
    dist = VL53L0X_ReadDistance() / 1000.0f;
    printf("dist = %f mm\r\n", dist);
    // 互斥访问激光缓存
    osMutexWait(laserBufMutexHandle, osWaitForever);
    laser_buf0[scan_angle] =dist;// dist;

    // 角度映射：120点对应0~180°
    uint8_t servo_angle = (scan_angle * SG90_MAX_ANGLE) / (LASER_POINTS - 1);
    
    // 控制舵机扫描方向
    if(scan_dir == 0){ // 正向扫描（0→119）
        SG90_SetAngle(SG90_CHANNEL, servo_angle);
        scan_angle++;
        if(scan_angle >= LASER_POINTS){
            scan_angle = LASER_POINTS -1;
            scan_dir = 1;
            memcpy(laser_buf1, laser_buf0, sizeof(laser_buf0));
            laser_ready = 1; // 数据就绪
        }
    }else{ // 反向扫描（119→0）
        SG90_SetAngle(SG90_CHANNEL, servo_angle);
        scan_angle--;
        if(scan_angle <= 0){
            scan_angle = 0;
            scan_dir = 0;
            memcpy(laser_buf1, laser_buf0, sizeof(laser_buf0));
            laser_ready = 1; // 数据就绪
        }
    }
    osMutexRelease(laserBufMutexHandle);
}

/* 里程计更新函数 */
void update_odom(void)
{
    //osMutexWait(odomDataMutexHandle, osWaitForever);
    int left_tick = Encoder_GetLeft();
    int right_tick = Encoder_GetRight();
    
    // 运动学更新位姿
    Kinematic_Update(left_tick - last_left_tick, 
                     right_tick - last_right_tick, 
                     &odom_x, &odom_y, &yaw_angle);
    
    last_left_tick = left_tick;
    last_right_tick = right_tick;
    //osMutexRelease(odomDataMutexHandle);
}

/**
  * @brief  应用入口函数
  * @retval int
  */
int main(void)
{
  /* 硬件初始化 */
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();

  printf("\r\n===== SYSTEM INIT START =====\r\n");

  /* ESP8266初始化（无线通信） */
//  ESP8266_Init();
//  HAL_Delay(1500);
//	ESP8266_JoinAP();   HAL_Delay(1500);
//	ESP8266_PrintIP();  // 【关键】在透传前执行，此时还能响应AT指令
//	ESP8266_ConnectUDP(); HAL_Delay(1500);
//	ESP8266_OpenTransparent(); HAL_Delay(2500);

	printf("\r\n===== SYSTEM READY =====\r\n");

	  /* 硬件外设初始化 */
  Motor_Init();       printf("Motor Init OK\r\n");
  Encoder_Init();     printf("Encoder Init OK\r\n");
	//Motor_SetSpeed(90, 90);
	
  /* 初始化FreeRTOS并启动内核 */
  MX_FREERTOS_Init();
	printf("osKernelStart \r\n");
  osKernelStart();
	
  /* 防止程序跑飞 */
  while (1)
  {

      if(USART3_RX_FLAG)  // 调试：打印ESP8266接收数据
      {
          USART3_RX_FLAG = 0;
          printf("ESP8266 Recv: %s\r\n", USART3_RX_BUF);
          USART3_ClearBuf();
      }
      osDelay(100);
  }
}

/**
  * @brief  系统时钟配置
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* 系统初始化任务（完成后自销毁） */
void sysInitTask(void const * argument)
{
  printf("\r\n===== HARDWARE INIT START =====\r\n");
	  // ===================== 【修复 I2C 失败】=====================
  osDelay(5);  // 等待 TIM6 时基启动 → I2C 正常工作
  // ===========================================================
	
  // 第一步：创建互斥锁（内核已启动，可正常创建）
  laserBufMutexHandle = osMutexCreate(osMutex(laserBufMutex));
  odomDataMutexHandle = osMutexCreate(osMutex(odomDataMutex));
  I2C_InitMutex(); // I2C互斥锁也在此初始化

  /* 硬件外设初始化 */
//  Motor_Init();       printf("Motor Init OK\r\n");
//  Encoder_Init();     printf("Encoder Init OK\r\n");
  VL53L0X_Init();     printf("VL53L0X Init OK\r\n");
  OLED_Init();        printf("OLED Init OK\r\n");
  PCA9685_Init(PCA9685_ADDR); 
  SG90_Channel0_Init();
  fast_ros_init();    printf("Fast-ROS Init OK\r\n");

  /* 运动学+PID初始化 */
  Kinematic_Init(WHEEL_DIAMETER, WHEEL_BASE, ENCODER_PPR);
  PID_Init(&pid_left, PID_KP, PID_KI, PID_KD, -PID_OUT_LIMIT, PID_OUT_LIMIT);
  PID_Init(&pid_right, PID_KP, PID_KI, PID_KD, -PID_OUT_LIMIT, PID_OUT_LIMIT);

  /* ROS激光消息初始化 */
  scan_msg.angle_min = 0.0f;                  // 扫描起始角度（rad）
  scan_msg.angle_max = M_PI / 1.5f;           // 扫描终止角度（120°）
  scan_msg.angle_increment = scan_msg.angle_max / LASER_POINTS; // 角度步长
  scan_msg.range_min = 0.02f;                 // 最小测距（2cm）
  scan_msg.range_max = 2.0f;                  // 最大测距（2m）
  scan_msg.ranges = laser_pub_buf;
  scan_msg.size = LASER_POINTS;

  /* 初始速度（调试用） */
  target_left  = 0.0f;
  target_right = 0.0f;

  printf("\r\n===== ALL HARDWARE INIT OK =====\r\n");

  /* 创建业务任务 */
  osThreadDef(cmdRecvTask,    cmdRecvTask,    osPriorityNormal, 0, 256);
  cmdRecvTaskHandle = osThreadCreate(osThread(cmdRecvTask), NULL);

  osThreadDef(motorPidTask,   motorPidTask,   osPriorityLow,     0, 512);
  motorPidTaskHandle = osThreadCreate(osThread(motorPidTask), NULL);
  
  osThreadDef(odomPubTask,    odomPubTask,    osPriorityNormal,       0, 256);
  odomPubTaskHandle = osThreadCreate(osThread(odomPubTask), NULL);
  
  osThreadDef(scanPubTask,    scanPubTask,    osPriorityLow,       0, 512);
  scanPubTaskHandle = osThreadCreate(osThread(scanPubTask), NULL);
  
  osThreadDef(laserTask,      laserTask,      osPriorityHigh,         0, 512);
  laserTaskHandle = osThreadCreate(osThread(laserTask), NULL);

  printf("===== ALL TASKS CREATED =====\r\n");
  
  /* 销毁初始化任务 */
  osThreadTerminate(sysInitTaskHandle);
}

/* 激光扫描任务 */
void laserTask_old(void const * argument)
{
  for(;;)
  {
    laser_step();
    osDelay(15); // 扫描周期≈15ms/点 → 120点≈1.8s
  }
}

void laserTask(void const * argument)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(20);  // 20ms周期
  for(;;)
  {
    laser_step();
    // 2. 精确延时20ms，等待下一个脉冲周期
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}


/* 电机PID控制任务 */
void motorPidTask(void const * argument)
{
	// 初始化上次唤醒时间（只执行一次）
	TickType_t xLastWakeTime = xTaskGetTickCount();
	
	// 固定 50ms 周期 = 20Hz 发布
	const TickType_t xPeriod = pdMS_TO_TICKS(50);

	for(;;)
	{
		// 等待下一个周期（精准、不漂移）
		vTaskDelayUntil(&xLastWakeTime, xPeriod);

    // 获取左右轮当前速度
    float speedL = Encoder_GetSpeedLeft();
    float speedR = Encoder_GetSpeedRight();

    // PID计算
    pwmL = PID_Calc(&pid_left, speedL, target_left);
    pwmR = PID_Calc(&pid_right, speedR, target_right);

    // 最小PWM保护（防止电机堵转）
    if(pwmL > 0 && pwmL < MOTOR_MIN_PWM) pwmL = MOTOR_MIN_PWM;
    if(pwmL < 0 && pwmL > -MOTOR_MIN_PWM) pwmL = -MOTOR_MIN_PWM;
    if(pwmR > 0 && pwmR < MOTOR_MIN_PWM) pwmR = MOTOR_MIN_PWM;
    if(pwmR < 0 && pwmR > -MOTOR_MIN_PWM) pwmR = -MOTOR_MIN_PWM;

    // 更新里程计+设置电机速度
    update_odom();
    Motor_SetSpeed(pwmL, pwmR);
    Motor_SetSpeed(90, 90);

  }
}

/* 里程计发布任务 */
void odomPubTask(void const * argument)
{
	// 初始化上次唤醒时间（只执行一次）
	TickType_t xLastWakeTime = xTaskGetTickCount();
	
	// 固定 50ms 周期 = 20Hz 发布
	const TickType_t xPeriod = pdMS_TO_TICKS(50);

	for(;;)
	{
		// 等待下一个周期（精准、不漂移）
		vTaskDelayUntil(&xLastWakeTime, xPeriod);

		// 互斥锁访问里程计数据
		osMutexWait(odomDataMutexHandle, osWaitForever);
		odom_msg.x = odom_x;
		odom_msg.y = odom_y;
		odom_msg.yaw = yaw_angle;
		osMutexRelease(odomDataMutexHandle);

		// 发送ROS消息
		fast_ros_send_odom(&odom_msg);
	}
}

/* 激光数据发布任务 */
void scanPubTask(void const * argument)
{
  for(;;)
  {
    if(laser_ready)
    {
      osMutexWait(laserBufMutexHandle, osWaitForever);
      memcpy(laser_pub_buf, laser_buf1, sizeof(laser_pub_buf));
      osMutexRelease(laserBufMutexHandle);

      // 发送ROS激光消息
      fast_ros_send_scan(&scan_msg);
      laser_ready = 0;
    }
    osDelay(10);
  }
}

/* ROS指令接收任务 */
void cmdRecvTask(void const * argument)
{
  for(;;)
  {
		osDelay(2000); // 高频检测
    if(USART3_RX_FLAG)
    {
      geometry_msgs__Twist twist_recv;
      // 解析ROS速度指令
      fast_ros_parse_cmd_vel(USART3_RX_BUF, USART3_RX_LEN, &twist_recv);
      target_vx = twist_recv.linear_x;
      target_wz = twist_recv.angular_z;
      
      // 运动学解析为左右轮目标速度
      Kinematic_Analyze(target_vx, target_wz, &target_left, &target_right);
      
      // 清空接收缓存
      USART3_RX_FLAG = 0;
      USART3_ClearBuf();
    }
    osDelay(2000); // 高频检测
  }
}

/**
  * @brief  错误处理函数
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
    // 错误指示灯（可自行添加）
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    HAL_Delay(500);
  }
}

/* USER CODE BEGIN 4 */

// ===================== 【唯一正确】HAL 时基 = TIM6 =====================
void HAL_TIM_PeriodElapsedCallback_old(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
}
// ===================== 【唯一正确】HAL 时基 = TIM6 =====================
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }

  // 把 TIM1 的中断逻辑 也写在这里面！！！
  if (htim->Instance == TIM1)
  {
    if (scan_running)
      scan_tick++;
  }
}


/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  printf("Assert failed: file=%s line=%d\r\n", file, line);
}
#endif /* USE_FULL_ASSERT */