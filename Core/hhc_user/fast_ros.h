#ifndef __FAST_ROS_H
#define __FAST_ROS_H


#include <stdint.h>
#include <string.h>

typedef struct {
    float linear_x;
    float angular_z;
} geometry_msgs__Twist;

typedef struct {
    float x;
    float y;
    float yaw;
    float vx;
    float wz;
} nav_msgs__Odometry;

typedef struct {
    float angle_min;
    float angle_max;
    float angle_increment;
    float range_min;
    float range_max;
    float *ranges;
    uint16_t size;
} sensor_msgs__LaserScan;

void fast_ros_init(void);
void fast_ros_send_odom(nav_msgs__Odometry *odom);
void fast_ros_send_scan(sensor_msgs__LaserScan *scan);
void fast_ros_parse_cmd_vel(uint8_t *buf, uint16_t len, geometry_msgs__Twist *twist);
void fast_ros_send_cmd_vel(float *vx_ptr, float *wz_ptr);

#endif
