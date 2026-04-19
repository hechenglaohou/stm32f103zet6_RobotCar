#ifndef __KINEMATICS_H
#define __KINEMATICS_H


void Kinematic_Init(float wheel_dia, float base, int ppr);
void Kinematic_Analyze(float vx, float wz, float *left, float *right);
void Kinematic_Update(int left_tick, int right_tick, float *x, float *y, float *yaw);

#endif
