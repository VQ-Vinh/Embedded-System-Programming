/**
 * @file    keypad.c
 * @brief   Cài đặt quét keypad 4x4 bằng GPIO.
 */
#include "keypad.h"

#include "main.h"
#include "dwt_delay.h"

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} pin_t;

static const pin_t ROWS[4] = {
    { KP_R1_GPIO_Port, KP_R1_Pin }, { KP_R2_GPIO_Port, KP_R2_Pin },
    { KP_R3_GPIO_Port, KP_R3_Pin }, { KP_R4_GPIO_Port, KP_R4_Pin },
};

static const pin_t COLS[4] = {
    { KP_C1_GPIO_Port, KP_C1_Pin }, { KP_C2_GPIO_Port, KP_C2_Pin },
    { KP_C3_GPIO_Port, KP_C3_Pin }, { KP_C4_GPIO_Port, KP_C4_Pin },
};

static const char KEYMAP[4][4] = {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' },
};

/* Mô tả đầy đủ: xem keypad.h. */
void keypad_init(void)
{
    uint8_t r;
    for (r = 0U; r < 4U; r++)
    {
        HAL_GPIO_WritePin(ROWS[r].port, ROWS[r].pin, GPIO_PIN_SET);
    }
}

/* Mô tả đầy đủ: xem keypad.h. */
char keypad_scan(void)
{
    char key = 0;
    uint8_t r;
    uint8_t c;

    for (r = 0U; (r < 4U) && (key == 0); r++)
    {
        HAL_GPIO_WritePin(ROWS[r].port, ROWS[r].pin, GPIO_PIN_RESET);
        /* * Chờ đường dây ổn định (điện dung dây + pull-up nội ~40 kOhm). */
        dwt_delay_us(5U);
        for (c = 0U; c < 4U; c++)
        {
            if (HAL_GPIO_ReadPin(COLS[c].port, COLS[c].pin) == GPIO_PIN_RESET)
            {
                key = KEYMAP[r][c];
                break;
            }
        }
        HAL_GPIO_WritePin(ROWS[r].port, ROWS[r].pin, GPIO_PIN_SET);
    }
    return key;
}
