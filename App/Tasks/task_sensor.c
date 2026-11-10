/**
 * @file    task_sensor.c
 * @brief   SensorTask: đọc HX711, lọc trung bình trượt, đọc cảm biến quang,
 *          gửi Measurement_t sang MainTask qua qMeas.
 *
 * Người phụ trách: Người 1 (HX711) + Người 3 (cảm biến quang).
 *
 * Mỗi HX711_POLL_MS kiểm tra DOUT và chống dội cảm biến quang. Khi HX711 có
 * mẫu mới (~10 lần/s) thì lọc và gửi. Quá HX711_TIMEOUT_MS không có mẫu ->
 * gửi trạng thái lỗi và reset nguồn HX711.
 */
#include "app_main.h"

#include "app_config.h"
#include "app_types.h"
#include "hx711.h"
#include "photo_sensor.h"
#include "scale_filter.h"

/** @brief Đẩy dữ liệu vào queue, không chờ (bỏ qua nếu queue đầy). */
static void send(const Measurement_t *m)
{
    /* * Không chờ: nếu MainTask chậm thì bỏ mẫu này, mẫu sau sẽ tới ngay. */
    (void)osMessageQueuePut(qMeasHandle, m, 0U, 0U);
}

/** @brief Thân SensorTask: kiểm tra HX711 mỗi 5 ms, lọc và gửi số đo. Không bao giờ trả về. */
void SensorTask_Run(void *argument)
{
    static mavg_t filter;
    Measurement_t m = { 0 };
    uint32_t last_sample_ms;
    int32_t raw;

    (void)argument;
    hx711_init();
    mavg_reset(&filter);
    last_sample_ms = osKernelGetTickCount();

    for (;;)
    {
        uint32_t now = osKernelGetTickCount();

        photo_sensor_update();

        if (hx711_is_ready())
        {
            raw = hx711_read_raw();
            m.raw_last = raw;
            m.photo = photo_sensor_present() ? 1U : 0U;
            m.tick_ms = now;
            if ((raw >= HX711_RAW_MAX) || (raw <= HX711_RAW_MIN))
            {
                m.status = (uint8_t)SENSOR_ERR_SATURATED;
                mavg_reset(&filter);
            }
            else
            {
                m.status = (uint8_t)SENSOR_OK;
                m.raw_avg = mavg_push(&filter, raw);
            }
            send(&m);
            last_sample_ms = now;
        }
        else if ((now - last_sample_ms) > HX711_TIMEOUT_MS)
        {
            m.status = (uint8_t)SENSOR_ERR_TIMEOUT;
            m.photo = photo_sensor_present() ? 1U : 0U;
            m.tick_ms = now;
            send(&m);
            mavg_reset(&filter);
            hx711_power_cycle();
            last_sample_ms = now;
        }
        else
        {
            /* Chưa có mẫu mới. */
        }

        osDelay(HX711_POLL_MS);
    }
}
