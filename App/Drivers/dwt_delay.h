/**
 * @file    dwt_delay.h
 * @brief   Trễ micro giây bằng bộ đếm chu kỳ DWT của Cortex-M4.
 *
 * Dùng cho các giao thức bit-bang cần thời gian chính xác (HX711, quét
 * keypad). Không dùng timer phần cứng nên không chiếm tài nguyên CubeMX.
 */
#ifndef DWT_DELAY_H
#define DWT_DELAY_H

#include <stdint.h>

/** @brief Bật bộ đếm chu kỳ DWT. Gọi một lần trước khi dùng dwt_delay_us(). */
void dwt_delay_init(void);

/**
 * @brief  Chờ bận trong us micro giây.
 * @note   Không nhường CPU; chỉ dùng cho khoảng chờ rất ngắn (< 100 us).
 */
void dwt_delay_us(uint32_t us);

#endif /* DWT_DELAY_H */
