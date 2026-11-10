/**
 * @file    weigh_fsm.h
 * @brief   Máy trạng thái quản lý một lượt cân, phân loại và thống kê.
 *
 * Bốn trạng thái theo đề tài:
 *
 *   CHỜ TÚI --(có vật + khối lượng >= ngưỡng)--> CHỜ ỔN ĐỊNH
 *   CHỜ ỔN ĐỊNH --(lấy túi ra trước khi ổn định)--> CHỜ TÚI
 *   CHỜ ỔN ĐỊNH --(ổn định, không lỗi)--> GHI KẾT QUẢ
 *   GHI KẾT QUẢ --(ghi đúng 1 lần)--> CHỜ LẤY TÚI
 *   CHỜ LẤY TÚI --(hết vật + cân về vùng không tải)--> CHỜ TÚI
 *
 * Quy tắc bảo đảm:
 * - Mỗi lượt chỉ ghi một kết quả; giữ túi lâu không ghi thêm.
 * - Lỗi cảm biến / quá tải không bao giờ sinh kết quả.
 * - Chắn tia bằng tay (không có khối lượng) chỉ tăng bộ đếm phát hiện,
 *   không tăng số sản phẩm đã kiểm tra.
 *
 * Module thuần logic, kiểm thử được trên máy tính.
 */
#ifndef WEIGH_FSM_H
#define WEIGH_FSM_H

#include <stdbool.h>
#include <stdint.h>
#include "app_types.h"

typedef enum
{
    WS_WAIT_ITEM = 0,   /**< CHỜ TÚI */
    WS_WAIT_STABLE,     /**< CHỜ ỔN ĐỊNH */
    WS_RECORD,          /**< GHI KẾT QUẢ */
    WS_WAIT_REMOVE      /**< CHỜ LẤY TÚI */
} weigh_state_t;

typedef enum
{
    WEV_NONE = 0,
    WEV_STARTED,        /**< Bắt đầu lượt mới */
    WEV_ABORTED,        /**< Túi bị lấy ra trước khi ổn định */
    WEV_RESULT,         /**< Vừa ghi kết quả (đọc out_result, out_weight) */
    WEV_REMOVED         /**< Túi đã lấy ra, mở lượt mới */
} weigh_event_t;

/** Thông số do người dùng / cấu hình quyết định. */
typedef struct
{
    int32_t  target_x10;
    int32_t  tol_x10;
    int32_t  item_min_x10;      /**< Khối lượng tối thiểu coi là có túi */
    int32_t  empty_band_x10;    /**< |cân| <= giá trị này coi là không tải */
    uint32_t stable_timeout_ms;
} weigh_params_t;

/** Đầu vào mỗi lần cập nhật. */
typedef struct
{
    int32_t  net_x10;   /**< Khối lượng tịnh (đã trừ bì) */
    bool     stable;
    bool     photo;     /**< Cảm biến quang phát hiện vật */
    bool     fault;     /**< Lỗi HX711 hoặc quá tải: không được ghi kết quả */
    uint32_t now_ms;
} weigh_input_t;

typedef struct
{
    uint32_t pass;
    uint32_t under;
    uint32_t over;
    uint32_t total;     /**< Luôn bằng pass + under + over */
    uint32_t detect;    /**< Số lần cảm biến quang chuyển sang "có vật" */
} weigh_stats_t;

typedef struct
{
    weigh_state_t  state;
    uint32_t       t_enter_ms;      /**< Thời điểm vào trạng thái hiện tại */
    bool           stable_timeout;  /**< Đang chờ ổn định quá lâu */
    bool           prev_photo;
    int32_t        captured_x10;    /**< Khối lượng ổn định chờ ghi */
    weigh_result_t last_result;
    int32_t        last_weight_x10;
    bool           has_last;
    weigh_stats_t  stats;
} weigh_fsm_t;

/** @brief Đưa máy trạng thái và thống kê về ban đầu. */
void weigh_fsm_init(weigh_fsm_t *m);

/** @brief Xóa thống kê, giữ nguyên trạng thái lượt hiện tại. */
void weigh_fsm_clear_stats(weigh_fsm_t *m);

/** @brief Đưa về CHỜ TÚI (dùng khi rời menu). Không đổi thống kê. */
void weigh_fsm_reset_state(weigh_fsm_t *m, uint32_t now_ms);

/**
 * @brief  Phân loại khối lượng theo mục tiêu và sai số (biên tính là ĐẠT).
 */
weigh_result_t weigh_classify(int32_t weight_x10, int32_t target_x10, int32_t tol_x10);

/**
 * @brief  Cập nhật máy trạng thái với một số đo mới.
 * @return Sự kiện vừa xảy ra. Với WEV_RESULT, kết quả nằm trong
 *         m->last_result và m->last_weight_x10; thống kê đã được cập nhật.
 */
weigh_event_t weigh_fsm_step(weigh_fsm_t *m, const weigh_params_t *p, const weigh_input_t *in);

/** @brief true nếu đang trong một lượt cân (không ở CHỜ TÚI). */
bool weigh_fsm_busy(const weigh_fsm_t *m);

/** @brief Tên trạng thái không dấu để hiển thị. */
const char *weigh_state_name(weigh_state_t s);

/** @brief Tên kết quả: "DAT", "THIEU", "DU". */
const char *weigh_result_name(weigh_result_t r);

/** @brief Tên kết quả dùng trong CSV: "PASS", "UNDER", "OVER". */
const char *weigh_result_csv(weigh_result_t r);

#endif /* WEIGH_FSM_H */
