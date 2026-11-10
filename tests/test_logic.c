/**
 * @file    test_logic.c
 * @brief   Kiểm thử logic ứng dụng trên máy tính (không cần kit).
 *
 * Mô phỏng các tình huống ở mục 11 của đề tài: phân loại tại biên, giữ túi
 * lâu, chắn tia bằng tay, số đo dao động, lỗi cảm biến, quá tải, trừ bì,
 * nhập ngoài phạm vi, hiệu chuẩn, lưu cấu hình và đối chiếu thống kê với
 * số bản ghi đã gửi.
 *
 * Chạy: tests/run_tests.sh  (hoặc xem lệnh gcc trong file đó).
 */
#include <stdio.h>
#include <string.h>
#include "app_config.h"
#include "app_logic.h"
#include "fmt.h"

static int g_fail = 0;
static int g_pass = 0;
#define CHECK(cond) do { if (cond) { g_pass++; } else { g_fail++; \
    printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); } } while (0)

/* ---------- Hook giả lập ---------- */
static ResultRecord_t g_rec[64];
static int g_nrec = 0;
static app_config_t g_saved;
static int g_nsave = 0;
static indicate_t g_ind = IND_OFF;
static int g_beeps = 0;

static void h_emit(const ResultRecord_t *r) { if (g_nrec < 64) { g_rec[g_nrec++] = *r; } }
static bool h_save(const app_config_t *c) { g_saved = *c; g_nsave++; return true; }
static void h_ind(indicate_t w, bool beep) { g_ind = w; if (beep) { g_beeps++; } }
static const app_hooks_t HOOKS = { h_emit, h_save, h_ind };

/* ---------- Mô phỏng cân ---------- */
#define FACTOR 2000.0f
#define OFFSET 100000
static app_ctx_t ctx;
static uint32_t now = 0;

static int count_results(void)
{
    int i, n = 0;
    for (i = 0; i < g_nrec; i++) { if (g_rec[i].kind == REC_RESULT) { n++; } }
    return n;
}

static void reset_all(void)
{
    app_config_t c;
    config_defaults(&c);
    c.cal_offset = OFFSET;
    c.cal_factor = FACTOR;
    config_seal(&c);
    g_nrec = 0; g_nsave = 0; g_beeps = 0; g_ind = IND_OFF;
    now = 0;
    app_init(&ctx, &HOOKS, &c, now);
}

/** Đưa vào n mẫu (mỗi mẫu 100 ms) với khối lượng gram x10 và trạng thái quang. */
static void feed(int n, int32_t grams_x10, int photo, int noise_x10)
{
    int i;
    Measurement_t m;
    for (i = 0; i < n; i++)
    {
        int32_t g = grams_x10 + ((i % 2) ? noise_x10 : -noise_x10);
        now += 100;
        m.raw_avg = OFFSET + (int32_t)((float)g * FACTOR / 10.0f);
        m.raw_last = m.raw_avg;
        m.photo = (uint8_t)photo;
        m.status = SENSOR_OK;
        m.tick_ms = now;
        app_on_measurement(&ctx, &m, now);
        app_tick(&ctx, now);
    }
}

static void feed_status(int n, uint8_t status, int photo)
{
    int i;
    Measurement_t m;
    for (i = 0; i < n; i++)
    {
        now += 100;
        memset(&m, 0, sizeof(m));
        m.status = status; m.photo = (uint8_t)photo; m.tick_ms = now;
        app_on_measurement(&ctx, &m, now);
        app_tick(&ctx, now);
    }
}

static void key(char k, uint8_t type)
{
    KeyEvent_t e = { k, type };
    now += 50;
    app_on_key(&ctx, &e, now);
}
static void hold_d(void) { key('D', KEY_EVT_PRESS); key('D', KEY_EVT_HOLD); }
static void keys(const char *s) { while (*s) { key(*s++, KEY_EVT_PRESS); } }

/** Một lượt cân hoàn chỉnh: đặt túi, chờ, lấy ra. */
static void one_bag(int32_t grams_x10)
{
    feed(30, grams_x10, 1, 0);
    feed(15, 0, 0, 0);
}

/* ---------- Các ca kiểm thử ---------- */

static void t_format(void)
{
    char b[16];
    printf("[fmt] dinh dang so\n");
    fmt_x10(b, 2502); CHECK(strcmp(b, "250.2") == 0);
    fmt_x10(b, -5);   CHECK(strcmp(b, "-0.5") == 0);
    fmt_x10(b, 0);    CHECK(strcmp(b, "0.0") == 0);
    fmt_i32(b, -2147483647 - 1); CHECK(strcmp(b, "-2147483648") == 0);
}

static void t_classify_boundaries(void)
{
    printf("[11.1] phan loai tai bien va gan bien (250 +/-5 g)\n");
    CHECK(weigh_classify(2449, 2500, 50) == RESULT_UNDER);
    CHECK(weigh_classify(2450, 2500, 50) == RESULT_PASS);
    CHECK(weigh_classify(2451, 2500, 50) == RESULT_PASS);
    CHECK(weigh_classify(2549, 2500, 50) == RESULT_PASS);
    CHECK(weigh_classify(2550, 2500, 50) == RESULT_PASS);
    CHECK(weigh_classify(2551, 2500, 50) == RESULT_OVER);

    reset_all();
    feed(20, 0, 0, 0);
    one_bag(2500);  /* dat */
    one_bag(2440);  /* thieu */
    one_bag(2560);  /* du */
    CHECK(ctx.fsm.stats.pass == 1);
    CHECK(ctx.fsm.stats.under == 1);
    CHECK(ctx.fsm.stats.over == 1);
    CHECK(ctx.fsm.stats.total == 3);
    CHECK(count_results() == 3);
    CHECK(g_rec[0].result == RESULT_PASS && g_rec[0].id == 1 && g_rec[0].weight_x10 == 2500);
    CHECK(g_rec[1].result == RESULT_UNDER && g_rec[1].id == 2);
    CHECK(g_rec[2].result == RESULT_OVER && g_rec[2].id == 3);
    CHECK(g_beeps == 2);  /* coi chi keu khi thieu/du */
}

static void t_hold_long(void)
{
    printf("[11.3] giu tui lau chi ghi 1 luot; lay ra dat lai ghi luot moi\n");
    reset_all();
    feed(20, 0, 0, 0);
    feed(300, 2500, 1, 0);  /* giu 30 s */
    CHECK(count_results() == 1);
    CHECK(ctx.fsm.state == WS_WAIT_REMOVE);
    CHECK(g_ind == IND_PASS);
    feed(15, 0, 0, 0);
    CHECK(ctx.fsm.state == WS_WAIT_ITEM);
    CHECK(g_ind == IND_OFF);
    one_bag(2500);
    CHECK(count_results() == 2);
    CHECK(g_rec[1].id == 2);
}

static void t_hand_block(void)
{
    printf("[11.4] chan tia bang tay khong tang so da kiem tra\n");
    reset_all();
    feed(20, 0, 0, 0);
    feed(20, 0, 1, 0);   /* chan tia, can = 0 */
    feed(10, 0, 0, 0);
    feed(20, 30, 1, 0);  /* cham nhe tay 3 g < nguong 10 g */
    feed(10, 0, 0, 0);
    CHECK(ctx.fsm.stats.total == 0);
    CHECK(ctx.fsm.stats.detect == 2);
    CHECK(count_results() == 0);
}

static void t_unstable_and_faults(void)
{
    printf("[11.5] dao dong chua ket luan; loi cam bien va qua tai khong bao dat\n");
    reset_all();
    feed(20, 0, 0, 0);
    feed(100, 2500, 1, 30);   /* dao dong +/-3 g lien tuc */
    CHECK(count_results() == 0);
    CHECK(ctx.fsm.state == WS_WAIT_STABLE);
    CHECK(ctx.fsm.stable_timeout);
    feed(15, 0, 0, 0);
    CHECK(ctx.fsm.state == WS_WAIT_ITEM);

    /* HX711 mat ket noi khi dang co tui */
    feed(5, 2500, 1, 0);
    feed_status(40, SENSOR_ERR_TIMEOUT, 1);
    CHECK(count_results() == 0);
    CHECK(app_fault(&ctx));
    CHECK(g_ind == IND_FAULT);

    /* Khong nhan duoc so do nao (task treo) */
    reset_all();
    feed(20, 0, 0, 0);
    now += 3000;
    app_tick(&ctx, now);
    CHECK(ctx.sensor_lost && app_fault(&ctx));

    /* Qua tai 950 g */
    reset_all();
    feed(20, 0, 0, 0);
    feed(40, 9500, 1, 0);
    CHECK(count_results() == 0);
    CHECK(ctx.overload);
    feed(15, 0, 0, 0);
    /* ADC bao hoa */
    feed_status(30, SENSOR_ERR_SATURATED, 1);
    CHECK(count_results() == 0);
}

static void t_tare_and_input(void)
{
    printf("[11.6] tru bi, doi nguong, huy nhap, nhap ngoai pham vi\n");
    reset_all();
    feed(20, 0, 0, 0);
    /* Dat khay 150 g (khong chan tia) roi tru bi */
    feed(30, 1500, 0, 0);
    key('C', KEY_EVT_PRESS);
    CHECK(ctx.tare_x10 == 1500);
    feed(5, 1500, 0, 0);
    CHECK(ctx.net_x10 == 0);
    /* Tui 250 g dat len khay -> gross 400 g, tinh 250 g */
    feed(30, 4000, 1, 0);
    CHECK(count_results() == 1 && g_rec[0].weight_x10 == 2500 && g_rec[0].result == RESULT_PASS);
    /* Dang co tui: khong cho tru bi / doi nguong */
    key('C', KEY_EVT_PRESS);
    CHECK(ctx.tare_x10 == 1500);
    key('A', KEY_EVT_PRESS);
    CHECK(ctx.ui == UI_HOME);
    feed(15, 1500, 0, 0);
    CHECK(ctx.fsm.state == WS_WAIT_ITEM);

    /* Dat muc tieu 500 g: A 5 0 0 # */
    key('A', KEY_EVT_PRESS); keys("500#");
    CHECK(ctx.cfg.target_x10 == 5000);
    CHECK(g_nsave == 1 && g_saved.target_x10 == 5000 && config_is_valid(&g_saved));
    /* Sai so 5 g: B 5 # */
    key('B', KEY_EVT_PRESS); keys("5#");
    CHECK(ctx.cfg.tol_x10 == 50);
    /* Ngoai pham vi 900 g -> khong doi, van o man hinh nhap */
    key('A', KEY_EVT_PRESS); keys("900#");
    CHECK(ctx.ui == UI_EDIT_TARGET && ctx.cfg.target_x10 == 5000);
    /* Xoa so bang * roi huy bang D */
    keys("12*");
    CHECK(strcmp(ctx.input.digits, "1") == 0);
    key('D', KEY_EVT_PRESS);
    CHECK(ctx.ui == UI_HOME && ctx.cfg.target_x10 == 5000);
    /* # khi chua nhap */
    key('B', KEY_EVT_PRESS); key('#', KEY_EVT_PRESS);
    CHECK(ctx.ui == UI_EDIT_TOL);
    key('D', KEY_EVT_PRESS);
    /* Tru bi khi chua on dinh -> tu choi */
    feed(2, 1000, 0, 50);
    key('C', KEY_EVT_PRESS);
    CHECK(ctx.tare_x10 == 1500);
}

static void t_calibration_and_persist(void)
{
    app_config_t c;
    printf("[11.7] hieu chuan, luu cau hinh, khoi dong lai van giu\n");
    reset_all();
    feed(20, 0, 0, 0);
    /* Gia lap he so that la 1000 dem/g thay vi 2000 */
    hold_d();
    CHECK(ctx.ui == UI_MENU);
    key('1', KEY_EVT_PRESS);
    CHECK(ctx.ui == UI_CAL_EMPTY);
    feed(20, 0, 0, 0);
    key('#', KEY_EVT_PRESS);
    CHECK(ctx.ui == UI_CAL_REF && ctx.cal_offset_new == OFFSET);
    CHECK(strcmp(ctx.input.digits, "200") == 0);
    key('#', KEY_EVT_PRESS);
    CHECK(ctx.ui == UI_CAL_PLACE && ctx.cal_ref_g == 200);
    /* Vat 200 g voi he so that 1000 dem/g = 200000 dem = "100 g" theo he so cu */
    feed(30, 1000, 0, 0);
    key('#', KEY_EVT_PRESS);
    CHECK(ctx.ui == UI_CAL_CONFIRM);
    CHECK(ctx.cal_factor_new > 999.0f && ctx.cal_factor_new < 1001.0f);
    key('#', KEY_EVT_PRESS);
    CHECK(ctx.ui == UI_HOME);
    CHECK(g_saved.cal_factor > 999.0f && config_is_valid(&g_saved));

    /* Khoi dong lai voi cau hinh da luu */
    c = g_saved;
    app_init(&ctx, &HOOKS, &c, now);
    CHECK(ctx.cfg_loaded && ctx.cfg.cal_factor == g_saved.cal_factor);
    /* Cau hinh hong -> mac dinh */
    c.target_x10 = 123;
    app_init(&ctx, &HOOKS, &c, now);
    CHECK(!ctx.cfg_loaded && ctx.cfg.target_x10 == DEFAULT_TARGET_X10);

    /* Huy hieu chuan giua chung */
    reset_all();
    feed(20, 0, 0, 0);
    hold_d(); key('1', KEY_EVT_PRESS); key('D', KEY_EVT_PRESS);
    CHECK(ctx.ui == UI_HOME && g_nsave == 0);

    /* Giu D de huy o man hinh nhap khong duoc lot vao menu */
    key('A', KEY_EVT_PRESS); hold_d();
    CHECK(ctx.ui == UI_HOME);
}

static void t_stats_match_records(void)
{
    int i;
    printf("[11.7] tong thong ke khop so ban ghi; xoa thong ke co xac nhan\n");
    reset_all();
    feed(20, 0, 0, 0);
    for (i = 0; i < 10; i++) { one_bag((i % 3 == 0) ? 2440 : 2500); }
    CHECK(ctx.fsm.stats.total == 10);
    CHECK(ctx.fsm.stats.total == ctx.fsm.stats.pass + ctx.fsm.stats.under + ctx.fsm.stats.over);
    CHECK(count_results() == 10);
    for (i = 0; i < 10; i++) { CHECK(g_rec[i].id == (uint32_t)(i + 1)); }
    hold_d(); key('2', KEY_EVT_PRESS);
    CHECK(ctx.ui == UI_CLEAR_CONFIRM);
    key('#', KEY_EVT_PRESS);
    CHECK(ctx.fsm.stats.total == 0 && g_rec[g_nrec - 1].kind == REC_STATS_CLEARED);
    /* Menu bi khoa khi dang can */
    feed(30, 2500, 1, 0);
    hold_d();
    CHECK(ctx.ui == UI_HOME);
}

static void t_render(void)
{
    DisplaySnapshot_t s;
    int r;
    printf("[ui] noi dung man hinh khong vuot qua %d cot\n", DISP_COLS);
    reset_all();
    feed(20, 0, 0, 0);
    one_bag(2500);
    feed(30, 2440, 1, 0);
    app_render(&ctx, &s);
    printf("  |%s|\n  big:%s\n", s.title, s.big);
    for (r = 0; r < DISP_TEXT_ROWS; r++) { printf("  %d|%s|\n", r, s.line[r]); CHECK(strlen(s.line[r]) <= DISP_COLS); }
    CHECK(strcmp(s.big, "244.0 G") == 0);
    key('A', KEY_EVT_PRESS);           /* dang co tui -> bi chan */
    CHECK(ctx.ui == UI_HOME);
    feed(15, 0, 0, 0);                 /* lay tui ra */
    key('A', KEY_EVT_PRESS); keys("25");
    app_render(&ctx, &s);
    CHECK(strcmp(s.big, "25_ G") == 0);
}

int main(void)
{
    t_format();
    t_classify_boundaries();
    t_hold_long();
    t_hand_block();
    t_unstable_and_faults();
    t_tare_and_input();
    t_calibration_and_persist();
    t_stats_match_records();
    t_render();
    printf("\nKET QUA: %d dat, %d loi\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
