/**
 * @file    fmt.c
 * @brief   Cài đặt các hàm định dạng chuỗi trong fmt.h.
 */
#include "fmt.h"

/* Mô tả đầy đủ: xem fmt.h. */
size_t fmt_i32(char *buf, int32_t v)
{
    char tmp[12];
    size_t n = 0U;
    size_t len = 0U;
    /* * Dùng uint32_t để đổi dấu an toàn cả với INT32_MIN. */
    uint32_t u = (v < 0) ? (0U - (uint32_t)v) : (uint32_t)v;

    do
    {
        tmp[n++] = (char)('0' + (u % 10U));
        u /= 10U;
    } while (u != 0U);

    if (v < 0)
    {
        buf[len++] = '-';
    }
    while (n > 0U)
    {
        buf[len++] = tmp[--n];
    }
    buf[len] = '\0';
    return len;
}

/* Mô tả đầy đủ: xem fmt.h. */
size_t fmt_x10(char *buf, int32_t v_x10)
{
    size_t len = 0U;
    uint32_t u = (v_x10 < 0) ? (0U - (uint32_t)v_x10) : (uint32_t)v_x10;

    if (v_x10 < 0)
    {
        buf[len++] = '-';
    }
    len += fmt_i32(&buf[len], (int32_t)(u / 10U));
    buf[len++] = '.';
    buf[len++] = (char)('0' + (u % 10U));
    buf[len] = '\0';
    return len;
}

/* Mô tả đầy đủ: xem fmt.h. */
size_t fmt_cat(char *dst, size_t cap, const char *src)
{
    size_t len = 0U;

    if (cap == 0U)
    {
        return 0U;
    }
    while ((len < cap) && (dst[len] != '\0'))
    {
        len++;
    }
    while ((len + 1U < cap) && (*src != '\0'))
    {
        dst[len++] = *src++;
    }
    if (len < cap)
    {
        dst[len] = '\0';
    }
    return len;
}

/* Mô tả đầy đủ: xem fmt.h. */
size_t fmt_cat_i32(char *dst, size_t cap, int32_t v)
{
    char tmp[12];
    (void)fmt_i32(tmp, v);
    return fmt_cat(dst, cap, tmp);
}

/* Mô tả đầy đủ: xem fmt.h. */
size_t fmt_cat_x10(char *dst, size_t cap, int32_t v_x10)
{
    char tmp[14];
    (void)fmt_x10(tmp, v_x10);
    return fmt_cat(dst, cap, tmp);
}

/* Mô tả đầy đủ: xem fmt.h. */
void fmt_copy(char *dst, size_t cap, const char *src)
{
    if (cap == 0U)
    {
        return;
    }
    dst[0] = '\0';
    (void)fmt_cat(dst, cap, src);
}
