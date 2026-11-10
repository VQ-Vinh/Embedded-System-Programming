/**
 * @file    scale_filter.c
 * @brief   Cài đặt bộ lọc trung bình trượt, đổi đơn vị và xét ổn định.
 */
#include "scale_filter.h"

/* Mô tả đầy đủ: xem scale_filter.h. */
void mavg_reset(mavg_t *f)
{
    f->sum = 0;
    f->idx = 0U;
    f->count = 0U;
}

/* Mô tả đầy đủ: xem scale_filter.h. */
int32_t mavg_push(mavg_t *f, int32_t sample)
{
    if (f->count == FILTER_WINDOW)
    {
        f->sum -= f->buf[f->idx];
    }
    else
    {
        f->count++;
    }
    f->buf[f->idx] = sample;
    f->sum += sample;
    f->idx = (uint8_t)((f->idx + 1U) % FILTER_WINDOW);
    return (int32_t)(f->sum / (int64_t)f->count);
}

/* Mô tả đầy đủ: xem scale_filter.h. */
int32_t scale_raw_to_x10(int32_t raw, int32_t offset, float factor)
{
    float grams_x10;

    // ! Hệ số quá nhỏ = chưa hiệu chuẩn đúng; trả 0 thay vì chia cho ~0.
    if ((factor < 1.0f) && (factor > -1.0f))
    {
        return 0;
    }
    grams_x10 = ((float)(raw - offset) * 10.0f) / factor;
    return (int32_t)((grams_x10 >= 0.0f) ? (grams_x10 + 0.5f) : (grams_x10 - 0.5f));
}

/* Mô tả đầy đủ: xem scale_filter.h. */
void stab_reset(stab_t *s)
{
    s->idx = 0U;
    s->count = 0U;
    s->hold = 0U;
    s->stable = false;
}

/* Mô tả đầy đủ: xem scale_filter.h. */
int32_t stab_span(const stab_t *s)
{
    int32_t mn;
    int32_t mx;
    uint8_t i;

    if (s->count == 0U)
    {
        return 0;
    }
    mn = s->win[0];
    mx = s->win[0];
    for (i = 1U; i < s->count; i++)
    {
        if (s->win[i] < mn) { mn = s->win[i]; }
        if (s->win[i] > mx) { mx = s->win[i]; }
    }
    return mx - mn;
}

/* Mô tả đầy đủ: xem scale_filter.h. */
bool stab_push(stab_t *s, int32_t value_x10)
{
    s->win[s->idx] = value_x10;
    s->idx = (uint8_t)((s->idx + 1U) % STABLE_WINDOW);
    if (s->count < STABLE_WINDOW)
    {
        s->count++;
    }

    /* * Chỉ xét khi cửa sổ đã đầy, rồi yêu cầu điều kiện đúng liên tục
     *   STABLE_HOLD_SAMPLES lần để tránh báo ổn định khi số đo vừa chững lại. */
    if ((s->count == STABLE_WINDOW) && (stab_span(s) <= STABLE_BAND_X10))
    {
        if (s->hold < STABLE_HOLD_SAMPLES)
        {
            s->hold++;
        }
    }
    else
    {
        s->hold = 0U;
    }
    s->stable = (s->hold >= STABLE_HOLD_SAMPLES);
    return s->stable;
}
