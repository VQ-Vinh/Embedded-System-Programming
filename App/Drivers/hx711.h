/**
 * @file    hx711.h
 * @brief   Driver bit-bang cho ADC 24 bit HX711 (kênh A, gain 128).
 *
 * Chân: HX711_DOUT (input) và HX711_SCK (output) do CubeMX cấu hình,
 * tên macro sinh trong main.h. Chỉ SensorTask được gọi driver này.
 *
 * Ràng buộc thời gian:
 * - Mỗi xung SCK mức cao phải < 60 us, nếu không HX711 vào chế độ ngủ.
 *   Vì vậy 25 xung được phát trong vùng cấm ngắt (~55 us).
 * - Ở 10 SPS (chân RATE nối GND trên đa số module) có mẫu mới mỗi ~100 ms.
 */
#ifndef HX711_H
#define HX711_H

#include <stdbool.h>
#include <stdint.h>

#define HX711_RAW_MAX  ((int32_t)0x7FFFFF)
#define HX711_RAW_MIN  ((int32_t)-0x800000)

/** @brief Đưa SCK về thấp (đánh thức chip) và khởi động bộ trễ DWT. */
void hx711_init(void);

/** @brief true nếu HX711 đã có mẫu mới (DOUT ở mức thấp). */
bool hx711_is_ready(void);

/**
 * @brief  Đọc một mẫu 24 bit có dấu và chọn lại kênh A gain 128.
 * @pre    hx711_is_ready() == true.
 * @return Giá trị trong [HX711_RAW_MIN, HX711_RAW_MAX].
 */
int32_t hx711_read_raw(void);

/** @brief Tắt nguồn HX711 (SCK cao > 60 us) rồi bật lại để reset chip. */
void hx711_power_cycle(void);

#endif /* HX711_H */
