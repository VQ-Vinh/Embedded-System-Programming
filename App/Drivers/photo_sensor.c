/**
 * @file    photo_sensor.c
 * @brief   Cài đặt đọc cảm biến quang với bộ đếm chống dội.
 */
#include "photo_sensor.h"

#include <stdint.h>
#include "main.h"
#include "app_config.h"

static bool s_state = false;
static uint8_t s_count = 0U;

/* Mô tả đầy đủ: xem photo_sensor.h. */
void photo_sensor_update(void)
{
    GPIO_PinState level = HAL_GPIO_ReadPin(PHOTO_IN_GPIO_Port, PHOTO_IN_Pin);
    bool raw = (level == ((PHOTO_ACTIVE_LEVEL != 0) ? GPIO_PIN_SET : GPIO_PIN_RESET));

    if (raw == s_state)
    {
        s_count = 0U;
    }
    else if (++s_count >= PHOTO_DEBOUNCE_COUNT)
    {
        s_state = raw;
        s_count = 0U;
    }
    else
    {
        /* Chờ đủ số lần đọc giống nhau. */
    }
}

/* Mô tả đầy đủ: xem photo_sensor.h. */
bool photo_sensor_present(void)
{
    return s_state;
}
