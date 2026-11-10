/**
 * @file    flash_store.h
 * @brief   Lưu / đọc app_config_t ở sector 11 của flash (0x080E0000, 128 KB).
 *
 * STM32F407 không có EEPROM nên dành riêng sector cuối cho cấu hình.
 * Chỉ ghi khi người dùng xác nhận thay đổi (không ghi theo từng mẫu cân).
 *
 * ! Xóa sector mất ~1-2 s và CPU dừng đọc flash trong lúc đó: mọi task và
 * ! ngắt bị trễ. MainTask chỉ gọi flash_store_save() khi không có lượt cân.
 */
#ifndef FLASH_STORE_H
#define FLASH_STORE_H

#include <stdbool.h>
#include "config.h"

#define FLASH_STORE_ADDR    0x080E0000UL
#define FLASH_STORE_SECTOR  FLASH_SECTOR_11

/**
 * @brief  Sao chép cấu hình đang lưu trong flash ra cfg.
 * @return true nếu dữ liệu hợp lệ (magic, CRC, phạm vi).
 */
bool flash_store_load(app_config_t *cfg);

/**
 * @brief  Xóa sector 11 rồi ghi cfg, sau đó đọc lại để kiểm tra.
 * @return true nếu ghi và kiểm tra thành công.
 */
bool flash_store_save(const app_config_t *cfg);

#endif /* FLASH_STORE_H */
