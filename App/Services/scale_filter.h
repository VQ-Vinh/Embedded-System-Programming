/**
 * @file    scale_filter.h
 * @brief   Lọc số đo, đổi số đếm ADC sang gam và phát hiện cân ổn định.
 *
 * - mavg_*   : trung bình trượt trên số đếm thô của HX711 (SensorTask dùng).
 * - scale_*  : đổi số đếm sang 0,1 g theo hệ số hiệu chuẩn (MainTask dùng).
 * - stab_*   : xét ổn định khi độ dao động trong cửa sổ dưới ngưỡng liên tục
 *              trong một số mẫu (MainTask dùng).
 *
 * Module thuần logic, không gọi HAL/FreeRTOS nên kiểm thử được trên máy tính.
 */
#ifndef SCALE_FILTER_H
#define SCALE_FILTER_H

#include <stdbool.h>
#include <stdint.h>
#include "app_config.h"

/* ---------- Trung bình trượt ---------- */

typedef struct
{
    int32_t buf[FILTER_WINDOW];
    int64_t sum;
    uint8_t idx;
    uint8_t count;
} mavg_t;

/** @brief Xóa toàn bộ mẫu cũ. */
void mavg_reset(mavg_t *f);

/**
 * @brief  Thêm một mẫu và trả về trung bình hiện tại.
 * @note   Khi chưa đủ FILTER_WINDOW mẫu, trung bình tính trên số mẫu đang có.
 */
int32_t mavg_push(mavg_t *f, int32_t sample);

/* ---------- Đổi đơn vị ---------- */

/**
 * @brief  Đổi số đếm ADC sang khối lượng 0,1 g.
 * @param  raw        Số đếm sau lọc.
 * @param  offset     Số đếm khi không tải (từ hiệu chuẩn).
 * @param  factor     Số đếm trên 1 gam (có thể âm nếu load cell nối ngược).
 * @return Khối lượng 0,1 g, làm tròn gần nhất; 0 nếu factor không hợp lệ.
 */
int32_t scale_raw_to_x10(int32_t raw, int32_t offset, float factor);

/* ---------- Phát hiện ổn định ---------- */

typedef struct
{
    int32_t win[STABLE_WINDOW];
    uint8_t idx;
    uint8_t count;
    uint8_t hold;       /**< Số lần liên tiếp cửa sổ nằm trong ngưỡng */
    bool    stable;
} stab_t;

/** @brief Xóa trạng thái ổn định (gọi khi đổi hiệu chuẩn hoặc lỗi cảm biến). */
void stab_reset(stab_t *s);

/**
 * @brief  Đưa một giá trị khối lượng mới vào bộ xét ổn định.
 * @param  value_x10 Khối lượng 0,1 g.
 * @return true nếu cân đang ổn định.
 */
bool stab_push(stab_t *s, int32_t value_x10);

/** @brief Biên độ dao động (max - min) trong cửa sổ hiện tại, 0,1 g. */
int32_t stab_span(const stab_t *s);

#endif /* SCALE_FILTER_H */
