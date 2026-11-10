/**
 * @file    tft.c
 * @brief   Cài đặt driver TFT ST7735 / ILI9341 qua SPI2.
 */
#include "tft.h"

#include <stdbool.h>
#include "main.h"
#include "cmsis_os.h"
#include "font5x7.h"

extern SPI_HandleTypeDef hspi2;

/* Lệnh chung của hai controller. */
#define CMD_SWRESET 0x01U
#define CMD_SLPOUT  0x11U
#define CMD_NORON   0x13U
#define CMD_INVOFF  0x20U
#define CMD_DISPON  0x29U
#define CMD_CASET   0x2AU
#define CMD_RASET   0x2BU
#define CMD_RAMWR   0x2CU
#define CMD_MADCTL  0x36U
#define CMD_COLMOD  0x3AU

/* Bit MADCTL */
#define MADCTL_MY   0x80U
#define MADCTL_MX   0x40U
#define MADCTL_MV   0x20U
#define MADCTL_BGR  0x08U

#define DELAY_FLAG  0x80U   /* Trong bảng init: byte số tham số có bit này -> có byte trễ (ms) theo sau */

/* Bộ đệm một ô ký tự lớn nhất (cỡ chữ to = 3 x TFT_TEXT_SCALE). */
#define MAX_SCALE   (3U * TFT_TEXT_SCALE)
static uint8_t s_buf[(6U * MAX_SCALE) * (8U * MAX_SCALE) * 2U];

/* ------------------------------------------------------------------------ */
/*                          Bảng lệnh khởi tạo                              */
/* Định dạng: lệnh, số tham số (| DELAY_FLAG), tham số..., [trễ ms]; kết thúc 0x00, 0x00 */
/* ------------------------------------------------------------------------ */

#if (TFT_CONTROLLER == TFT_CONTROLLER_ILI9341)
#define MADCTL_LANDSCAPE ((TFT_FLIP ? (MADCTL_MX | MADCTL_MY | MADCTL_MV) : MADCTL_MV) | (TFT_BGR ? MADCTL_BGR : 0U))
static const uint8_t INIT_SEQ[] = {
    CMD_SWRESET, DELAY_FLAG, 150,
    0xEF, 3, 0x03, 0x80, 0x02,
    0xCF, 3, 0x00, 0xC1, 0x30,
    0xED, 4, 0x64, 0x03, 0x12, 0x81,
    0xE8, 3, 0x85, 0x00, 0x78,
    0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
    0xF7, 1, 0x20,
    0xEA, 2, 0x00, 0x00,
    0xC0, 1, 0x23,                  /* PWCTR1 */
    0xC1, 1, 0x10,                  /* PWCTR2 */
    0xC5, 2, 0x3E, 0x28,            /* VMCTR1 */
    0xC7, 1, 0x86,                  /* VMCTR2 */
    CMD_MADCTL, 1, MADCTL_LANDSCAPE,
    0x37, 1, 0x00,                  /* VSCRSADD */
    CMD_COLMOD, 1, 0x55,            /* 16 bit/pixel */
    0xB1, 2, 0x00, 0x18,            /* FRMCTR1 */
    0xB6, 3, 0x08, 0x82, 0x27,      /* DFUNCTR */
    0xF2, 1, 0x00,                  /* tắt 3 gamma */
    0x26, 1, 0x01,                  /* GAMMASET */
    0xE0, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
              0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
    0xE1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
              0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
    CMD_SLPOUT, DELAY_FLAG, 150,
    CMD_DISPON, DELAY_FLAG, 150,
    0x00, 0x00
};
#else
#define MADCTL_LANDSCAPE ((TFT_FLIP ? (MADCTL_MX | MADCTL_MV) : (MADCTL_MY | MADCTL_MV)) | (TFT_BGR ? MADCTL_BGR : 0U))
static const uint8_t INIT_SEQ[] = {
    CMD_SWRESET, DELAY_FLAG, 150,
    CMD_SLPOUT, DELAY_FLAG, 255,
    0xB1, 3, 0x01, 0x2C, 0x2D,                      /* FRMCTR1 */
    0xB2, 3, 0x01, 0x2C, 0x2D,                      /* FRMCTR2 */
    0xB3, 6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,    /* FRMCTR3 */
    0xB4, 1, 0x07,                                  /* INVCTR */
    0xC0, 3, 0xA2, 0x02, 0x84,                      /* PWCTR1 */
    0xC1, 1, 0xC5,                                  /* PWCTR2 */
    0xC2, 2, 0x0A, 0x00,                            /* PWCTR3 */
    0xC3, 2, 0x8A, 0x2A,                            /* PWCTR4 */
    0xC4, 2, 0x8A, 0xEE,                            /* PWCTR5 */
    0xC5, 1, 0x0E,                                  /* VMCTR1 */
    CMD_INVOFF, 0,
    CMD_MADCTL, 1, MADCTL_LANDSCAPE,
    CMD_COLMOD, 1, 0x05,                            /* 16 bit/pixel */
    0xE0, 16, 0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D,
              0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10,
    0xE1, 16, 0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
              0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10,
    CMD_NORON, DELAY_FLAG, 10,
    CMD_DISPON, DELAY_FLAG, 100,
    0x00, 0x00
};
#endif

/* ------------------------------------------------------------------------ */
/*                              Mức thấp SPI                                */
/* ------------------------------------------------------------------------ */

static inline void cs_low(void)  { HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_RESET); }
static inline void cs_high(void) { HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET); }
static inline void dc_cmd(void)  { HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET); }
static inline void dc_data(void) { HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET); }

/** @brief Gửi một khối byte qua SPI2 (chế độ chặn, timeout 100 ms). */
static void spi_write(const uint8_t *data, uint16_t len)
{
    (void)HAL_SPI_Transmit(&hspi2, (uint8_t *)data, len, 100U);
}

/** @brief Gửi một byte lệnh (DC thấp) rồi đưa DC lên cao cho dữ liệu kế tiếp. CS phải đang thấp. */
static void write_cmd(uint8_t cmd)
{
    dc_cmd();
    spi_write(&cmd, 1U);
    dc_data();
}

/** @brief Chạy bảng lệnh khởi tạo; byte trễ được thực hiện bằng osDelay để nhường CPU. */
static void run_init_sequence(const uint8_t *seq)
{
    uint8_t cmd;
    uint8_t n;

    for (;;)
    {
        cmd = *seq++;
        n = *seq++;
        if ((cmd == 0x00U) && (n == 0x00U))
        {
            break;
        }
        cs_low();
        write_cmd(cmd);
        if ((n & (uint8_t)~DELAY_FLAG) > 0U)
        {
            spi_write(seq, (uint16_t)(n & (uint8_t)~DELAY_FLAG));
            seq += (n & (uint8_t)~DELAY_FLAG);
        }
        cs_high();
        if ((n & DELAY_FLAG) != 0U)
        {
            osDelay(*seq++);
        }
    }
}

/** @brief Đặt cửa sổ ghi [x0..x1] x [y0..y1] và gửi lệnh RAMWR (CS vẫn thấp). */
static void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t d[4];

    x0 += TFT_X_OFFSET; x1 += TFT_X_OFFSET;
    y0 += TFT_Y_OFFSET; y1 += TFT_Y_OFFSET;

    write_cmd(CMD_CASET);
    d[0] = (uint8_t)(x0 >> 8); d[1] = (uint8_t)x0; d[2] = (uint8_t)(x1 >> 8); d[3] = (uint8_t)x1;
    spi_write(d, 4U);
    write_cmd(CMD_RASET);
    d[0] = (uint8_t)(y0 >> 8); d[1] = (uint8_t)y0; d[2] = (uint8_t)(y1 >> 8); d[3] = (uint8_t)y1;
    spi_write(d, 4U);
    write_cmd(CMD_RAMWR);
}

/* ------------------------------------------------------------------------ */
/*                              API công khai                               */
/* ------------------------------------------------------------------------ */

/* Mô tả đầy đủ: xem tft.h. */
void tft_init(void)
{
    cs_high();
    HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_SET);
    osDelay(5U);
    HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_RESET);
    osDelay(20U);
    HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_SET);
    osDelay(150U);

    run_init_sequence(INIT_SEQ);
    tft_fill_screen(0x0000U);
}

/* Mô tả đầy đủ: xem tft.h. */
void tft_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    uint32_t total;
    uint32_t chunk;
    uint32_t i;
    uint32_t cap = sizeof(s_buf) / 2U;

    if ((x >= TFT_WIDTH) || (y >= TFT_HEIGHT) || (w == 0U) || (h == 0U))
    {
        return;
    }
    if ((x + w) > TFT_WIDTH)  { w = (uint16_t)(TFT_WIDTH - x); }
    if ((y + h) > TFT_HEIGHT) { h = (uint16_t)(TFT_HEIGHT - y); }

    total = (uint32_t)w * h;
    chunk = (total < cap) ? total : cap;
    for (i = 0U; i < chunk; i++)
    {
        s_buf[2U * i] = (uint8_t)(color >> 8);
        s_buf[(2U * i) + 1U] = (uint8_t)color;
    }

    cs_low();
    set_window(x, y, (uint16_t)(x + w - 1U), (uint16_t)(y + h - 1U));
    while (total > 0U)
    {
        uint32_t n = (total < chunk) ? total : chunk;
        spi_write(s_buf, (uint16_t)(n * 2U));
        total -= n;
    }
    cs_high();
}

/* Mô tả đầy đủ: xem tft.h. */
void tft_fill_screen(uint16_t color)
{
    tft_fill_rect(0U, 0U, TFT_WIDTH, TFT_HEIGHT, color);
}

/** @brief Vẽ một ô ký tự (6x8 nhân scale) gồm cả nền. */
static void draw_char(uint16_t x, uint16_t y, char ch, uint8_t scale, uint16_t fg, uint16_t bg)
{
    uint16_t cw = (uint16_t)(6U * scale);
    uint16_t chh = (uint16_t)(8U * scale);
    const uint8_t *glyph;
    uint16_t px;
    uint16_t py;
    uint32_t idx = 0U;
    uint8_t c = (uint8_t)ch;

    if ((scale == 0U) || (scale > MAX_SCALE) || ((x + cw) > TFT_WIDTH) || ((y + chh) > TFT_HEIGHT))
    {
        return;
    }
    if ((c < FONT5X7_FIRST) || (c > FONT5X7_LAST))
    {
        c = (uint8_t)'?';
    }
    glyph = font5x7[c - FONT5X7_FIRST];

    for (py = 0U; py < chh; py++)
    {
        uint8_t row = (uint8_t)(py / scale);
        for (px = 0U; px < cw; px++)
        {
            uint8_t col = (uint8_t)(px / scale);
            bool on = (col < 5U) && (row < 7U) && (((glyph[col] >> row) & 1U) != 0U);
            uint16_t color = on ? fg : bg;
            s_buf[idx++] = (uint8_t)(color >> 8);
            s_buf[idx++] = (uint8_t)color;
        }
    }

    cs_low();
    set_window(x, y, (uint16_t)(x + cw - 1U), (uint16_t)(y + chh - 1U));
    spi_write(s_buf, (uint16_t)idx);
    cs_high();
}

void tft_draw_text(uint16_t x, uint16_t y, const char *text, uint8_t scale,
                   uint16_t fg, uint16_t bg, uint8_t width_chars)
{
    uint8_t n = 0U;
    uint16_t cw = (uint16_t)(6U * scale);

    while (*text != '\0')
    {
        if ((width_chars > 0U) && (n >= width_chars))
        {
            break;
        }
        draw_char(x, y, *text++, scale, fg, bg);
        x = (uint16_t)(x + cw);
        n++;
    }
    /* Xóa phần còn lại của vùng để không sót ký tự cũ. */
    if ((width_chars > n) && (x < TFT_WIDTH))
    {
        tft_fill_rect(x, y, (uint16_t)((width_chars - n) * cw), (uint16_t)(8U * scale), bg);
    }
}
