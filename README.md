STM32F103ZET6 RobotCar README.md
# STM32F103ZET6 RobotCar

STM32F103ZET6 智能小车 - 电机调速 / 舵机激光雷达 / FreeRTOS / VL53L0X

![项目效果图](此处插入效果图路径) | [完整项目下载](#项目下载)

项目简介
基于 STM32F103ZET6 主控芯片，在 Keil MDK5 环境下开发的智能小车开源项目。集成电机闭环调速、舵机配合 VL53L0X 激光测距实现简易扫描雷达、MPU6050 姿态检测、PID 闭环控制、FreeRTOS 实时多任务调度，同时支持 OLED 状态显示、ESP8266 通信扩展，可直接用于学习、二次开发或课程设计。

项目源码基于 STM32 HAL 库开发，文件结构清晰，注释规范，所有功能模块（电机、雷达、姿态、通信）均独立封装，便于修改和扩展。

硬件清单
主控板：STM32F103ZET6 开发板

电机模块：直流减速电机 + 编码器（用于速度反馈）

舵机：SG90 舵机（用于驱动激光雷达扫描）

激光测距：VL53L0X ToF 激光测距传感器（I2C 通信）

姿态传感器：MPU6050（加速度计+陀螺仪，I2C 通信）

PWM 扩展：PCA9685（用于舵机、电机 PWM 控制扩展）

显示模块：OLED 显示屏（I2C 通信，用于实时显示小车状态）

通信模块：ESP8266（串口通信，用于无线控制扩展）

辅助：12V 电源、电机驱动板（如 L298N）、杜邦线若干

接线表（STM32F103ZET6 ↔ 各模块）
说明：接线时请断电操作，确保引脚对应正确，避免短路损坏器件；I2C 模块（VL53L0X、MPU6050、PCA9685、OLED）可共线（SDA/SCL 复用），注意区分地址。

STM32F103ZET6 引脚	连接模块	模块引脚	备注
PB6（I2C1_SCL）	VL53L0X / MPU6050 / PCA9685 / OLED	SCL	I2C 时钟线，可共线
PB7（I2C1_SDA）	VL53L0X / MPU6050 / PCA9685 / OLED	SDA	I2C 数据线，可共线
PA0（TIM2_CH1）	SG90 舵机	PWM 信号脚	舵机角度控制，也可通过 PCA9685 扩展
PA8（TIM1_CH1）	电机1 PWM	PWM 信号脚	电机速度控制，接电机驱动板
PA9（TIM1_CH2）	电机2 PWM	PWM 信号脚	电机速度控制，接电机驱动板
PA10（USART1_RX）	ESP8266	TX	ESP8266 无线通信，串口接收
PA9（USART1_TX）	ESP8266	RX	ESP8266 无线通信，串口发送
PB0 / PB1	编码器	A 相 / B 相	电机速度反馈，接编码器输出脚
3.3V	所有模块	VCC	给 VL53L0X、MPU6050、OLED、PCA9685 供电
GND	所有模块	GND	共地，避免干扰
主要功能
电机控制：基于 PID 算法的闭环调速，通过编码器反馈实现精准速度控制，支持前进、后退、转向

简易激光雷达：SG90 舵机匀速扫描 + VL53L0X 实时测距，可检测周围障碍物距离

多任务调度：基于 FreeRTOS 实现，将雷达扫描、电机控制、串口通信、OLED 显示拆分为独立任务，运行稳定

姿态检测：MPU6050 采集小车姿态数据（角度、加速度），可用于小车平衡控制（可二次开发）

状态显示：OLED 实时显示电机速度、障碍物距离、舵机角度等关键参数

通信扩展：ESP8266 支持无线通信，可通过手机/电脑远程控制小车（需二次配置）

运动学解算：集成小车运动学正逆解算法，支持精准运动控制

开发环境
IDE：Keil MDK5（版本 5.28 及以上）

固件库：STM32 HAL 库（STM32F1xx HAL Driver）

操作系统：FreeRTOS（实时多任务调度）

调试工具：ST-Link（用于下载程序、在线调试）

文件说明
项目文件按功能模块分类，结构清晰，便于维护和二次开发：

main.c / main.h：主程序入口，FreeRTOS 任务创建与调度

motor.c / motor.h：电机驱动、速度控制、正反转逻辑

vl53l0x.c / vl53l0x.h：VL53L0X 激光测距传感器驱动，测距函数封装

pid.c / pid.h：PID 算法实现，用于电机闭环调速

freertos.c：FreeRTOS 任务定义、任务调度配置

tim.c / tim.h：定时器配置，用于 PWM 输出（电机、舵机）、编码器计数

i2c.c / i2c.h / i2c_hhc.c / i2c_hhc.h：I2C 通信驱动，适配所有 I2C 模块

oled.c / oled.h：OLED 显示屏驱动，状态显示函数

encoder.c / encoder.h：编码器驱动，速度采集与计算

kinematics.c / kinematics.h：小车运动学正逆解算法

mpu6050.c / mpu6050.h：MPU6050 姿态传感器驱动，数据采集

pca9685.c / pca9685.h：PCA9685 PWM 扩展板驱动，用于舵机/电机扩展控制

esp8266.c / esp8266.h：ESP8266 无线通信驱动，串口数据收发

usart.c / usart.h / usart3.c / usart3.h：串口驱动，用于调试、ESP8266 通信

sys.c / sys.h / system_stm32f1xx.c：系统配置、时钟初始化

stm32f1xx_hal_msp.c / stm32f1xx_it.c：HAL 库底层配置、中断服务函数

.gitignore：Git 忽略文件，过滤 Keil 编译垃圾、工程配置文件

LICENSE：开源许可证（MIT License）

使用步骤（新手必看）
1. 下载项目源码
方式1：直接下载 ZIP 包（无需 Git） 方式2：使用 Git 克隆到本地

2. 打开项目工程
打开 Keil MDK5，点击「Project」→「Open Project」，找到项目文件夹中的 `.uvprojx` 文件（STM32F103ZET6 工程文件），双击打开。

打开后，Keil 会自动加载所有源码文件，若提示「缺少 HAL 库」，请手动添加 STM32F1xx HAL 库（或直接替换为自己的 HAL 库路径）。

3. 配置与编译
检查项目配置：点击「魔术棒」→「Device」，确认选择「STM32F103ZET6」；「Target」中确认时钟频率为 72MHz。

根据自己的硬件接线，修改 `main.c`、`motor.c`、`vl53l0x.c` 中的引脚定义（若与接线表一致，可直接跳过）。

点击 Keil 工具栏「Build」按钮，编译项目，若没有错误，会生成 `.hex` 文件（用于下载到主控板）。

4. 下载程序到主控板
将 ST-Link 调试器连接到 STM32F103ZET6 开发板，另一端连接电脑 USB 口。

在 Keil 中点击「Download」按钮，将编译好的程序下载到主控板。

下载完成后，断开 ST-Link，给小车接上 12V 电源，即可启动程序。

5. 功能测试
启动后，OLED 会显示电机速度、舵机角度、障碍物距离等信息。

舵机会匀速扫描，VL53L0X 实时检测距离，若检测到障碍物，可通过代码修改实现小车避障（需二次开发）。

通过串口调试助手（波特率默认 115200），可查看实时数据，也可发送指令控制小车运动（需配置串口指令逻辑）。


项目下载
ZIP 完整下载（直接解压可用）：https://github.com/hechenglaohou/stm32f103zet6_RobotCar/archive/refs/heads/main.zip

Git 克隆地址（需安装 Git）：git clone https://github\.com/hechenglaohou/stm32f103zet6\_RobotCar\.git

注意事项
接线时务必断电，避免接反电源或引脚接错，导致器件损坏。

若 VL53L0X 测距异常，检查 I2C 接线是否正确，或修改 `vl53l0x.c` 中的传感器地址。

电机调速异常时，检查 PID 参数（在 `pid.c` 中修改），或编码器接线是否正确。

FreeRTOS 任务优先级可根据需求调整，避免高优先级任务阻塞低优先级任务。

项目仅提供基础功能，可根据需求二次开发（如避障算法、无线遥控、路径规划等）。

开源许可证
本项目采用 MIT License 开源，允许个人、企业自由使用、修改、商用，只需注明原作者及项目地址即可。

作者信息
GitHub 地址：https://github.com/hechenglaohou
