/**
 * @file    dwt_delay.c
 * @brief   Cài đặt trễ micro giây bằng DWT->CYCCNT.
 */
#include "dwt_delay.h"

#include "stm32f4xx.h"

/* Mô tả đầy đủ: xem dwt_delay.h. */
void dwt_delay_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* Mô tả đầy đủ: xem dwt_delay.h. */
void dwt_delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000U);

    /* * Phép trừ không dấu vẫn đúng khi CYCCNT tràn qua 0. */
    while ((DWT->CYCCNT - start) < ticks)
    {
    }
}
