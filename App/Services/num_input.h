/**
 * @file    num_input.h
 * @brief   Bộ soạn số nguyên từ keypad: nhập chữ số, xóa bằng '*', kiểm tra phạm vi.
 */
#ifndef NUM_INPUT_H
#define NUM_INPUT_H

#include <stdbool.h>
#include <stdint.h>

#define NUM_INPUT_MAX_DIGITS 4

typedef struct
{
    char    digits[NUM_INPUT_MAX_DIGITS + 1];
    uint8_t len;
    uint8_t max_len;
    int32_t min;
    int32_t max;
} num_input_t;

/**
 * @brief  Bắt đầu phiên nhập mới.
 * @param  max_len Số chữ số tối đa (<= NUM_INPUT_MAX_DIGITS).
 * @param  min,max Phạm vi hợp lệ (đơn vị gam nguyên).
 */
void num_input_begin(num_input_t *n, uint8_t max_len, int32_t min, int32_t max);

/** @brief Nạp sẵn một giá trị (ví dụ giá trị hiện tại) để người dùng sửa. */
void num_input_preset(num_input_t *n, int32_t value);

/**
 * @brief  Xử lý phím chữ số hoặc '*'.
 * @return true nếu phím đã được dùng.
 */
bool num_input_key(num_input_t *n, char key);

/**
 * @brief  Lấy giá trị đã nhập.
 * @param  out Giá trị nếu hợp lệ.
 * @return 0 = hợp lệ, -1 = chưa nhập, -2 = ngoài phạm vi.
 */
int num_input_value(const num_input_t *n, int32_t *out);

#endif /* NUM_INPUT_H */
