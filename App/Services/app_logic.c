/**
 * @file    app_logic.c
 * @brief   Cài đặt logic ứng dụng của MainTask (xem app_logic.h).
 *
 * Bảng phím theo đề tài:
 *   0-9 nhập số | A mục tiêu | B sai số | C trừ bì | D hủy/quay lại
 *   * xóa chữ số | # xác nhận | GIỮ D (1 s) ở màn hình cân -> menu
 *
 * Menu: 1 = hiệu chuẩn (3 bước, có xác nhận), 2 = xóa thống kê (có xác nhận).
 */
#include "app_logic.h"

#include <string.h>
#include "app_config.h"
#include "fmt.h"

/* ========================================================================== */
/*                               Tiện ích nội bộ                              */
/* ========================================================================== */

/** @brief Trị tuyệt đối của số nguyên 32 bit. */
static int32_t abs_i32(int32_t v)
{
    return (v < 0) ? -v : v;
}

/** @brief Hiện một dòng thông báo trong MESSAGE_SHOW_MS. */
static void show_msg(app_ctx_t *ctx, const char *text, uint16_t color)
{
    fmt_copy(ctx->msg, sizeof(ctx->msg), text);
    ctx->msg_color = color;
    ctx->msg_until_ms = ctx->now_ms + MESSAGE_SHOW_MS;
}

/** @brief Gom thông số cấu hình hiện tại cho máy trạng thái lượt cân. */
static void build_params(const app_ctx_t *ctx, weigh_params_t *p)
{
    p->target_x10 = ctx->cfg.target_x10;
    p->tol_x10 = ctx->cfg.tol_x10;
    p->item_min_x10 = APP_ITEM_MIN_X10;
    p->empty_band_x10 = APP_EMPTY_BAND_X10;
    p->stable_timeout_ms = STABLE_TIMEOUT_MS;
}

/** @brief Tạo một ResultRecord_t và đẩy qua hook emit_record (sang UartTask). */
static void emit(app_ctx_t *ctx, record_kind_t kind, uint32_t id, int32_t w, uint8_t res)
{
    ResultRecord_t rec;

    rec.kind = (uint8_t)kind;
    rec.result = res;
    rec.id = id;
    rec.weight_x10 = w;
    rec.target_x10 = ctx->cfg.target_x10;
    rec.tol_x10 = ctx->cfg.tol_x10;
    if ((ctx->hooks != NULL) && (ctx->hooks->emit_record != NULL))
    {
        ctx->hooks->emit_record(&rec);
    }
}

/** @brief Gọi hook đèn/còi khi trạng thái đổi hoặc khi cần kêu còi. */
static void set_indicator(app_ctx_t *ctx, indicate_t what, bool beep)
{
    if ((what != ctx->ind_state) || beep)
    {
        ctx->ind_state = what;
        if ((ctx->hooks != NULL) && (ctx->hooks->indicate != NULL))
        {
            ctx->hooks->indicate(what, beep);
        }
    }
}

/** @brief Chọn trạng thái đèn phù hợp (không kêu còi). */
static void refresh_indicator(app_ctx_t *ctx)
{
    indicate_t want = IND_OFF;

    if (app_fault(ctx))
    {
        want = IND_FAULT;
    }
    else if ((ctx->fsm.state == WS_WAIT_REMOVE) && ctx->fsm.has_last)
    {
        want = (ctx->fsm.last_result == RESULT_PASS) ? IND_PASS : IND_FAIL;
    }
    else
    {
        want = IND_OFF;
    }
    set_indicator(ctx, want, false);
}

/** @brief Lưu cấu hình; trả về true nếu ghi flash thành công. */
static bool save_cfg(app_ctx_t *ctx)
{
    config_seal(&ctx->cfg);
    if ((ctx->hooks != NULL) && (ctx->hooks->save_config != NULL))
    {
        return ctx->hooks->save_config(&ctx->cfg);
    }
    return false;
}

/** @brief Quay về màn hình cân và mở lại lượt cân từ CHỜ TÚI. */
static void go_home(app_ctx_t *ctx)
{
    ctx->ui = UI_HOME;
    /* * Lượt cân luôn bắt đầu lại từ CHỜ TÚI sau khi rời menu / màn hình nhập. */
    weigh_fsm_reset_state(&ctx->fsm, ctx->now_ms);
}

/* ========================================================================== */
/*                               API công khai                                */
/* ========================================================================== */

/* Mô tả đầy đủ: xem app_logic.h. */
void app_init(app_ctx_t *ctx, const app_hooks_t *hooks, const app_config_t *loaded, uint32_t now_ms)
{
    (void)memset(ctx, 0, sizeof(*ctx));
    ctx->hooks = hooks;
    ctx->now_ms = now_ms;
    if ((loaded != NULL) && config_is_valid(loaded))
    {
        ctx->cfg = *loaded;
        ctx->cfg_loaded = true;
    }
    else
    {
        config_defaults(&ctx->cfg);
        ctx->cfg_loaded = false;
    }
    stab_reset(&ctx->stab);
    weigh_fsm_init(&ctx->fsm);
    ctx->ui = UI_HOME;
    ctx->ind_state = IND_OFF;
    show_msg(ctx, ctx->cfg_loaded ? "DA DOC CAU HINH" : "CAU HINH MAC DINH", COLOR_CYAN);
}

/* Mô tả đầy đủ: xem app_logic.h. */
bool app_fault(const app_ctx_t *ctx)
{
    return !ctx->have_meas || ctx->sensor_lost || (ctx->sensor_status != (uint8_t)SENSOR_OK) ||
           ctx->overload;
}

/* Mô tả đầy đủ: xem app_logic.h. */
void app_on_measurement(app_ctx_t *ctx, const Measurement_t *m, uint32_t now_ms)
{
    weigh_params_t p;
    weigh_input_t in;
    weigh_event_t ev;

    ctx->now_ms = now_ms;
    ctx->last_meas_ms = now_ms;
    ctx->have_meas = true;
    ctx->sensor_lost = false;
    ctx->sensor_status = m->status;

    if (m->status == (uint8_t)SENSOR_OK)
    {
        ctx->raw_avg = m->raw_avg;
        ctx->gross_x10 = scale_raw_to_x10(m->raw_avg, ctx->cfg.cal_offset, ctx->cfg.cal_factor);
        ctx->net_x10 = ctx->gross_x10 - ctx->tare_x10;
        ctx->stable = stab_push(&ctx->stab, ctx->gross_x10);
        ctx->overload = (ctx->gross_x10 > APP_OVERLOAD_X10) || (ctx->net_x10 > APP_OVERLOAD_X10);
    }
    else
    {
        stab_reset(&ctx->stab);
        ctx->stable = false;
        ctx->overload = false;
    }

#if APP_USE_PHOTO_SENSOR
    ctx->photo = (m->photo != 0U);
#else
    ctx->photo = (!app_fault(ctx)) && (ctx->net_x10 >= APP_ITEM_MIN_X10);
#endif

    /* * Máy trạng thái chỉ chạy ở màn hình cân: trong menu/màn hình nhập
     *   không ghi nhận lượt cân. */
    if (ctx->ui == UI_HOME)
    {
        build_params(ctx, &p);
        in.net_x10 = ctx->net_x10;
        in.stable = ctx->stable;
        in.photo = ctx->photo;
        in.fault = app_fault(ctx);
        in.now_ms = now_ms;
        ev = weigh_fsm_step(&ctx->fsm, &p, &in);
        if (ev == WEV_RESULT)
        {
            emit(ctx, REC_RESULT, ctx->fsm.stats.total, ctx->fsm.last_weight_x10,
                 (uint8_t)ctx->fsm.last_result);
            if (ctx->fsm.last_result == RESULT_PASS)
            {
                set_indicator(ctx, IND_PASS, false);
            }
            else
            {
                set_indicator(ctx, IND_FAIL, true);
            }
        }
    }
    refresh_indicator(ctx);
}

/* Mô tả đầy đủ: xem app_logic.h. */
void app_tick(app_ctx_t *ctx, uint32_t now_ms)
{
    ctx->now_ms = now_ms;
    /* ! Không nhận được số đo trong APP_SENSOR_LOST_MS: coi như mất HX711
     *   (SensorTask bị treo hoặc queue tắc) và chặn mọi kết quả. */
    if ((now_ms - ctx->last_meas_ms) > APP_SENSOR_LOST_MS)
    {
        if (!ctx->sensor_lost)
        {
            ctx->sensor_lost = true;
            ctx->stable = false;
            stab_reset(&ctx->stab);
        }
    }
    refresh_indicator(ctx);
}

/* ========================================================================== */
/*                               Xử lý phím                                   */
/* ========================================================================== */

/** @brief Kiểm tra có được phép đổi cấu hình/trừ bì/hiệu chuẩn lúc này không. */
static bool allow_settings(app_ctx_t *ctx)
{
    if (weigh_fsm_busy(&ctx->fsm))
    {
        show_msg(ctx, "DANG CAN - LAY TUI RA", COLOR_ORANGE);
        return false;
    }
    return true;
}

/** @brief Kiểm tra số đo đủ tốt để chụp (trừ bì, hiệu chuẩn). */
static bool measurement_ready(app_ctx_t *ctx)
{
    if (app_fault(ctx))
    {
        show_msg(ctx, "LOI CAM BIEN / QUA TAI", COLOR_RED);
        return false;
    }
    if (!ctx->stable)
    {
        show_msg(ctx, "CHUA ON DINH - CHO THEM", COLOR_ORANGE);
        return false;
    }
    return true;
}

/** @brief Xử lý phím ở màn hình cân: A/B nhập ngưỡng, C trừ bì, giữ D mở menu. */
static void key_home(app_ctx_t *ctx, const KeyEvent_t *k)
{
    if ((k->type == (uint8_t)KEY_EVT_HOLD) && (k->key == 'D'))
    {
        /* ! Chỉ mở menu nếu lần nhấn D bắt đầu ở màn hình cân; tránh trường hợp
         *   giữ D để hủy ở màn hình nhập rồi lọt luôn vào menu. */
        if (ctx->d_armed && allow_settings(ctx))
        {
            ctx->ui = UI_MENU;
        }
        return;
    }
    if (k->type != (uint8_t)KEY_EVT_PRESS)
    {
        return;
    }
    switch (k->key)
    {
    case 'A':
        if (allow_settings(ctx))
        {
            num_input_begin(&ctx->input, 3U, TARGET_MIN_G, TARGET_MAX_G);
            ctx->ui = UI_EDIT_TARGET;
        }
        break;
    case 'B':
        if (allow_settings(ctx))
        {
            num_input_begin(&ctx->input, 3U, TOL_MIN_G, TOL_MAX_G);
            ctx->ui = UI_EDIT_TOL;
        }
        break;
    case 'C':
        if (allow_settings(ctx) && measurement_ready(ctx))
        {
            /* * Trừ bì chỉ dời mốc 0 của phép đo, không đụng tới hiệu chuẩn. */
            ctx->tare_x10 = ctx->gross_x10;
            ctx->net_x10 = 0;
            show_msg(ctx, "DA TRU BI", COLOR_GREEN);
        }
        break;
    default:
        break;
    }
}

/** @brief Xử lý phím khi nhập mục tiêu (A) hoặc sai số (B); # xác nhận và lưu flash. */
static void key_edit(app_ctx_t *ctx, const KeyEvent_t *k)
{
    int32_t v = 0;
    int rc;
    char text[DISP_COLS + 1];

    if (k->type != (uint8_t)KEY_EVT_PRESS)
    {
        return;
    }
    if (k->key == 'D')
    {
        go_home(ctx);
        show_msg(ctx, "DA HUY", COLOR_GRAY);
        return;
    }
    if (num_input_key(&ctx->input, k->key))
    {
        return;
    }
    if (k->key != '#')
    {
        return;
    }

    rc = num_input_value(&ctx->input, &v);
    if (rc == -1)
    {
        show_msg(ctx, "CHUA NHAP GIA TRI", COLOR_ORANGE);
        return;
    }
    if (rc == -2)
    {
        show_msg(ctx, "NGOAI PHAM VI CHO PHEP", COLOR_RED);
        num_input_begin(&ctx->input, ctx->input.max_len, ctx->input.min, ctx->input.max);
        return;
    }

    if (ctx->ui == UI_EDIT_TARGET)
    {
        ctx->cfg.target_x10 = v * 10;
        fmt_copy(text, sizeof(text), "MUC TIEU ");
    }
    else
    {
        ctx->cfg.tol_x10 = v * 10;
        fmt_copy(text, sizeof(text), "SAI SO +/-");
    }
    (void)fmt_cat_i32(text, sizeof(text), v);
    (void)fmt_cat(text, sizeof(text), " G");

    go_home(ctx);
    if (save_cfg(ctx))
    {
        (void)fmt_cat(text, sizeof(text), " - DA LUU");
        show_msg(ctx, text, COLOR_GREEN);
    }
    else
    {
        show_msg(ctx, "LOI GHI FLASH (CHUA LUU)", COLOR_RED);
    }
    emit(ctx, REC_CONFIG_CHANGED, ctx->fsm.stats.total, 0, 0U);
}

/** @brief Xử lý phím trong menu: 1 hiệu chuẩn, 2 xóa thống kê, D thoát. */
static void key_menu(app_ctx_t *ctx, const KeyEvent_t *k)
{
    if (k->type != (uint8_t)KEY_EVT_PRESS)
    {
        return;
    }
    switch (k->key)
    {
    case '1':
        ctx->ui = UI_CAL_EMPTY;
        break;
    case '2':
        ctx->ui = UI_CLEAR_CONFIRM;
        break;
    case 'D':
        go_home(ctx);
        break;
    default:
        break;
    }
}

/** @brief Xử lý 3 bước hiệu chuẩn và bước xác nhận lưu hệ số. */
static void key_calibration(app_ctx_t *ctx, const KeyEvent_t *k)
{
    int32_t v = 0;
    int32_t diff;

    if (k->type != (uint8_t)KEY_EVT_PRESS)
    {
        return;
    }
    if (k->key == 'D')
    {
        go_home(ctx);
        show_msg(ctx, "DA HUY HIEU CHUAN", COLOR_GRAY);
        return;
    }

    switch (ctx->ui)
    {
    case UI_CAL_EMPTY:
        if ((k->key == '#') && measurement_ready(ctx))
        {
            ctx->cal_offset_new = ctx->raw_avg;
            num_input_begin(&ctx->input, 3U, CAL_REF_MIN_G, CAL_REF_MAX_G);
            num_input_preset(&ctx->input, CAL_REF_DEFAULT_G);
            ctx->ui = UI_CAL_REF;
        }
        break;

    case UI_CAL_REF:
        if (num_input_key(&ctx->input, k->key))
        {
            break;
        }
        if (k->key == '#')
        {
            if (num_input_value(&ctx->input, &v) == 0)
            {
                ctx->cal_ref_g = v;
                ctx->ui = UI_CAL_PLACE;
            }
            else
            {
                show_msg(ctx, "NHAP 50 - 900 G", COLOR_RED);
            }
        }
        break;

    case UI_CAL_PLACE:
        if ((k->key == '#') && measurement_ready(ctx))
        {
            diff = ctx->raw_avg - ctx->cal_offset_new;
            if (abs_i32(diff) < CAL_MIN_DIFF_COUNTS)
            {
                show_msg(ctx, "CHENH LECH NHO - KT DAY", COLOR_RED);
            }
            else
            {
                ctx->cal_factor_new = (float)diff / (float)ctx->cal_ref_g;
                ctx->ui = UI_CAL_CONFIRM;
            }
        }
        break;

    case UI_CAL_CONFIRM:
        if (k->key == '#')
        {
            ctx->cfg.cal_offset = ctx->cal_offset_new;
            ctx->cfg.cal_factor = ctx->cal_factor_new;
            ctx->tare_x10 = 0;
            /* * Đơn vị cũ trong cửa sổ ổn định không còn đúng: xét lại từ đầu. */
            stab_reset(&ctx->stab);
            ctx->stable = false;
            go_home(ctx);
            if (save_cfg(ctx))
            {
                show_msg(ctx, "DA HIEU CHUAN - DA LUU", COLOR_GREEN);
            }
            else
            {
                show_msg(ctx, "LOI GHI FLASH (CHUA LUU)", COLOR_RED);
            }
            emit(ctx, REC_CONFIG_CHANGED, ctx->fsm.stats.total, 0, 0U);
        }
        break;

    default:
        break;
    }
}

/** @brief Xác nhận (#) hoặc hủy (D) việc xóa thống kê. */
static void key_clear(app_ctx_t *ctx, const KeyEvent_t *k)
{
    if (k->type != (uint8_t)KEY_EVT_PRESS)
    {
        return;
    }
    if (k->key == '#')
    {
        weigh_fsm_clear_stats(&ctx->fsm);
        go_home(ctx);
        show_msg(ctx, "DA XOA THONG KE", COLOR_GREEN);
        emit(ctx, REC_STATS_CLEARED, 0U, 0, 0U);
    }
    else if (k->key == 'D')
    {
        go_home(ctx);
    }
    else
    {
        /* Bỏ qua phím khác. */
    }
}

/* Mô tả đầy đủ: xem app_logic.h. */
void app_on_key(app_ctx_t *ctx, const KeyEvent_t *k, uint32_t now_ms)
{
    ctx->now_ms = now_ms;
    if (k->type == (uint8_t)KEY_EVT_PRESS)
    {
        ctx->d_armed = (ctx->ui == UI_HOME) && (k->key == 'D');
    }
    switch (ctx->ui)
    {
    case UI_HOME:
        key_home(ctx, k);
        break;
    case UI_EDIT_TARGET:
    case UI_EDIT_TOL:
        key_edit(ctx, k);
        break;
    case UI_MENU:
        key_menu(ctx, k);
        break;
    case UI_CAL_EMPTY:
    case UI_CAL_REF:
    case UI_CAL_PLACE:
    case UI_CAL_CONFIRM:
        key_calibration(ctx, k);
        break;
    case UI_CLEAR_CONFIRM:
    default:
        key_clear(ctx, k);
        break;
    }
    refresh_indicator(ctx);
}

/* ========================================================================== */
/*                               Dựng màn hình                                */
/* ========================================================================== */

/** @brief Ghi một dòng chữ nhỏ vào bản chụp màn hình. */
static void line(DisplaySnapshot_t *o, uint8_t row, const char *text, uint16_t color)
{
    if (row < DISP_TEXT_ROWS)
    {
        fmt_copy(o->line[row], sizeof(o->line[row]), text);
        o->line_fg[row] = color;
    }
}

/** @brief Ghi khối lượng dạng "250.2 G" vào chuỗi cỡ lớn. */
static void big_weight(DisplaySnapshot_t *o, int32_t v_x10, uint16_t color)
{
    o->big[0] = '\0';
    (void)fmt_cat_x10(o->big, sizeof(o->big), v_x10);
    (void)fmt_cat(o->big, sizeof(o->big), " G");
    o->big_fg = color;
}

/** @brief Ghi chuỗi đang nhập kèm con trỏ "_". */
static void big_input(DisplaySnapshot_t *o, const num_input_t *in)
{
    fmt_copy(o->big, sizeof(o->big), in->digits);
    (void)fmt_cat(o->big, sizeof(o->big), "_ G");
    o->big_fg = COLOR_YELLOW;
}

/** @brief Chuỗi mô tả lỗi cảm biến hiện tại. */
static const char *fault_text(const app_ctx_t *ctx)
{
    if (!ctx->have_meas)
    {
        return "DANG KHOI DONG HX711...";
    }
    if (ctx->sensor_lost || (ctx->sensor_status == (uint8_t)SENSOR_ERR_TIMEOUT))
    {
        return "LOI: HX711 KHONG PHAN HOI";
    }
    if (ctx->sensor_status == (uint8_t)SENSOR_ERR_SATURATED)
    {
        return "LOI: ADC BAO HOA";
    }
    return "LOI: QUA TAI";
}

/** @brief Ghi số cân cỡ lớn; hiện "----.-" khi lỗi, màu vàng khi chưa ổn định. */
static void render_live_weight(const app_ctx_t *ctx, DisplaySnapshot_t *o, int32_t v_x10)
{
    if (app_fault(ctx))
    {
        fmt_copy(o->big, sizeof(o->big), "----.- G");
        o->big_fg = COLOR_RED;
    }
    else
    {
        big_weight(o, v_x10, ctx->stable ? COLOR_WHITE : COLOR_YELLOW);
    }
}

/** @brief Dựng màn hình cân: trạng thái lượt, số cân, ngưỡng, kết quả, thống kê. */
static void render_home(const app_ctx_t *ctx, DisplaySnapshot_t *o)
{
    char t[DISP_COLS + 1];
    const weigh_fsm_t *f = &ctx->fsm;
    uint16_t res_color;

    /* Thanh tiêu đề theo trạng thái lượt cân. */
    fmt_copy(o->title, sizeof(o->title), " ");
    (void)fmt_cat(o->title, sizeof(o->title), weigh_state_name(f->state));
    switch (f->state)
    {
    case WS_WAIT_ITEM:   o->title_bg = COLOR_DARKBLUE; break;
    case WS_WAIT_STABLE:
    case WS_RECORD:      o->title_bg = COLOR_ORANGE;   break;
    default:
        o->title_bg = (f->last_result == RESULT_PASS) ? COLOR_GREEN : COLOR_RED;
        break;
    }
    if (app_fault(ctx))
    {
        fmt_copy(o->title, sizeof(o->title), " LOI CAM BIEN");
        o->title_bg = COLOR_RED;
    }

    render_live_weight(ctx, o, ctx->net_x10);

    /* Dòng 0: ngưỡng */
    fmt_copy(t, sizeof(t), "MUC TIEU ");
    (void)fmt_cat_x10(t, sizeof(t), ctx->cfg.target_x10);
    (void)fmt_cat(t, sizeof(t), " +/-");
    (void)fmt_cat_x10(t, sizeof(t), ctx->cfg.tol_x10);
    line(o, 0U, t, COLOR_CYAN);

    /* Dòng 1: kết quả gần nhất */
    if (f->has_last)
    {
        fmt_copy(t, sizeof(t), "KQ #");
        (void)fmt_cat_i32(t, sizeof(t), (int32_t)f->stats.total);
        (void)fmt_cat(t, sizeof(t), ": ");
        (void)fmt_cat(t, sizeof(t), weigh_result_name(f->last_result));
        (void)fmt_cat(t, sizeof(t), " ");
        (void)fmt_cat_x10(t, sizeof(t), f->last_weight_x10);
        (void)fmt_cat(t, sizeof(t), " G");
        res_color = (f->last_result == RESULT_PASS) ? COLOR_GREEN : COLOR_RED;
        line(o, 1U, t, res_color);
    }
    else
    {
        line(o, 1U, "KQ: CHUA CO", COLOR_GRAY);
    }

    /* Dòng 2-3: thống kê */
    fmt_copy(t, sizeof(t), "TONG ");
    (void)fmt_cat_i32(t, sizeof(t), (int32_t)f->stats.total);
    (void)fmt_cat(t, sizeof(t), "  DAT ");
    (void)fmt_cat_i32(t, sizeof(t), (int32_t)f->stats.pass);
    line(o, 2U, t, COLOR_WHITE);

    fmt_copy(t, sizeof(t), "THIEU ");
    (void)fmt_cat_i32(t, sizeof(t), (int32_t)f->stats.under);
    (void)fmt_cat(t, sizeof(t), "  DU ");
    (void)fmt_cat_i32(t, sizeof(t), (int32_t)f->stats.over);
    (void)fmt_cat(t, sizeof(t), "  PH ");
    (void)fmt_cat_i32(t, sizeof(t), (int32_t)f->stats.detect);
    line(o, 3U, t, COLOR_WHITE);

    /* Dòng 4: bì và cảm biến quang */
    fmt_copy(t, sizeof(t), "BI ");
    (void)fmt_cat_x10(t, sizeof(t), ctx->tare_x10);
    (void)fmt_cat(t, sizeof(t), ctx->photo ? "  QUANG: CO VAT" : "  QUANG: TRONG");
    line(o, 4U, t, COLOR_GRAY);

    /* Dòng 5: trạng thái số đo */
    if (app_fault(ctx))
    {
        line(o, 5U, fault_text(ctx), COLOR_RED);
    }
    else if (f->stable_timeout)
    {
        line(o, 5U, "CHO ON DINH QUA LAU", COLOR_ORANGE);
    }
    else
    {
        line(o, 5U, ctx->stable ? "SO DO ON DINH" : "SO DO DANG DAO DONG",
             ctx->stable ? COLOR_GREEN : COLOR_YELLOW);
    }

    line(o, 8U, "A:MT B:SS C:BI GIU D:MENU", COLOR_GRAY);
}

/** @brief Dựng màn hình nhập mục tiêu / sai số. */
static void render_edit(const app_ctx_t *ctx, DisplaySnapshot_t *o)
{
    char t[DISP_COLS + 1];
    bool target = (ctx->ui == UI_EDIT_TARGET);

    fmt_copy(o->title, sizeof(o->title), target ? " CAI MUC TIEU (G)" : " CAI SAI SO +/- (G)");
    o->title_bg = COLOR_BLUE;
    big_input(o, &ctx->input);

    fmt_copy(t, sizeof(t), "HIEN TAI: ");
    (void)fmt_cat_x10(t, sizeof(t), target ? ctx->cfg.target_x10 : ctx->cfg.tol_x10);
    (void)fmt_cat(t, sizeof(t), " G");
    line(o, 0U, t, COLOR_CYAN);

    fmt_copy(t, sizeof(t), "PHAM VI: ");
    (void)fmt_cat_i32(t, sizeof(t), ctx->input.min);
    (void)fmt_cat(t, sizeof(t), " - ");
    (void)fmt_cat_i32(t, sizeof(t), ctx->input.max);
    (void)fmt_cat(t, sizeof(t), " G");
    line(o, 1U, t, COLOR_WHITE);

    line(o, 6U, "0-9: NHAP    *: XOA SO", COLOR_GRAY);
    line(o, 7U, "#: XAC NHAN  D: HUY", COLOR_GRAY);
}

/** @brief Dựng màn hình menu. */
static void render_menu(const app_ctx_t *ctx, DisplaySnapshot_t *o)
{
    (void)ctx;
    fmt_copy(o->title, sizeof(o->title), " MENU");
    o->title_bg = COLOR_BLUE;
    fmt_copy(o->big, sizeof(o->big), "MENU");
    o->big_fg = COLOR_CYAN;
    line(o, 0U, "1. HIEU CHUAN CAN", COLOR_WHITE);
    line(o, 1U, "2. XOA THONG KE", COLOR_WHITE);
    line(o, 7U, "D: THOAT", COLOR_GRAY);
}

/** @brief Dựng màn hình các bước hiệu chuẩn. */
static void render_calibration(const app_ctx_t *ctx, DisplaySnapshot_t *o)
{
    char t[DISP_COLS + 1];

    o->title_bg = COLOR_BLUE;
    fmt_copy(t, sizeof(t), "ADC: ");
    (void)fmt_cat_i32(t, sizeof(t), ctx->raw_avg);

    switch (ctx->ui)
    {
    case UI_CAL_EMPTY:
        fmt_copy(o->title, sizeof(o->title), " HIEU CHUAN 1/3");
        render_live_weight(ctx, o, ctx->gross_x10);
        line(o, 0U, "DE TRONG BAN CAN", COLOR_WHITE);
        line(o, 1U, "CHO ON DINH ROI NHAN #", COLOR_WHITE);
        line(o, 3U, t, COLOR_GRAY);
        break;

    case UI_CAL_REF:
        fmt_copy(o->title, sizeof(o->title), " HIEU CHUAN 2/3");
        big_input(o, &ctx->input);
        line(o, 0U, "NHAP KL VAT CHUAN (G)", COLOR_WHITE);
        line(o, 1U, "PHAM VI 50 - 900 G", COLOR_WHITE);
        line(o, 6U, "#: TIEP  *: XOA SO", COLOR_GRAY);
        break;

    case UI_CAL_PLACE:
        fmt_copy(o->title, sizeof(o->title), " HIEU CHUAN 3/3");
        render_live_weight(ctx, o, ctx->gross_x10);
        fmt_copy(o->line[0], sizeof(o->line[0]), "DAT VAT CHUAN ");
        (void)fmt_cat_i32(o->line[0], sizeof(o->line[0]), ctx->cal_ref_g);
        (void)fmt_cat(o->line[0], sizeof(o->line[0]), " G");
        o->line_fg[0] = COLOR_WHITE;
        line(o, 1U, "CHO ON DINH ROI NHAN #", COLOR_WHITE);
        line(o, 3U, t, COLOR_GRAY);
        break;

    default: /* UI_CAL_CONFIRM */
        fmt_copy(o->title, sizeof(o->title), " XAC NHAN HIEU CHUAN");
        fmt_copy(o->big, sizeof(o->big), "LUU?");
        o->big_fg = COLOR_YELLOW;
        fmt_copy(o->line[0], sizeof(o->line[0]), "HE SO: ");
        (void)fmt_cat_x10(o->line[0], sizeof(o->line[0]), (int32_t)(ctx->cal_factor_new * 10.0f));
        (void)fmt_cat(o->line[0], sizeof(o->line[0]), " DEM/G");
        o->line_fg[0] = COLOR_WHITE;
        fmt_copy(o->line[1], sizeof(o->line[1]), "OFFSET: ");
        (void)fmt_cat_i32(o->line[1], sizeof(o->line[1]), ctx->cal_offset_new);
        o->line_fg[1] = COLOR_WHITE;
        line(o, 6U, "#: LUU   D: HUY", COLOR_GRAY);
        break;
    }
    if (app_fault(ctx))
    {
        line(o, 5U, fault_text(ctx), COLOR_RED);
    }
    else if (ctx->ui != UI_CAL_REF && ctx->ui != UI_CAL_CONFIRM)
    {
        line(o, 5U, ctx->stable ? "SO DO ON DINH" : "SO DO DANG DAO DONG",
             ctx->stable ? COLOR_GREEN : COLOR_YELLOW);
    }
    else
    {
        /* Không hiện trạng thái số đo ở bước nhập. */
    }
    line(o, 7U, "D: HUY HIEU CHUAN", COLOR_GRAY);
}

/** @brief Dựng màn hình xác nhận xóa thống kê. */
static void render_clear(const app_ctx_t *ctx, DisplaySnapshot_t *o)
{
    char t[DISP_COLS + 1];

    fmt_copy(o->title, sizeof(o->title), " XOA THONG KE");
    o->title_bg = COLOR_RED;
    fmt_copy(o->big, sizeof(o->big), "XOA?");
    o->big_fg = COLOR_YELLOW;
    fmt_copy(t, sizeof(t), "TONG HIEN TAI: ");
    (void)fmt_cat_i32(t, sizeof(t), (int32_t)ctx->fsm.stats.total);
    line(o, 0U, t, COLOR_WHITE);
    line(o, 6U, "#: XOA   D: HUY", COLOR_GRAY);
}

/* Mô tả đầy đủ: xem app_logic.h. */
void app_render(const app_ctx_t *ctx, DisplaySnapshot_t *out)
{
    (void)memset(out, 0, sizeof(*out));
    out->title_bg = COLOR_DARKBLUE;
    out->big_fg = COLOR_WHITE;

    switch (ctx->ui)
    {
    case UI_HOME:
        render_home(ctx, out);
        break;
    case UI_EDIT_TARGET:
    case UI_EDIT_TOL:
        render_edit(ctx, out);
        break;
    case UI_MENU:
        render_menu(ctx, out);
        break;
    case UI_CLEAR_CONFIRM:
        render_clear(ctx, out);
        break;
    default:
        render_calibration(ctx, out);
        break;
    }

    /* Dòng cuối: thông báo ngắn hạn. */
    if ((int32_t)(ctx->msg_until_ms - ctx->now_ms) > 0)
    {
        line(out, (uint8_t)(DISP_TEXT_ROWS - 1U), ctx->msg, ctx->msg_color);
    }
}
