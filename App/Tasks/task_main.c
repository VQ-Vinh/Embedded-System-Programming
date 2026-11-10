/**
 * @file    task_main.c
 * @brief   MainTask: nơi duy nhất giữ trạng thái lượt cân, cấu hình, thống kê.
 *
 * Người phụ trách: Người 3 (lượt cân, thống kê, LED/còi) + Người 2 (menu, cấu hình).
 *
 * Vòng lặp: lấy hết sự kiện phím -> chờ số đo tối đa 20 ms -> cập nhật logic
 * (app_logic.c) -> cập nhật đèn/còi -> gửi bản chụp màn hình mỗi 100 ms.
 * Logic thật nằm trong App/Services/app_logic.c để kiểm thử được trên máy tính.
 */
#include "app_main.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "app_logic.h"
#include "flash_store.h"
#include "indicator.h"

#define MAIN_WAIT_MS     20U
#define MAIN_RENDER_MS   100U

static app_ctx_t s_app;
static DisplaySnapshot_t s_snap;

/** @brief Hook gửi bản ghi sang UartTask, chờ tối đa 50 ms nếu queue đầy. */
static void hook_emit(const ResultRecord_t *rec)
{
    /* Chờ tối đa 50 ms nếu UartTask đang bận; bản ghi kết quả không nên mất. */
    (void)osMessageQueuePut(qUartHandle, rec, 0U, 50U);
}

/** @brief Hook ghi cấu hình vào flash. */
static bool hook_save(const app_config_t *cfg)
{
    return flash_store_save(cfg);
}

/** @brief Hook điều khiển LED/còi. */
static void hook_indicate(indicate_t what, bool beep)
{
    indicator_set(what, beep, osKernelGetTickCount());
}

static const app_hooks_t HOOKS = { hook_emit, hook_save, hook_indicate };

/** @brief Thân MainTask: đọc cấu hình flash, xử lý phím và số đo, gửi bản chụp màn hình. Không bao giờ trả về. */
void MainTask_Run(void *argument)
{
    app_config_t stored;
    KeyEvent_t key;
    Measurement_t meas;
    uint32_t last_render = 0U;
    bool loaded;

    (void)argument;
    indicator_init();
    loaded = flash_store_load(&stored);
    app_init(&s_app, &HOOKS, loaded ? &stored : NULL, osKernelGetTickCount());

    for (;;)
    {
        uint32_t now;

        while (osMessageQueueGet(qKeyHandle, &key, NULL, 0U) == osOK)
        {
            app_on_key(&s_app, &key, osKernelGetTickCount());
        }

        if (osMessageQueueGet(qMeasHandle, &meas, NULL, MAIN_WAIT_MS) == osOK)
        {
            app_on_measurement(&s_app, &meas, osKernelGetTickCount());
        }

        now = osKernelGetTickCount();
        app_tick(&s_app, now);
        indicator_tick(now);

        if ((now - last_render) >= MAIN_RENDER_MS)
        {
            last_render = now;
            app_render(&s_app, &s_snap);
            /* * Ghi đè bản cũ: DisplayTask luôn vẽ trạng thái mới nhất. */
            (void)xQueueOverwrite((QueueHandle_t)qDisplayHandle, &s_snap);
        }
    }
}
