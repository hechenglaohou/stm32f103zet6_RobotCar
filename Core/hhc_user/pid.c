#include "pid.h"

void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float min, float max)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->out_min = min;
    pid->out_max = max;
    pid->target = 0;
    pid->error = 0;
    pid->last_error = 0;
    pid->integral = 0;
}

float PID_Calc(PID_TypeDef *pid, float feedback, float target)
{
    float output;
    pid->error = target - feedback;
    pid->integral += pid->error;

    if(pid->integral > pid->out_max) pid->integral = pid->out_max;
    if(pid->integral < pid->out_min) pid->integral = pid->out_min;

    output = pid->kp*pid->error + pid->ki*pid->integral + pid->kd*(pid->error-pid->last_error);
    pid->last_error = pid->error;

    if(output > pid->out_max) output = pid->out_max;
    if(output < pid->out_min) output = pid->out_min;

    return output;
}
