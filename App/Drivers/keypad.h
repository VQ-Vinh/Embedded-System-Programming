/**
 * @file    keypad.h
 * @brief   Quét keypad ma trận 4x4: hàng là output open-drain, cột là input pull-up.
 *
 * ! Hàng để open-drain: nhấn 2 phím cùng cột không làm chập 2 chân output.
 *
 * Bố trí phím (nhìn từ mặt trước keypad):
 *     C1  C2  C3  C4
 * R1   1   2   3   A
 * R2   4   5   6   B
 * R3   7   8   9   C
 * R4   *   0   #   D
 *
 * Chống dội và phát hiện giữ phím do KeypadTask đảm nhiệm; driver này chỉ
 * trả về phím đang được nhấn tại thời điểm quét.
 */
#ifndef KEYPAD_H
#define KEYPAD_H

/** @brief Đưa tất cả hàng lên mức cao (không chọn hàng nào). */
void keypad_init(void);

/**
 * @brief  Quét toàn bộ ma trận một lần.
 * @return Ký tự phím đang nhấn, hoặc 0 nếu không có phím nào.
 *         Nếu nhấn nhiều phím cùng lúc, trả về phím gặp đầu tiên.
 */
char keypad_scan(void);

#endif /* KEYPAD_H */
