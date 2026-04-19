#ifndef __PID_H
#define __PID_H


typedef struct
{
    float kp,ki,kd;
    float target;
    float error,last_error;
    float integral;
    float out_min,out_max;
}PID_TypeDef;

void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float min, float max);
float PID_Calc(PID_TypeDef *pid, float feedback, float target);

#endif
