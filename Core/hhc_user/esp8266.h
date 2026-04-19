#ifndef __ESP8266_H
#define __ESP8266_H

#include "main.h"  // HAL 库标准头文件，包含 uint8_t 等类型

// WiFi 配置
#define ESP8266_WIFI_SSID      "Galaxy M30sF6AF"
#define ESP8266_WIFI_PWD       "123456px"

// UDP 服务器配置
#define ESP8266_UDP_SERVER_IP  "192.168.43.212"
#define ESP8266_UDP_SERVER_PORT "8888"

// 函数声明（全部使用 HAL 标准类型）
uint8_t ESP8266_SendATCmd(uint8_t *cmd, uint8_t *ack, uint32_t timeout);
uint8_t ESP8266_Init(void);
uint8_t ESP8266_JoinAP(void);
uint8_t ESP8266_ConnectUDP(void);
uint8_t ESP8266_OpenTransparent(void);
void    ESP8266_ExitTransparent(void);
void ESP8266_PrintIP(void);


#endif
