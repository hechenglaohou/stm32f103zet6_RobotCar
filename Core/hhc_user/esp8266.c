#include "esp8266.h"
#include "usart3.h"
#include "string.h"
#include "main.h" 
#include "sys.h"
#include <stdio.h>

// 发送AT指令并等待指定应答
uint8_t ESP8266_SendATCmd(uint8_t* cmd, uint8_t* ack, uint32_t timeout)
{
    USART3_ClearBuf();
    USART3_SendString(cmd);
    USART3_SendString((uint8_t*)"\r\n");

    while(timeout--)
    {
        HAL_Delay(1);  // 统一使用HAL库延迟接口，上电即可用
        
        if(USART3_RX_FLAG)
        {
            if(strstr((char*)USART3_RX_BUF, (char*)ack))
            {
                return 0; // 匹配到应答，返回成功
            }
            USART3_ClearBuf(); // 未匹配到，清空缓冲区继续等待
        }
    }
    return 1; // 超时未匹配，返回失败
}

// ESP8266初始化（设置STA模式+重启）
uint8_t ESP8266_Init(void)
{
    HAL_Delay(500);
    if(ESP8266_SendATCmd((uint8_t*)"AT", (uint8_t*)"OK", 1000)) return 1;
    if(ESP8266_SendATCmd((uint8_t*)"AT+CWMODE=1", (uint8_t*)"OK", 1000)) return 1;
    if(ESP8266_SendATCmd((uint8_t*)"AT+RST", (uint8_t*)"OK", 2000)) return 1;
    HAL_Delay(2000);
    return 0;
}

// 连接WiFi AP
uint8_t ESP8266_JoinAP(void)
{
    uint8_t buf[64];
    sprintf((char*)buf,"AT+CWJAP=\"%s\",\"%s\"",ESP8266_WIFI_SSID,ESP8266_WIFI_PWD);
    return ESP8266_SendATCmd(buf, (uint8_t*)"WIFI GOT IP", 15000);
}

// 建立UDP连接
uint8_t ESP8266_ConnectUDP(void)
{
    uint8_t buf[64];
    sprintf((char*)buf,"AT+CIPSTART=\"UDP\",\"%s\",%s",ESP8266_UDP_SERVER_IP,ESP8266_UDP_SERVER_PORT);
    return ESP8266_SendATCmd(buf, (uint8_t*)"OK", 8000);
}

// 开启透传模式
uint8_t ESP8266_OpenTransparent(void)
{
    if(ESP8266_SendATCmd((uint8_t*)"AT+CIPMODE=1", (uint8_t*)"OK", 1000)) return 1;
    HAL_Delay(200);
    if(ESP8266_SendATCmd((uint8_t*)"AT+CIPSEND", (uint8_t*)">", 2000)) return 1;
    HAL_Delay(200);
    return 0;
}

// 退出透传模式（发送+++）
void ESP8266_ExitTransparent(void)
{
    USART3_SendByte('+');
    USART3_SendByte('+');
    USART3_SendByte('+');
    HAL_Delay(800);
}

// 打印ESP8266的IP地址（发送AT+CIFSR指令并读取返回）
void ESP8266_PrintIP(void)
{
    uint32_t timeout = 3000;
    USART3_ClearBuf();

    // STM32 自动发送指令给 ESP8266
    USART3_SendString((uint8_t*)"AT+CIFSR\r\n");

    // 等待3秒，确保数据接收完整
    while (timeout > 0)
    {
        HAL_Delay(1);
        timeout--;
    }

    printf("==== ESP8266 IP ====\r\n");
    if(USART3_RX_LEN > 0)
    {
        printf("Received: %s\r\n", USART3_RX_BUF);
    }
    else
    {
        printf("USART3_RX_BUF is empty! (ESP8266 no response)\r\n");
    }
    USART3_ClearBuf();
}
