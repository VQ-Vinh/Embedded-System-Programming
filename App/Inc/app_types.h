/**
 * @file    app_types.h
 * @brief   Kiểu dữ liệu trao đổi giữa các task qua Queue.
 *
 * Đây là "hợp đồng" chung của cả nhóm: mọi task chỉ giao tiếp với nhau qua
 * các struct trong file này. Thay đổi struct nào phải báo cả nhóm vì nó ảnh
 * hưởng tới task gửi lẫn task nhận.
 *
 * | Queue     | Kiểu              | Gửi -> nhận               |
 * |-----------|-------------------|---------------------------|
 * | qMeas     | Measurement_t     | SensorTask -> MainTask    |
 * | qKey      | KeyEvent_t        | KeypadTask -> MainTask    |
 * | qDisplay  | DisplaySnapshot_t | MainTask -> DisplayTask   |
 * | qUart     | ResultRecord_t    | MainTask -> UartTask      |
 *
 * Phụ thuộc: chỉ <stdint.h>, dùng được cả trên máy tính khi kiểm thử.
 */
#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <stdint.h>

/* ---------- Số đo từ SensorTask ---------- */

/** Trạng thái đọc HX711. */
typedef enum
{
    SENSOR_OK = 0,          /**< Có mẫu mới hợp lệ */
    SENSOR_ERR_TIMEOUT,     /**< HX711 không báo sẵn sàng (đứt dây, mất nguồn) */
    SENSOR_ERR_SATURATED    /**< ADC chạm giới hạn 24 bit (quá tải cơ học / nối sai) */
} sensor_status_t;

/** Một số đo đã lọc gửi từ SensorTask sang MainTask. */
typedef struct
{
    int32_t  raw_avg;   /**< Giá trị ADC sau trung bình trượt (chưa đổi sang gam) */
    int32_t  raw_last;  /**< Mẫu ADC mới nhất, dùng khi gỡ lỗi */
    uint8_t  photo;     /**< 1 = cảm biến quang đang phát hiện vật (đã chống dội) */
    uint8_t  status;    /**< sensor_status_t */
    uint32_t tick_ms;   /**< Thời điểm lấy mẫu (ms từ khi khởi động) */
} Measurement_t;

/* ---------- Sự kiện phím từ KeypadTask ---------- */

typedef enum
{
    KEY_EVT_PRESS = 0,  /**< Vừa nhấn (đã chống dội) */
    KEY_EVT_HOLD        /**< Giữ phím lâu hơn KEYPAD_HOLD_MS (gửi đúng 1 lần) */
} key_evt_type_t;

typedef struct
{
    char    key;        /**< '0'-'9', 'A'-'D', '*', '#' */
    uint8_t type;       /**< key_evt_type_t */
} KeyEvent_t;

/* ---------- Bản chụp màn hình cho DisplayTask ---------- */

#define DISP_COLS        26   /**< Số ký tự mỗi dòng (vừa cả ST7735 lẫn ILI9341) */
#define DISP_TEXT_ROWS   10   /**< Số dòng chữ nhỏ bên dưới số cân */
#define DISP_BIG_LEN     11   /**< Số ký tự tối đa của dòng số cân cỡ lớn */

/* Màu RGB565 dùng chung cho logic và driver màn hình. */
#define COLOR_BLACK      0x0000U
#define COLOR_WHITE      0xFFFFU
#define COLOR_RED        0xF800U
#define COLOR_GREEN      0x07E0U
#define COLOR_BLUE       0x001FU
#define COLOR_YELLOW     0xFFE0U
#define COLOR_CYAN       0x07FFU
#define COLOR_ORANGE     0xFD20U
#define COLOR_GRAY       0x8410U
#define COLOR_DARKBLUE   0x0010U

/** Toàn bộ nội dung màn hình tại một thời điểm. MainTask dựng struct này,
 *  DisplayTask chỉ việc vẽ, không cần biết logic ứng dụng. */
typedef struct
{
    char     title[DISP_COLS + 1];                    /**< Thanh tiêu đề */
    uint16_t title_bg;                                /**< Màu nền thanh tiêu đề */
    char     big[DISP_BIG_LEN + 1];                   /**< Số cân / giá trị đang nhập, cỡ lớn */
    uint16_t big_fg;                                  /**< Màu chữ cỡ lớn */
    char     line[DISP_TEXT_ROWS][DISP_COLS + 1];     /**< Các dòng chữ nhỏ */
    uint16_t line_fg[DISP_TEXT_ROWS];                 /**< Màu từng dòng */
} DisplaySnapshot_t;

/* ---------- Bản ghi kết quả cho UartTask ---------- */

typedef enum
{
    RESULT_PASS = 0,
    RESULT_UNDER,
    RESULT_OVER
} weigh_result_t;

typedef enum
{
    REC_RESULT = 0,     /**< Một lượt cân hợp lệ -> một dòng CSV */
    REC_STATS_CLEARED,  /**< Thống kê vừa bị xóa -> dòng chú thích '#' */
    REC_CONFIG_CHANGED  /**< Đổi mục tiêu / sai số / hiệu chuẩn -> dòng chú thích '#' */
} record_kind_t;

typedef struct
{
    uint8_t  kind;          /**< record_kind_t */
    uint8_t  result;        /**< weigh_result_t (chỉ với REC_RESULT) */
    uint32_t id;            /**< Mã lượt, tăng dần, dùng để phát hiện đếm trùng */
    int32_t  weight_x10;    /**< Khối lượng tịnh ổn định, đơn vị 0,1 g */
    int32_t  target_x10;
    int32_t  tol_x10;
} ResultRecord_t;

#endif /* APP_TYPES_H */
