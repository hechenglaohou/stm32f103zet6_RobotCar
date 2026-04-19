#include "fast_ros.h"

#include "usart3.h"
#include <stdio.h>
#include <string.h>
#include "sys.h"

void fast_ros_init(void)
{
}

void fast_ros_send_odom(nav_msgs__Odometry *odom)
{
    uint8_t buf[256];
    //int len;

    // 取消注释，格式化ODOM数据
    sprintf((char*)buf,
        "ODOM %.2f %.2f %.2f %.2f %.2f\n",
        odom->x, odom->y, odom->yaw, odom->vx, odom->wz);

    USART3_SendString(buf);
}




void fast_ros_send_scan(sensor_msgs__LaserScan *scan)
{
    char buf[128];  // 缓冲区大小128，不爆
    int i;

    // 发：SCAN（只发4字节）
    sprintf(buf, "SCAN");
    USART3_SendString((uint8_t*)buf);
	  osDelay(1);

    // 循环发：角度 距离
    for(i=0; i<120; i++)
    {
        float dis = scan->ranges[i];
        if(dis < 0.02) dis = 0.02;
        if(dis > 2.0)  dis = 2.0;

        // 每次只发一对，绝对不超128字节
        sprintf(buf, " %.1f %.2f", (float)i, dis);
        USART3_SendString((uint8_t*)buf);
			  osDelay(1);
			   
    }

    // 发换行（告诉Python一帧结束）
    sprintf(buf, "\n");
    USART3_SendString((uint8_t*)buf);
		osDelay(1);
}



// 原来的函数 → 替换成这个
void fast_ros_send_scan_old(sensor_msgs__LaserScan *scan)
{
    char buf[256];  // 一次性发完，不拆分
    int i;

    // 严格匹配 Python 格式：SCAN 120个点 换行
    sprintf(buf, "SCAN");

    // 发送 120 个激光点
    for(i=0; i<120; i++)
    {
        float dis = scan->ranges[i];
        if(dis < 0.02) dis = 0.02;
        if(dis > 2.0)  dis = 2.0;

        // 拼接字符串
        sprintf(buf + strlen(buf), " %.2f", dis);
    }

    // 结尾换行
    sprintf(buf + strlen(buf), "\n");

    // 一次性发送
    USART3_SendString((uint8_t*)buf);
}




void fast_ros_parse_cmd_vel(uint8_t *buf, uint16_t len, geometry_msgs__Twist *twist)
{
    float vx, wz;

    if (sscanf((char*)buf, "CMD %f %f", &vx, &wz) == 2)
    {
        twist->linear_x = vx;
        twist->angular_z = wz;
    }
}

// 发送 cmd_vel 线速度 + 角速度
void fast_ros_send_cmd_vel(float *vx_ptr, float *wz_ptr)
{
    char buf[128];
    float vx = *vx_ptr;   // 取出 target_vx
    float wz = *wz_ptr;   // 取出 target_wz

    // 发送帧头
    sprintf(buf, "VEL_hhc");
    USART3_SendString((uint8_t*)buf);
    osDelay(1);

    // 发送 vx 和 wz
    sprintf(buf, " %.2f %.2f", vx, wz);
    USART3_SendString((uint8_t*)buf);
    osDelay(1);

    // 发送结束换行
    sprintf(buf, "\n");
    USART3_SendString((uint8_t*)buf);
    osDelay(1);
}
