/**
 * @file    hx711.c
 * @brief   Cài đặt driver HX711 bằng GPIO + DWT.
 */
#include "hx711.h"

#include "main.h"
#include "dwt_delay.h"

#define SCK_HIGH() (HX711_SCK_GPIO_Port->BSRR = HX711_SCK_Pin)
#define SCK_LOW()  (HX711_SCK_GPIO_Port->BSRR = ((uint32_t)HX711_SCK_Pin << 16U))
#define DOUT_READ() ((HX711_DOUT_GPIO_Port->IDR & HX711_DOUT_Pin) != 0U)

/* Mô tả đầy đủ: xem hx711.h. */
void hx711_init(void)
{
    dwt_delay_init();
    SCK_LOW();
}

/* Mô tả đầy đủ: xem hx711.h. */
bool hx711_is_ready(void)
{
    return !DOUT_READ();
}

/* Mô tả đầy đủ: xem hx711.h. */
int32_t hx711_read_raw(void)
{
    uint32_t value = 0U;
    uint32_t primask;
    uint8_t i;

    // ! Cấm ngắt trong lúc phát xung: nếu một ngắt/đổi task làm SCK cao quá
    // ! 60 us thì HX711 tự tắt và mẫu bị hỏng. Toàn bộ vùng này ~55 us.
    primask = __get_PRIMASK();
    __disable_irq();

    for (i = 0U; i < 24U; i++)
    {
        SCK_HIGH();
        dwt_delay_us(1U);
        value = (value << 1) | (DOUT_READ() ? 1U : 0U);
        SCK_LOW();
        dwt_delay_us(1U);
    }
    /* Xung thứ 25: lần đọc sau dùng kênh A, gain 128. */
    SCK_HIGH();
    dwt_delay_us(1U);
    SCK_LOW();
    dwt_delay_us(1U);

    __set_PRIMASK(primask);

    /* Mở rộng dấu từ 24 bit sang 32 bit. */
    if ((value & 0x800000U) != 0U)
    {
        value |= 0xFF000000U;
    }
    return (int32_t)value;
}

/* Mô tả đầy đủ: xem hx711.h. */
void hx711_power_cycle(void)
{
    SCK_HIGH();
    dwt_delay_us(100U);
    SCK_LOW();
}
