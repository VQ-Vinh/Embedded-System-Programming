/**
 * @file    config.c
 * @brief   Giá trị mặc định, CRC và kiểm tra hợp lệ cho app_config_t.
 */
#include "config.h"

#include <stddef.h>
#include <string.h>
#include "app_config.h"

/* Mô tả đầy đủ: xem config.h. */
uint32_t config_crc32(const void *data, uint32_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    uint32_t crc = 0xFFFFFFFFUL;
    uint32_t i;
    uint8_t b;

    for (i = 0U; i < len; i++)
    {
        crc ^= p[i];
        for (b = 0U; b < 8U; b++)
        {
            crc = (crc & 1U) ? ((crc >> 1) ^ 0xEDB88320UL) : (crc >> 1);
        }
    }
    return ~crc;
}

/* Mô tả đầy đủ: xem config.h. */
void config_seal(app_config_t *cfg)
{
    cfg->magic = CONFIG_MAGIC;
    cfg->version = CONFIG_VERSION;
    cfg->size = (uint16_t)sizeof(app_config_t);
    cfg->crc = config_crc32(cfg, (uint32_t)offsetof(app_config_t, crc));
}

/* Mô tả đầy đủ: xem config.h. */
void config_defaults(app_config_t *cfg)
{
    (void)memset(cfg, 0, sizeof(*cfg));
    cfg->target_x10 = DEFAULT_TARGET_X10;
    cfg->tol_x10 = DEFAULT_TOL_X10;
    cfg->cal_offset = DEFAULT_CAL_OFFSET;
    cfg->cal_factor = DEFAULT_CAL_FACTOR;
    config_seal(cfg);
}

/* Mô tả đầy đủ: xem config.h. */
bool config_is_valid(const app_config_t *cfg)
{
    float f = cfg->cal_factor;

    if ((cfg->magic != CONFIG_MAGIC) || (cfg->version != CONFIG_VERSION) ||
        (cfg->size != (uint16_t)sizeof(app_config_t)))
    {
        return false;
    }
    if (cfg->crc != config_crc32(cfg, (uint32_t)offsetof(app_config_t, crc)))
    {
        return false;
    }
    if ((cfg->target_x10 < (TARGET_MIN_G * 10)) || (cfg->target_x10 > (TARGET_MAX_G * 10)) ||
        (cfg->tol_x10 < (TOL_MIN_G * 10)) || (cfg->tol_x10 > (TOL_MAX_G * 10)))
    {
        return false;
    }
    /* f != f bắt trường hợp NaN khi flash chứa rác. */
    if ((f != f) || ((f < 1.0f) && (f > -1.0f)) || (f > 1.0e7f) || (f < -1.0e7f))
    {
        return false;
    }
    return true;
}
