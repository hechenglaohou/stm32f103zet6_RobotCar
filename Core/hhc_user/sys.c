#include "main.h"
#include "cmsis_os.h"

// 假设系统时钟 72MHz, FreeRTOS 心跳 1ms (1000Hz)
#define fac_us         (72)    // 72MHz 下 1us = 72 个tick
#define fac_ms         (1000)  // OS 1个tick = 1ms

// 只读 SysTick->VAL 计数, 不修改任何寄存器!!!
void delay_us_old(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;    // 只读取, 不修改

    ticks = nus * fac_us;

    vTaskSuspendAll();                //  FreeRTOS 锁定调度（不破坏心跳）
    told = SysTick->VAL;              //  只读取

    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow;
            else
                tcnt += reload - tnow + told;

            told = tnow;

            if (tcnt >= ticks)
                break;
        }
    }

    xTaskResumeAll();                // 解锁调度
}



void delay_us(uint32_t us)
{
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    ticks /= 5; // 校准系数，需实际测试调整
    while(ticks--);
}


// 毫秒延时（任务里用OS延时，初始化用us延时）
void delay_ms(uint16_t nms)
{
    // 如果OS已经运行, 且在任务层
    if (osKernelRunning() && (__get_IPSR() == 0))
    {
        osDelay(nms);               // 不阻塞, 不破坏心跳
    }
    else
    {
        delay_us(nms * 1000);       // 初始化阶段用纯us延时
    }
}

// 兼容函数
void Delay_ms(uint32_t nms)
{
    delay_ms(nms);
}

#if 0
#include "sys.h"
#include "stm32f1xx_hal.h"

//static uint32_t fac_us = 0;
//static uint32_t fac_ms = 0;

// 延时初始化（HAL库正确写法）
void delay_init(void)
{
    // HAL库 SysTick 时钟 = HCLK 或 HCLK/8
    // STM32F1 默认：SysTick = HCLK (72MHz)
    // 我们手动配置成 72MHz / 8 = 9MHz，和你原来逻辑完全一样
    SysTick->CTRL &= ~(1 << 2); // 清零 CLKSOURCE 位 → 选择 HCLK_Div8

    //fac_us = SystemCoreClock / 8000000;
    //fac_ms = fac_us * 1000;
}

// 微秒延时 (不操作SysTick，不影响HAL)
void delay_us(uint32_t us)
{
    uint32_t cnt = us * 9;  // 72M 下精准 1us
    while(cnt--) __NOP();
}

// 毫秒延时 (基于HAL_Dela y，100% 安全)
void delay_ms(uint32_t nms)
{
    uint32_t i;
    for(; nms > 0; nms--)
    {
        i = 12000;  // 72MHz 下 ≈ 1ms
        while(i--);
    }
}



// ===================== 无阻塞延时 =====================
#define NOBLOCK_NUM  8

typedef struct {
    uint32_t last;
    uint32_t ms;
    uint8_t  flag;
} noblock_t;

static noblock_t nbt[NOBLOCK_NUM] = {0};

uint8_t delay_noblock(uint8_t ch, uint32_t ms)
{
    uint32_t t;

    if(ch >= NOBLOCK_NUM) return 0;

    if(nbt[ch].flag == 0)
    {
        nbt[ch].ms = ms;
        nbt[ch].last = HAL_GetTick();
        nbt[ch].flag = 1;
    }

    t = HAL_GetTick();

    if(t - nbt[ch].last >= nbt[ch].ms)
    {
        nbt[ch].flag = 0;
        return 1;
    }
    return 0;
}
#endif
