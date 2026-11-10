/**
 * @file    app_logic.h
 * @brief   Logic ứng dụng của MainTask: menu, nhập ngưỡng, trừ bì, hiệu chuẩn,
 *          quản lý lượt cân, cảnh báo và dựng nội dung màn hình.
 *
 * Toàn bộ trạng thái nằm trong app_ctx_t và chỉ MainTask được sửa. Các tác
 * động ra ngoài (gửi bản ghi UART, ghi flash, bật LED/còi) đi qua bảng hàm
 * app_hooks_t, nhờ vậy module này không gọi HAL/FreeRTOS và chạy được
 * trong chương trình kiểm thử trên máy tính (tests/test_logic.c).
 */
#ifndef APP_LOGIC_H
#define APP_LOGIC_H

#include <stdbool.h>
#include <stdint.h>
#include "app_types.h"
#include "config.h"
#include "num_input.h"
#include "scale_filter.h"
#include "weigh_fsm.h"

/** Lệnh cho đèn và còi. */
typedef enum
{
    IND_OFF = 0,    /**< Tắt hết */
    IND_PASS,       /**< LED xanh */
    IND_FAIL,       /**< LED đỏ (+ còi nếu beep = true) */
    IND_FAULT       /**< LED đỏ nhấp nháy: lỗi cảm biến / quá tải */
} indicate_t;

/** Các tác động ra ngoài do MainTask cung cấp. */
typedef struct
{
    void (*emit_record)(const ResultRecord_t *rec);   /**< Đẩy bản ghi sang UartTask */
    bool (*save_config)(const app_config_t *cfg);     /**< Ghi flash; true = thành công */
    void (*indicate)(indicate_t what, bool beep);     /**< Điều khiển LED/còi */
} app_hooks_t;

typedef enum
{
    UI_HOME = 0,
    UI_EDIT_TARGET,
    UI_EDIT_TOL,
    UI_MENU,
    UI_CAL_EMPTY,
    UI_CAL_REF,
    UI_CAL_PLACE,
    UI_CAL_CONFIRM,
    UI_CLEAR_CONFIRM
} ui_mode_t;

typedef struct
{
    const app_hooks_t *hooks;
    app_config_t cfg;
    bool         cfg_loaded;        /**< true nếu cấu hình đọc được từ flash */

    /* Số đo mới nhất */
    int32_t  raw_avg;
    int32_t  gross_x10;             /**< Khối lượng đã hiệu chuẩn, chưa trừ bì */
    int32_t  net_x10;               /**< Khối lượng tịnh = gross - bì */
    int32_t  tare_x10;
    bool     stable;
    bool     photo;
    uint8_t  sensor_status;         /**< sensor_status_t */
    bool     have_meas;
    bool     sensor_lost;
    bool     overload;
    uint32_t last_meas_ms;
    uint32_t now_ms;
    stab_t   stab;

    /* Lượt cân và thống kê */
    weigh_fsm_t fsm;

    /* Giao diện */
    ui_mode_t   ui;
    num_input_t input;
    int32_t     cal_ref_g;
    int32_t     cal_offset_new;
    float       cal_factor_new;
    char        msg[DISP_COLS + 1];
    uint16_t    msg_color;
    uint32_t    msg_until_ms;

    indicate_t  ind_state;
    bool        d_armed;            /**< Phím D được nhấn khi đang ở màn hình cân */
} app_ctx_t;

/**
 * @brief  Khởi tạo logic.
 * @param  loaded Cấu hình đọc từ flash, NULL hoặc không hợp lệ -> dùng mặc định.
 */
void app_init(app_ctx_t *ctx, const app_hooks_t *hooks, const app_config_t *loaded, uint32_t now_ms);

/** @brief Xử lý một số đo mới từ SensorTask. */
void app_on_measurement(app_ctx_t *ctx, const Measurement_t *m, uint32_t now_ms);

/** @brief Xử lý một sự kiện phím từ KeypadTask. */
void app_on_key(app_ctx_t *ctx, const KeyEvent_t *k, uint32_t now_ms);

/**
 * @brief  Gọi định kỳ (mỗi vòng MainTask): phát hiện mất cảm biến, cập nhật đèn.
 */
void app_tick(app_ctx_t *ctx, uint32_t now_ms);

/** @brief true nếu đang lỗi (chưa có số đo, mất HX711, bão hòa hoặc quá tải). */
bool app_fault(const app_ctx_t *ctx);

/** @brief Dựng nội dung màn hình theo trạng thái hiện tại. */
void app_render(const app_ctx_t *ctx, DisplaySnapshot_t *out);

#endif /* APP_LOGIC_H */
