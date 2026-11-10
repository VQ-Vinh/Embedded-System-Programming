/**
 * @file    indicator.c
 * @brief   Cài đặt điều khiển LED và còi.
 */
#include "indicator.h"

#include "main.h"
#include "app_config.h"

static indicate_t s_mode = IND_OFF;
static uint32_t s_beep_until = 0U;
static bool s_beeping = false;

/** @brief Bật/tắt một chân output (LED hoặc transistor còi). */
static void led(GPIO_TypeDef *port, uint16_t pin, bool on)
{
    HAL_GPIO_WritePin(port, pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* Mô tả đầy đủ: xem indicator.h. */
void indicator_init(void)
{
    led(LED_GREEN_GPIO_Port, LED_GREEN_Pin, false);
    led(LED_RED_GPIO_Port, LED_RED_Pin, false);
    led(BUZZER_GPIO_Port, BUZZER_Pin, false);
    s_mode = IND_OFF;
    s_beeping = false;
}

/* Mô tả đầy đủ: xem indicator.h. */
void indicator_set(indicate_t what, bool beep, uint32_t now_ms)
{
    s_mode = what;
    led(LED_GREEN_GPIO_Port, LED_GREEN_Pin, what == IND_PASS);
    led(LED_RED_GPIO_Port, LED_RED_Pin, (what == IND_FAIL) || (what == IND_FAULT));
    if (beep)
    {
        s_beeping = true;
        s_beep_until = now_ms + BUZZER_BEEP_MS;
        led(BUZZER_GPIO_Port, BUZZER_Pin, true);
    }
}

/* Mô tả đầy đủ: xem indicator.h. */
void indicator_tick(uint32_t now_ms)
{
    if (s_beeping && ((int32_t)(now_ms - s_beep_until) >= 0))
    {
        s_beeping = false;
        led(BUZZER_GPIO_Port, BUZZER_Pin, false);
    }
    if (s_mode == IND_FAULT)
    {
        /* Nhấp nháy 2 Hz để phân biệt lỗi với sản phẩm không đạt. */
        led(LED_RED_GPIO_Port, LED_RED_Pin, ((now_ms / 250U) % 2U) == 0U);
    }
}
