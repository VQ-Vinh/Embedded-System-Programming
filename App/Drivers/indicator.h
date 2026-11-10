/**
 * @file    indicator.h
 * @brief   Điều khiển LED xanh, LED đỏ và còi báo.
 *
 * Chỉ MainTask gọi. indicator_tick() phải được gọi đều (<= 50 ms) để tắt
 * còi đúng thời gian và nhấp nháy LED đỏ khi lỗi.
 */
#ifndef INDICATOR_H
#define INDICATOR_H

#include <stdbool.h>
#include <stdint.h>
#include "app_logic.h"

/** @brief Tắt hết đèn và còi. */
void indicator_init(void);

/**
 * @brief  Đặt trạng thái đèn.
 * @param  what  IND_OFF / IND_PASS / IND_FAIL / IND_FAULT.
 * @param  beep  true -> kêu còi BUZZER_BEEP_MS.
 */
void indicator_set(indicate_t what, bool beep, uint32_t now_ms);

/** @brief Cập nhật nhấp nháy và tắt còi theo thời gian. */
void indicator_tick(uint32_t now_ms);

#endif /* INDICATOR_H */
