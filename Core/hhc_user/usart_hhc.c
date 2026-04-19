// 新版 HAL 库 支持，不报错
#include <stdio.h>
#include "usart.h"
// 关闭半主机模式（新版 Keil 写法）
#if (__ARMCC_VERSION >= 6000000)
__asm(".global __use_no_semihosting\n\t");
#endif

void _sys_exit(int x) {}

// HAL 库 printf 重定向
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);
  return ch;
}
