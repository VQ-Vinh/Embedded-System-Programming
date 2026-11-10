/**
 * @file    task_keypad.c
 * @brief   KeypadTask: quét keypad, chống dội, gửi sự kiện nhấn/giữ qua qKey.
 *
 * Người phụ trách: Người 2.
 *
 * Phím được chấp nhận khi đọc giống nhau KEYPAD_DEBOUNCE_SCANS lần liên tiếp
 * (3 x 10 ms). Giữ phím lâu hơn KEYPAD_HOLD_MS gửi thêm đúng một sự kiện HOLD.
 */
#include "app_main.h"

#include <stdbool.h>
#include "app_config.h"
#include "app_types.h"
#include "keypad.h"

/** @brief Đẩy dữ liệu vào queue, không chờ (bỏ qua nếu queue đầy). */
static void send(char key, key_evt_type_t type)
{
    KeyEvent_t e;
    e.key = key;
    e.type = (uint8_t)type;
    (void)osMessageQueuePut(qKeyHandle, &e, 0U, 0U);
}

/** @brief Thân KeypadTask: quét keypad mỗi 10 ms, chống dội, gửi PRESS/HOLD. Không bao giờ trả về. */
void KeypadTask_Run(void *argument)
{
    char candidate = 0;
    char stable_key = 0;
    uint8_t same_count = 0U;
    uint32_t press_ms = 0U;
    bool hold_sent = false;
    uint32_t wake;

    (void)argument;
    keypad_init();
    wake = osKernelGetTickCount();

    for (;;)
    {
        char k = keypad_scan();
        uint32_t now = osKernelGetTickCount();

        if (k == candidate)
        {
            if (same_count < 255U)
            {
                same_count++;
            }
        }
        else
        {
            candidate = k;
            same_count = 1U;
        }

        if ((same_count == KEYPAD_DEBOUNCE_SCANS) && (candidate != stable_key))
        {
            stable_key = candidate;
            if (stable_key != 0)
            {
                send(stable_key, KEY_EVT_PRESS);
                press_ms = now;
                hold_sent = false;
            }
        }

        if ((stable_key != 0) && !hold_sent && ((now - press_ms) >= KEYPAD_HOLD_MS))
        {
            send(stable_key, KEY_EVT_HOLD);
            hold_sent = true;
        }

        /* * osDelayUntil giữ chu kỳ quét đều 10 ms, không trôi theo thời gian quét. */
        wake += KEYPAD_SCAN_MS;
        (void)osDelayUntil(wake);
    }
}
