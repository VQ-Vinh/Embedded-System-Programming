/**
 * @file    task_display.c
 * @brief   DisplayTask: nhận DisplaySnapshot_t và vẽ lên TFT.
 *
 * Người phụ trách: Người 2.
 *
 * Chỉ vẽ lại phần thay đổi so với lần trước để màn hình không nhấp nháy và
 * SPI không bận liên tục. Bố cục (đơn vị: hàng chữ cao TFT_CHAR_H):
 *   hàng 0      : thanh tiêu đề
 *   hàng 1-3    : số cân cỡ lớn (font x3)
 *   hàng 4-13   : 10 dòng chữ nhỏ
 */
#include "app_main.h"

#include <stdbool.h>
#include <string.h>
#include "app_config.h"
#include "app_types.h"
#include "tft.h"

#define BIG_SCALE      (3U * TFT_TEXT_SCALE)
#define BIG_CHAR_W     (6U * BIG_SCALE)
#define BIG_CELLS      ((uint8_t)(TFT_WIDTH / BIG_CHAR_W))
#define ROW_Y(r)       ((uint16_t)((r) * TFT_CHAR_H))

static DisplaySnapshot_t s_cur;
static DisplaySnapshot_t s_prev;

/** @brief Vẽ thanh tiêu đề tràn hết chiều rộng màn hình. */
static void draw_title(const DisplaySnapshot_t *s)
{
    tft_draw_text(0U, ROW_Y(0U), s->title, TFT_TEXT_SCALE, COLOR_WHITE, s->title_bg, DISP_COLS);
    /* Tô nốt phần lề phải cho hết chiều rộng màn hình. */
    tft_fill_rect((uint16_t)(DISP_COLS * TFT_CHAR_W), ROW_Y(0U),
                  (uint16_t)(TFT_WIDTH - (DISP_COLS * TFT_CHAR_W)), TFT_CHAR_H, s->title_bg);
}

/** @brief Vẽ số cân cỡ lớn, căn giữa bằng khoảng trắng để phủ kín vùng cũ. */
static void draw_big(const DisplaySnapshot_t *s)
{
    char cell[DISP_BIG_LEN + 1U];
    size_t len = strlen(s->big);
    size_t pad;
    size_t i;
    uint16_t x0 = (uint16_t)((TFT_WIDTH - (BIG_CELLS * BIG_CHAR_W)) / 2U);

    if (len > BIG_CELLS)
    {
        len = BIG_CELLS;
    }
    /* Căn giữa bằng khoảng trắng để mỗi lần vẽ phủ kín cả vùng. */
    pad = (BIG_CELLS - len) / 2U;
    for (i = 0U; i < pad; i++)
    {
        cell[i] = ' ';
    }
    (void)memcpy(&cell[pad], s->big, len);
    cell[pad + len] = '\0';
    tft_draw_text(x0, ROW_Y(1U), cell, (uint8_t)BIG_SCALE, s->big_fg, COLOR_BLACK, BIG_CELLS);
}

/** @brief Vẽ một dòng chữ nhỏ, tô nền phần thừa để xóa ký tự cũ. */
static void draw_line(const DisplaySnapshot_t *s, uint8_t i)
{
    tft_draw_text(0U, ROW_Y(4U + i), s->line[i], TFT_TEXT_SCALE, s->line_fg[i], COLOR_BLACK, DISP_COLS);
}

/** @brief Thân DisplayTask: khởi tạo TFT và vẽ phần thay đổi của bản chụp. Không bao giờ trả về. */
void DisplayTask_Run(void *argument)
{
    bool first = true;
    uint8_t i;

    (void)argument;
    tft_init();
    tft_draw_text(0U, ROW_Y(0U), " CAN KIEM TRA STM32F4", TFT_TEXT_SCALE, COLOR_WHITE, COLOR_DARKBLUE, DISP_COLS);

    for (;;)
    {
        if (osMessageQueueGet(qDisplayHandle, &s_cur, NULL, osWaitForever) == osOK)
        {
            if (first || (strcmp(s_cur.title, s_prev.title) != 0) || (s_cur.title_bg != s_prev.title_bg))
            {
                draw_title(&s_cur);
            }
            if (first || (strcmp(s_cur.big, s_prev.big) != 0) || (s_cur.big_fg != s_prev.big_fg))
            {
                draw_big(&s_cur);
            }
            for (i = 0U; i < DISP_TEXT_ROWS; i++)
            {
                if (first || (strcmp(s_cur.line[i], s_prev.line[i]) != 0) ||
                    (s_cur.line_fg[i] != s_prev.line_fg[i]))
                {
                    draw_line(&s_cur, i);
                }
            }
            s_prev = s_cur;
            first = false;
        }
        osDelay(DISPLAY_PERIOD_MS);
    }
}
