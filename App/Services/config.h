/**
 * @file    config.h
 * @brief   Cấu hình cần lưu khi mất điện: mục tiêu, sai số, hệ số hiệu chuẩn.
 *
 * Struct được ghi nguyên khối vào flash (xem flash_store.c). Có magic,
 * version và CRC32 để phát hiện vùng flash trống hoặc dữ liệu hỏng; khi đó
 * dùng giá trị mặc định trong app_config.h.
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#define CONFIG_MAGIC    0x43414E31UL  /* "CAN1" */
#define CONFIG_VERSION  1U

typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    int32_t  target_x10;    /**< Khối lượng mục tiêu, 0,1 g */
    int32_t  tol_x10;       /**< Sai số cho phép, 0,1 g */
    int32_t  cal_offset;    /**< Số đếm ADC khi không tải */
    float    cal_factor;    /**< Số đếm trên 1 gam */
    uint32_t crc;           /**< CRC32 của mọi trường phía trên */
} app_config_t;

/** @brief Ghi giá trị mặc định vào cfg (đã tính CRC). */
void config_defaults(app_config_t *cfg);

/** @brief Tính lại CRC sau khi sửa một trường. */
void config_seal(app_config_t *cfg);

/** @brief Kiểm tra magic, version, CRC và phạm vi từng trường. */
bool config_is_valid(const app_config_t *cfg);

/** @brief CRC32 (đa thức 0xEDB88320) của một vùng nhớ. */
uint32_t config_crc32(const void *data, uint32_t len);

#endif /* CONFIG_H */
