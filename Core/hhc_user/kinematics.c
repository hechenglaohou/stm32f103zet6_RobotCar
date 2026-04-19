#include "kinematics.h"
#include <math.h>

#define M_PI 3.1415926535f

static float wheel_dia, wheel_base;
static int encoder_ppr;
static float x=0,y=0,yaw=0;

// ??/????(????????)
extern int encoder_ppr;
extern float wheel_dia;      // ????
extern float wheel_base;     // ????


void Kinematic_Init(float dia, float base, int ppr)
{
    wheel_dia = dia;
    wheel_base = base;
    encoder_ppr = ppr;
}

void Kinematic_Analyze(float vx, float wz, float *left, float *right)
{
    *left = vx - wz * wheel_base / 2.0f;
    *right = vx + wz * wheel_base / 2.0f;
}


// ????????(???)
void Kinematic_Update(int delta_left, int delta_right, float *ox, float *oy, float *oyaw)
{
    // 1. ?? ? ??????
    float dl = (float)delta_left / encoder_ppr * M_PI * wheel_dia;
    float dr = (float)delta_right / encoder_ppr * M_PI * wheel_dia;
    
    // 2. ?????? & ????
    float d = (dl + dr) / 2.0f;
    float dyaw = (dr - dl) / wheel_base;

    // 3. ??????????????(????)
    float x = *ox;
    float y = *oy;
    float yaw = *oyaw;

    x += d * cosf(yaw);
    y += d * sinf(yaw);
    yaw += dyaw;

    // 4. ????? [-p, p]
    while(yaw > M_PI)  yaw -= 2*M_PI;
    while(yaw < -M_PI) yaw += 2*M_PI;

    // ????
    *ox = x;
    *oy = y;
    *oyaw = yaw;
}


void Kinematic_Update_unuse(int left, int right, float *ox, float *oy, float *oyaw)
{
    float dl = (float)left / encoder_ppr * 3.14159f * wheel_dia;
    float dr = (float)right / encoder_ppr * 3.14159f * wheel_dia;
    float d = (dl + dr) / 2.0f;

    x += d * cos(yaw);
    y += d * sin(yaw);
    yaw += (dr - dl) / wheel_base;

    *ox = x;
    *oy = y;
    *oyaw = yaw;
}
