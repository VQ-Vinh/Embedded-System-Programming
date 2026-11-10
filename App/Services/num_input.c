/**
 * @file    num_input.c
 * @brief   Cài đặt bộ soạn số nguyên từ keypad.
 */
#include "num_input.h"

#include "fmt.h"

/* Mô tả đầy đủ: xem num_input.h. */
void num_input_begin(num_input_t *n, uint8_t max_len, int32_t min, int32_t max)
{
    n->len = 0U;
    n->digits[0] = '\0';
    n->max_len = (max_len > NUM_INPUT_MAX_DIGITS) ? NUM_INPUT_MAX_DIGITS : max_len;
    n->min = min;
    n->max = max;
}

/* Mô tả đầy đủ: xem num_input.h. */
void num_input_preset(num_input_t *n, int32_t value)
{
    char tmp[12];
    size_t len = fmt_i32(tmp, value);

    if ((value < 0) || (len > n->max_len))
    {
        return;
    }
    fmt_copy(n->digits, sizeof(n->digits), tmp);
    n->len = (uint8_t)len;
}

/* Mô tả đầy đủ: xem num_input.h. */
bool num_input_key(num_input_t *n, char key)
{
    if ((key >= '0') && (key <= '9'))
    {
        /* Bỏ số 0 đứng đầu để "0500" không chiếm chỗ. */
        if ((n->len == 1U) && (n->digits[0] == '0'))
        {
            n->len = 0U;
        }
        if (n->len < n->max_len)
        {
            n->digits[n->len++] = key;
            n->digits[n->len] = '\0';
        }
        return true;
    }
    if (key == '*')
    {
        if (n->len > 0U)
        {
            n->len--;
            n->digits[n->len] = '\0';
        }
        return true;
    }
    return false;
}

/* Mô tả đầy đủ: xem num_input.h. */
int num_input_value(const num_input_t *n, int32_t *out)
{
    int32_t v = 0;
    uint8_t i;

    if (n->len == 0U)
    {
        return -1;
    }
    for (i = 0U; i < n->len; i++)
    {
        v = (v * 10) + (int32_t)(n->digits[i] - '0');
    }
    if ((v < n->min) || (v > n->max))
    {
        return -2;
    }
    *out = v;
    return 0;
}
