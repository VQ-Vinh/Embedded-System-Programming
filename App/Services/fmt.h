/**
 * @file    fmt.h
 * @brief   Hàm định dạng chuỗi nhỏ gọn, không dùng printf.
 *
 * Tránh snprintf để không phải bật in số thực và không lo vấn đề reentrant
 * của newlib khi nhiều task cùng định dạng chuỗi. Mọi hàm ghi vào bộ đệm
 * do người gọi cấp và luôn kết thúc bằng '\0'.
 */
#ifndef FMT_H
#define FMT_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief  Ghi số nguyên có dấu dạng thập phân.
 * @param  buf  Bộ đệm đích (tối thiểu 12 byte).
 * @param  v    Giá trị cần ghi.
 * @return Số ký tự đã ghi (không tính '\0').
 */
size_t fmt_i32(char *buf, int32_t v);

/**
 * @brief  Ghi giá trị 0,1 đơn vị dạng "x.y", ví dụ 2502 -> "250.2", -5 -> "-0.5".
 * @param  buf  Bộ đệm đích (tối thiểu 13 byte).
 * @param  v_x10 Giá trị nhân 10.
 * @return Số ký tự đã ghi.
 */
size_t fmt_x10(char *buf, int32_t v_x10);

/**
 * @brief  Nối chuỗi src vào cuối dst, không vượt quá cap byte (kể cả '\0').
 * @return Độ dài mới của dst.
 */
size_t fmt_cat(char *dst, size_t cap, const char *src);

/** @brief Nối số nguyên vào cuối dst. */
size_t fmt_cat_i32(char *dst, size_t cap, int32_t v);

/** @brief Nối giá trị x10 (một chữ số thập phân) vào cuối dst. */
size_t fmt_cat_x10(char *dst, size_t cap, int32_t v_x10);

/**
 * @brief  Gán dst = src (cắt bớt nếu dài hơn cap - 1).
 */
void fmt_copy(char *dst, size_t cap, const char *src);

#endif /* FMT_H */
