/**
 * @file    flash_store.c
 * @brief   Cài đặt lưu cấu hình vào flash nội bằng HAL.
 */
#include "flash_store.h"

#include <string.h>
#include "main.h"

/* Ký hiệu từ linker script: cuối vùng code + dữ liệu khởi tạo trong flash. */
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;

/** @brief Kiểm tra firmware không lấn sang sector dùng để lưu cấu hình. */
static bool firmware_fits(void)
{
    uint32_t end = (uint32_t)&_sidata + ((uint32_t)&_edata - (uint32_t)&_sdata);
    return end <= FLASH_STORE_ADDR;
}

/* Mô tả đầy đủ: xem flash_store.h. */
bool flash_store_load(app_config_t *cfg)
{
    (void)memcpy(cfg, (const void *)FLASH_STORE_ADDR, sizeof(*cfg));
    return config_is_valid(cfg);
}

/* Mô tả đầy đủ: xem flash_store.h. */
bool flash_store_save(const app_config_t *cfg)
{
    FLASH_EraseInitTypeDef erase;
    uint32_t sector_error = 0U;
    const uint32_t *words = (const uint32_t *)cfg;
    uint32_t n_words = (sizeof(*cfg) + 3U) / 4U;
    uint32_t i;
    bool ok = true;

    // ! Không bao giờ xóa sector đang chứa chương trình.
    if (!firmware_fits())
    {
        return false;
    }

    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        return false;
    }
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector = FLASH_STORE_SECTOR;
    erase.NbSectors = 1U;
    if (HAL_FLASHEx_Erase(&erase, &sector_error) != HAL_OK)
    {
        ok = false;
    }

    for (i = 0U; ok && (i < n_words); i++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_STORE_ADDR + (i * 4U), words[i]) != HAL_OK)
        {
            ok = false;
        }
    }
    (void)HAL_FLASH_Lock();

    /* Đọc lại để chắc chắn dữ liệu trong flash đúng như bản trong RAM. */
    if (ok && (memcmp((const void *)FLASH_STORE_ADDR, cfg, sizeof(*cfg)) != 0))
    {
        ok = false;
    }
    return ok;
}
