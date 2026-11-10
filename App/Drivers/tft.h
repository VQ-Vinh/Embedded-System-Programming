/**
 * @file    tft.h
 * @brief   Driver màn hình TFT SPI (ST7735 1,8" hoặc ILI9341 2,4-2,8").
 *
 * Chọn loại bằng TFT_CONTROLLER trong app_config.h. Màn hình luôn đặt nằm
 * ngang (landscape). Văn bản dùng font 5x7 phóng to nguyên lần.
 *
 * Bố cục theo "hàng chữ" dùng chung cho cả hai loại màn hình:
 *   - Cỡ chữ thường TFT_TEXT_SCALE: ST7735 = 1 (6x8 px), ILI9341 = 2 (12x16 px)
 *   - 26 cột x 15-16 hàng
 *
 * Phụ thuộc: hspi2 (CubeMX), các chân TFT_CS/DC/RST trong main.h, osDelay.
 * Chỉ DisplayTask được gọi driver này (không cần mutex SPI).
 */
#ifndef TFT_H
#define TFT_H

#include <stdint.h>
#include "app_config.h"

#if (TFT_CONTROLLER == TFT_CONTROLLER_ILI9341)
#define TFT_WIDTH       320U
#define TFT_HEIGHT      240U
#define TFT_TEXT_SCALE  2U
#else
#define TFT_WIDTH       160U
#define TFT_HEIGHT      128U
#define TFT_TEXT_SCALE  1U
#endif

#define TFT_CHAR_W      (6U * TFT_TEXT_SCALE)   /**< Ô ký tự rộng (5 px + 1 px cách) */
#define TFT_CHAR_H      (8U * TFT_TEXT_SCALE)   /**< Ô ký tự cao (7 px + 1 px cách) */

/** @brief Reset phần cứng và gửi chuỗi lệnh khởi tạo, xóa màn hình đen. */
void tft_init(void);

/** @brief Tô một hình chữ nhật một màu. Phần vượt màn hình bị cắt. */
void tft_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/** @brief Tô toàn màn hình. */
void tft_fill_screen(uint16_t color);

/**
 * @brief  Vẽ chuỗi ASCII tại (x, y) với hệ số phóng to scale.
 * @param  width_chars Nếu > 0: tô nền đủ width_chars ô để xóa ký tự cũ.
 */
void tft_draw_text(uint16_t x, uint16_t y, const char *text, uint8_t scale,
                   uint16_t fg, uint16_t bg, uint8_t width_chars);

#endif /* TFT_H */
