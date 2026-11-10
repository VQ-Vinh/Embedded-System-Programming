/**
 * @file    app_config.h
 * @brief   Toàn bộ hằng số cấu hình của ứng dụng cân kiểm tra khối lượng.
 *
 * Gom mọi thông số cần tinh chỉnh khi thử nghiệm (ngưỡng ổn định, vùng
 * không tải, giới hạn nhập, loại màn hình...) vào một chỗ để nhóm chỉ sửa
 * một file. Đơn vị khối lượng trong toàn bộ chương trình là 0,1 g, lưu bằng
 * số nguyên có hậu tố _X10 (ví dụ 2500 = 250,0 g) để tránh in số thực.
 *
 * Phụ thuộc: không có. File này được include bởi cả code chạy trên kit lẫn
 * code kiểm thử chạy trên máy tính.
 */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* ===================== Màn hình TFT ===================== */

#define TFT_CONTROLLER_ST7735   1  /**< TFT 1,8" 128x160 (mạch đỏ phổ biến) */
#define TFT_CONTROLLER_ILI9341  2  /**< TFT 2,4"-2,8" 240x320 */

// ? Chưa chốt loại màn hình: đổi giá trị này nếu dùng ILI9341.
#ifndef TFT_CONTROLLER
#define TFT_CONTROLLER          TFT_CONTROLLER_ST7735
#endif

// * Nếu màu đỏ và xanh dương bị đảo trên màn hình, đổi TFT_BGR giữa 0 và 1.
// * Mặc định: ST7735 mạch đỏ dùng RGB (0), ILI9341 dùng BGR (1).
#ifndef TFT_BGR
#if (TFT_CONTROLLER == TFT_CONTROLLER_ILI9341)
#define TFT_BGR                 1
#else
#define TFT_BGR                 0
#endif
#endif

// * Nếu chữ bị lật ngược 180 độ, đổi TFT_FLIP giữa 0 và 1.
#ifndef TFT_FLIP
#define TFT_FLIP                0
#endif

// * Một số module ST7735 ("green tab") bị lệch vài pixel: chỉnh 2 giá trị này.
#ifndef TFT_X_OFFSET
#define TFT_X_OFFSET            0
#endif
#ifndef TFT_Y_OFFSET
#define TFT_Y_OFFSET            0
#endif

/* ===================== Cảm biến quang ===================== */

/** Có dùng cảm biến quang hay không. Đặt 0 để chạy thử khi chưa lắp cảm
 *  biến: khi đó "có vật" được suy ra từ khối lượng vượt APP_ITEM_MIN_X10. */
#ifndef APP_USE_PHOTO_SENSOR
#define APP_USE_PHOTO_SENSOR    1
#endif

/** Mức logic tại PHOTO_IN khi có vật chắn tia.
 *  E18-D80NK và đa số module hồng ngoại: 0 (kéo xuống khi phát hiện). */
#define PHOTO_ACTIVE_LEVEL      0

/** Số lần đọc liên tiếp giống nhau mới chấp nhận đổi trạng thái (mỗi lần 5 ms). */
#define PHOTO_DEBOUNCE_COUNT    4

/* ===================== HX711 và xử lý số đo ===================== */

#define HX711_POLL_MS           5     /**< Chu kỳ kiểm tra chân DOUT */
#define HX711_TIMEOUT_MS        500   /**< Không có mẫu mới sau thời gian này -> lỗi */
#define FILTER_WINDOW           8     /**< Số mẫu trung bình trượt (10 SPS -> 0,8 s) */

#define STABLE_WINDOW           8     /**< Số mẫu đã lọc dùng để xét ổn định */
#define STABLE_BAND_X10         5     /**< Dao động tối đa trong cửa sổ: 0,5 g */
#define STABLE_HOLD_SAMPLES     3     /**< Số lần liên tiếp đạt điều kiện mới báo ổn định */
#define STABLE_TIMEOUT_MS       6000  /**< Chờ ổn định quá lâu -> báo, không ghi kết quả */

#define APP_ITEM_MIN_X10        100   /**< Khối lượng tối thiểu coi là có túi: 10 g */
#define APP_EMPTY_BAND_X10      30    /**< Vùng không tải: |cân| <= 3 g */
#define APP_OVERLOAD_X10        9000  /**< Quá tải phần mềm: 900 g */
#define APP_SENSOR_LOST_MS      1500  /**< Task chính không nhận số đo -> lỗi cảm biến */

/** Hệ số mặc định (số đếm / gam) khi chưa hiệu chuẩn, tính cho load cell
 *  1 kg độ nhạy 1 mV/V ở gain 128: 0,001 * 128 * 2^24 / 1000 ~ 2147. */
#define DEFAULT_CAL_FACTOR      2147.0f
#define DEFAULT_CAL_OFFSET      0

/* ===================== Ngưỡng và giới hạn nhập ===================== */

#define DEFAULT_TARGET_X10      2500  /**< 250 g */
#define DEFAULT_TOL_X10         50    /**< ±5 g */

#define TARGET_MIN_G            10
#define TARGET_MAX_G            800
#define TOL_MIN_G               1
#define TOL_MAX_G               100
#define CAL_REF_MIN_G           50
#define CAL_REF_MAX_G           900
#define CAL_REF_DEFAULT_G       200
#define CAL_MIN_DIFF_COUNTS     2000  /**< Chênh lệch số đếm tối thiểu khi hiệu chuẩn */

/* ===================== Keypad, cảnh báo, hiển thị ===================== */

#define KEYPAD_SCAN_MS          10
#define KEYPAD_DEBOUNCE_SCANS   3
#define KEYPAD_HOLD_MS          1000  /**< Giữ phím lâu hơn -> sự kiện HOLD */

#define BUZZER_BEEP_MS          400   /**< Thời gian kêu khi sản phẩm không đạt */
#define DISPLAY_PERIOD_MS       200   /**< Chu kỳ vẽ lại màn hình */
#define MESSAGE_SHOW_MS         1500  /**< Thời gian hiện dòng thông báo */

#define UART_BAUDRATE_TEXT      "115200"

#endif /* APP_CONFIG_H */
