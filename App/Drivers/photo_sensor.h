/**
 * @file    photo_sensor.h
 * @brief   Đọc cảm biến quang phát hiện túi, có chống dội.
 *
 * Mức tích cực đặt bằng PHOTO_ACTIVE_LEVEL trong app_config.h. Chỉ
 * SensorTask gọi; photo_sensor_update() phải được gọi đều đặn (mỗi
 * HX711_POLL_MS) để bộ chống dội hoạt động.
 */
#ifndef PHOTO_SENSOR_H
#define PHOTO_SENSOR_H

#include <stdbool.h>

/** @brief Đọc mức hiện tại và cập nhật bộ chống dội. */
void photo_sensor_update(void);

/** @brief Trạng thái đã chống dội: true = đang có vật chắn tia. */
bool photo_sensor_present(void);

#endif /* PHOTO_SENSOR_H */
