#ifndef __MPU6050_H
#define __MPU6050_H


void MPU6050_Init(void);
void MPU6050_Read(void);
float MPU6050_GetYaw(void);

// 只在这里声明，不要定义！
extern float yaw;


#endif
