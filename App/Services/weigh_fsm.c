/**
 * @file    weigh_fsm.c
 * @brief   Cài đặt máy trạng thái lượt cân (xem sơ đồ trong weigh_fsm.h).
 */
#include "weigh_fsm.h"

#include <string.h>

/** @brief Trị tuyệt đối của số nguyên 32 bit. */
static int32_t abs_i32(int32_t v)
{
    return (v < 0) ? -v : v;
}

/** @brief Chuyển trạng thái và ghi lại thời điểm vào trạng thái mới. */
static void enter(weigh_fsm_t *m, weigh_state_t s, uint32_t now_ms)
{
    m->state = s;
    m->t_enter_ms = now_ms;
    m->stable_timeout = false;
}

/* Mô tả đầy đủ: xem weigh_fsm.h. */
void weigh_fsm_init(weigh_fsm_t *m)
{
    (void)memset(m, 0, sizeof(*m));
    m->state = WS_WAIT_ITEM;
}

/* Mô tả đầy đủ: xem weigh_fsm.h. */
void weigh_fsm_clear_stats(weigh_fsm_t *m)
{
    (void)memset(&m->stats, 0, sizeof(m->stats));
    m->has_last = false;
}

/* Mô tả đầy đủ: xem weigh_fsm.h. */
void weigh_fsm_reset_state(weigh_fsm_t *m, uint32_t now_ms)
{
    enter(m, WS_WAIT_ITEM, now_ms);
}

/* Mô tả đầy đủ: xem weigh_fsm.h. */
weigh_result_t weigh_classify(int32_t weight_x10, int32_t target_x10, int32_t tol_x10)
{
    if (weight_x10 < (target_x10 - tol_x10))
    {
        return RESULT_UNDER;
    }
    if (weight_x10 > (target_x10 + tol_x10))
    {
        return RESULT_OVER;
    }
    return RESULT_PASS;
}

/* Mô tả đầy đủ: xem weigh_fsm.h. */
bool weigh_fsm_busy(const weigh_fsm_t *m)
{
    return m->state != WS_WAIT_ITEM;
}

/* Mô tả đầy đủ: xem weigh_fsm.h. */
weigh_event_t weigh_fsm_step(weigh_fsm_t *m, const weigh_params_t *p, const weigh_input_t *in)
{
    weigh_event_t ev = WEV_NONE;
    bool has_item = in->photo && (in->net_x10 >= p->item_min_x10);

    /* * Bộ đếm phát hiện: đếm cạnh lên của cảm biến quang, tách riêng khỏi
     *   số sản phẩm đã kiểm tra. */
    if (in->photo && !m->prev_photo)
    {
        m->stats.detect++;
    }
    m->prev_photo = in->photo;

    switch (m->state)
    {
    case WS_WAIT_ITEM:
        if (!in->fault && has_item)
        {
            enter(m, WS_WAIT_STABLE, in->now_ms);
            ev = WEV_STARTED;
        }
        break;

    case WS_WAIT_STABLE:
        if (in->fault)
        {
            // ! Lỗi cảm biến hoặc quá tải: giữ nguyên, không bao giờ ghi kết quả.
            break;
        }
        if (!has_item)
        {
            enter(m, WS_WAIT_ITEM, in->now_ms);
            ev = WEV_ABORTED;
        }
        else if (in->stable)
        {
            m->captured_x10 = in->net_x10;
            enter(m, WS_RECORD, in->now_ms);
        }
        else if ((in->now_ms - m->t_enter_ms) > p->stable_timeout_ms)
        {
            /* * Chỉ báo, vẫn tiếp tục chờ: nếu sau đó cân ổn định thì vẫn ghi. */
            m->stable_timeout = true;
        }
        else
        {
            /* Tiếp tục chờ. */
        }
        break;

    case WS_RECORD:
        /* * Ghi đúng một lần bằng khối lượng đã chốt lúc ổn định. */
        m->last_weight_x10 = m->captured_x10;
        m->last_result = weigh_classify(m->captured_x10, p->target_x10, p->tol_x10);
        m->has_last = true;
        switch (m->last_result)
        {
        case RESULT_PASS:  m->stats.pass++;  break;
        case RESULT_UNDER: m->stats.under++; break;
        default:           m->stats.over++;  break;
        }
        m->stats.total++;
        enter(m, WS_WAIT_REMOVE, in->now_ms);
        ev = WEV_RESULT;
        break;

    case WS_WAIT_REMOVE:
    default:
        if (!in->fault && !in->photo && (abs_i32(in->net_x10) <= p->empty_band_x10))
        {
            enter(m, WS_WAIT_ITEM, in->now_ms);
            ev = WEV_REMOVED;
        }
        break;
    }
    return ev;
}

/* Mô tả đầy đủ: xem weigh_fsm.h. */
const char *weigh_state_name(weigh_state_t s)
{
    switch (s)
    {
    case WS_WAIT_ITEM:   return "CHO TUI";
    case WS_WAIT_STABLE: return "CHO ON DINH";
    case WS_RECORD:      return "GHI KET QUA";
    default:             return "CHO LAY TUI";
    }
}

/* Mô tả đầy đủ: xem weigh_fsm.h. */
const char *weigh_result_name(weigh_result_t r)
{
    switch (r)
    {
    case RESULT_PASS:  return "DAT";
    case RESULT_UNDER: return "THIEU";
    default:           return "DU";
    }
}

/* Mô tả đầy đủ: xem weigh_fsm.h. */
const char *weigh_result_csv(weigh_result_t r)
{
    switch (r)
    {
    case RESULT_PASS:  return "PASS";
    case RESULT_UNDER: return "UNDER";
    default:           return "OVER";
    }
}
