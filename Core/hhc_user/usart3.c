#include "usart3.h"
#include "stm32f1xx_hal.h"

/* 全局缓冲区和状态变量 - 与原附件完全一致 */
uint8_t USART3_RX_BUF[128];
uint32_t USART3_RX_LEN = 0;
uint8_t USART3_RX_FLAG = 0;

/* 声明USART3句柄（需在其他地方定义如stm32f1xx_hal_msp.c/主函数） */
extern UART_HandleTypeDef huart3;

/**
 * @brief  USART3初始化函数
 * @note   仅开启接收中断（1字节），与原附件初始化核心逻辑对齐
 * @param  无
 * @retval 无
 */
//void USART3_Init(void)
//{
//    // 仅开启1字节接收中断，与原附件"初始化仅使能RXNE中断"逻辑一致
//    HAL_UART_Receive_IT(&huart3, &USART3_RX_BUF[USART3_RX_LEN], 1);
//}

// HAL 库版本，完全替代你原来的标准库初始化
void MX_USART3_UART_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // ========== 1. 开时钟 ==========
  __HAL_RCC_USART3_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // ========== 2. PB10 TX 复用推挽 ==========
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // ========== 3. PB11 RX 浮空输入 ==========
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // ========== 4. 串口配置 ==========
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart3);

  // ========== 5. 开启接收中断 ==========
  __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);

  // ========== 6. 中断优先级（和你标准库一样） ==========
  HAL_NVIC_SetPriority(USART3_IRQn, 1, 1);
  HAL_NVIC_EnableIRQ(USART3_IRQn);
}

#if 0
/* USART3 init function */
void MX_USART3_UART_Init(void)
{
  /* ************************ 新增/修改 开始 ************************ */
  huart3.Instance = USART3;                    // 绑定USART3实例
  huart3.Init.BaudRate = 115200;               // 波特率配置（可根据需求修改）
  huart3.Init.WordLength = UART_WORDLENGTH_8B; // 8位数据位
  huart3.Init.StopBits = UART_STOPBITS_1;      // 1位停止位
  huart3.Init.Parity = UART_PARITY_NONE;       // 无校验位
  huart3.Init.Mode = UART_MODE_TX_RX;          // 收发模式
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE; // 无硬件流控
  huart3.Init.OverSampling = UART_OVERSAMPLING_16; // 16倍过采样
  if (HAL_UART_Init(&huart3) != HAL_OK)        // 初始化USART3
  {
    Error_Handler();                           // 初始化失败则进入错误处理
  }

  // 启用USART3接收中断（和USART1逻辑对齐，适配中断接收）
  __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
  // 配置USART3中断优先级（可选，根据系统中断优先级规划调整）
  HAL_NVIC_SetPriority(USART3_IRQn, 3, 4); // 优先级低于USART1，避免冲突
  HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* ************************ 新增/修改 结束 ************************ */
}
#endif
/**
 * @brief  发送单个字节
 * @note   与原附件USART3_SendByte逻辑完全一致（阻塞发送1字节）
 * @param  data: 要发送的字节
 * @retval 无
 */
void USART3_SendByte(uint8_t data)
{
    // HAL阻塞发送1字节，超时100ms（对齐原附件"等待TXE后发数据"逻辑）
    HAL_UART_Transmit(&huart3, &data, 1, 100);
}

/**
 * @brief  发送字符串
 * @note   与原附件USART3_SendString逻辑完全一致（逐字节发送）
 * @param  str: 字符串指针
 * @retval 无
 */
void USART3_SendString(uint8_t *str)
{
    while (*str)
    {
        USART3_SendByte(*str);
        str++;
    }
}

/**
 * @brief  清空接收缓冲区
 * @note   与原附件USART3_ClearBuf逻辑完全一致（清长度、清标志、重置接收）
 * @param  无
 * @retval 无
 */
void USART3_ClearBuf(void)
{
    USART3_RX_LEN = 0;
    USART3_RX_FLAG = 0;
    // 重新开启1字节接收中断，对齐原附件"仅重置缓冲区状态"逻辑
    HAL_UART_Receive_IT(&huart3, &USART3_RX_BUF[0], 1);
}

/**
 * @brief  HAL UART接收完成回调函数
 * @note   与原附件USART3_IRQHandler逻辑逐行对齐（仅处理USART3、缓冲区、标志）
 * @param  huart: UART句柄指针
 * @retval 无
 */
// 正确 HAL 接收中断（完全等价于你原来的标准库代码）
void USART3_IRQHandler_old(void)
{
    uint8_t Res;

    // 判断接收中断（HAL 等价写法）
    if(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE) != RESET)
    {
        // 1. 自己读数据（和标准库一样）
        Res = (uint8_t)(USART3->DR & 0x00FF);

        // 2. 自己存数据（和标准库一样）
        if(USART3_RX_LEN < 127)
        {
            USART3_RX_BUF[USART3_RX_LEN++] = Res;
        }

        // 3. 标志位置1（和标准库一样）
        USART3_RX_FLAG = 1;

        // 4. 清除标志（HAL 等价写法）
        __HAL_UART_CLEAR_FLAG(&huart3, UART_FLAG_RXNE);
    }

    // HAL 必须调用
    HAL_UART_IRQHandler(&huart3);
}



// 完全等价于你原来的标准库中断
void USART3_IRQHandler(void)
{
  uint8_t res;

  // HAL 方式判断接收中断
  if(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE) != RESET)
  {
    // 读取数据
    res = (uint8_t)(USART3->DR & 0x00FF);

    // 存入缓冲区（和你原来逻辑一样）
    if(USART3_RX_LEN < 127)
    {
      USART3_RX_BUF[USART3_RX_LEN++] = res;
    }

    // 接收标志位置1
    USART3_RX_FLAG = 1;

    // 清除中断标志
    __HAL_UART_CLEAR_FLAG(&huart3, UART_FLAG_RXNE);
  }

  // HAL 库必须调用
  HAL_UART_IRQHandler(&huart3);
}
