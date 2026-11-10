# Hướng dẫn firmware cân kiểm tra khối lượng (STM32F4 Discovery + FreeRTOS)

Tài liệu này mô tả firmware hoàn chỉnh của dự án: cách nối dây, build, nạp, chạy lần đầu, hiệu chuẩn và kiểm thử. Đọc từ trên xuống khi bắt đầu.

## 1. Cấu trúc mã nguồn

```text
App/
  Inc/        app_config.h  (MỌI thông số chỉnh được: màn hình, ngưỡng ổn định, giới hạn nhập...)
              app_types.h   (struct trao đổi qua Queue - hợp đồng chung của cả nhóm)
              app_main.h    (handle Queue, khai báo 5 task)
  Drivers/    hx711, keypad, tft (+ font5x7), photo_sensor, indicator (LED/còi), flash_store, dwt_delay
  Services/   Logic thuần, không gọi HAL -> kiểm thử được trên máy tính:
              scale_filter (lọc, đổi gam, ổn định), weigh_fsm (4 trạng thái lượt cân, phân loại, thống kê),
              num_input (nhập số), config (cấu hình + CRC), app_logic (menu, trừ bì, hiệu chuẩn, màn hình), fmt
  Tasks/      app_main.c (tạo Queue + task), task_sensor, task_keypad, task_main, task_display, task_uart
tests/        test_logic.c + run_tests.sh (96 ca kiểm thử logic, chạy bằng gcc trên máy tính)
tools/        serial_logger.py (ghi CSV từ UART), gen_font.py (sinh font5x7.h)
docs/         tài liệu này, ảnh mô phỏng màn hình
```

Phân lớp đúng như đề tài yêu cầu: **driver** → **dịch vụ** → **ứng dụng (task)**.

| Task | Ưu tiên | Stack | Việc | Người phụ trách |
| --- | --- | --- | --- | --- |
| SensorTask | AboveNormal | 256 word | Đọc HX711, lọc trung bình 8 mẫu, đọc cảm biến quang → `qMeas` | Người 1 |
| KeypadTask | Normal | 256 word | Quét 10 ms, chống dội, sự kiện nhấn/giữ → `qKey` | Người 2 |
| MainTask | Normal | 512 word | Trạng thái lượt cân, cấu hình, thống kê, LED/còi, lưu flash | Người 3 |
| DisplayTask | BelowNormal | 512 word | Vẽ phần thay đổi của màn hình | Người 2 |
| UartTask | Low | 384 word | Gửi một dòng CSV mỗi lượt | Người 3 |

Mỗi ngoại vi chỉ thuộc về một task, nên không cần mutex. MainTask là task duy nhất sửa trạng thái chung.

## 2. Nối dây

Hướng dẫn chi tiết từng chân (kèm nguồn, GND) và cách chọn file để nạp: [NAP_KIT_VA_NOI_DAY.md](NAP_KIT_VA_NOI_DAY.md).

| Module | Chân module | STM32 | Ghi chú |
| --- | --- | --- | --- |
| TFT | SCK / SDA / CS / DC(A0) / RES | PB13 / PB15 / PB12 / PB11 / PB1 | LED (đèn nền) nối 3,3 V |
| HX711 | DT / SCK | PE7 / PE8 | VCC 3,3 V |
| Cảm biến quang | OUT | PE9 | Pull-up nội; mức tích cực đặt trong `PHOTO_ACTIVE_LEVEL` |
| LED xanh / đỏ | Anode qua 330 Ω | PE10 / PE11 | |
| Còi | Base S8050 qua 1 kΩ | PE12 | Còi chủ động 5 V |
| Keypad | R1–R4 / C1–C4 | PD0–PD3 / PD6–PD9 | |
| USB-UART | RX / TX / GND | PA2 / PA3 / GND | 115200 8N1, KHÔNG nối VCC |

Không dùng các chân đã có linh kiện trên Discovery: PA0, PA4–PA7, PE3, PB6, PB9, PD4, PC7, PC10, PC12, PA9–PA12, PC0, PD5, PB10, PC3, PD12–PD15, PB2, PB3.

## 3. Việc cần làm trước khi build

1. **Chọn loại màn hình** trong `App/Inc/app_config.h`:
   - TFT 1,8" 128×160 (ST7735): giữ nguyên `TFT_CONTROLLER_ST7735`.
   - TFT 2,4–2,8" 240×320 (ILI9341): đổi thành `TFT_CONTROLLER_ILI9341`.
2. **Kiểm tra lại cấu hình bằng CubeMX** (theo quy ước AGENTS.md). Phần cấu hình CubeMX đã được cập nhật sẵn trong `Src.ioc` và mã sinh ra (SPI2, USART2, GPIO + User Label, clock HSE 8 MHz → 168 MHz, heap FreeRTOS 32 KB). Bạn chỉ cần:
   - Mở `Src.ioc` bằng STM32CubeMX 6.16.1. Kiểm tra tab Pinout và tab Clock (HCLK 168 MHz, nguồn PLL là HSE 8 MHz).
   - Bấm **Generate Code**. Mọi code ứng dụng nằm trong `App/` hoặc trong vùng `USER CODE`, nên không bị mất.
   - Chạy `git diff`. Nếu CubeMX chỉ sắp xếp lại vài dòng, hoặc thêm lại `stm32f4xx_hal_spi.c` / `stm32f4xx_hal_uart.c` vào thư mục Drivers, thì hoàn toàn bình thường.
3. Build: `.\build.bat Debug` → flash: `.\flash.bat Debug` (cắm ST-LINK qua cổng mini-USB của Discovery).

## 4. Chạy lần đầu (bring-up theo từng bước)

| Bước | Làm gì | Kết quả mong đợi |
| --- | --- | --- |
| 1 | Chỉ cắm kit + TFT, nạp firmware | Màn hình hiện "CAN KIEM TRA STM32F4" rồi "LOI CAM BIEN" (chưa có HX711) |
| 2 | Mở `python tools/serial_logger.py COM?` | Thấy dòng `# CAN KIEM TRA...` và tiêu đề CSV sau khi nhấn RESET |
| 3 | Nối HX711 + load cell | Hết lỗi; số cân dao động quanh một giá trị bất kỳ (chưa hiệu chuẩn) |
| 4 | Nối keypad, bấm A | Vào màn hình "CAI MUC TIEU"; bấm D để thoát |
| 5 | Hiệu chuẩn (mục 5) | Đặt vật chuẩn thấy đúng khối lượng |
| 6 | Nối cảm biến quang | Dòng "QUANG: CO VAT / TRONG" đổi khi che tia. Nếu ngược thì đổi `PHOTO_ACTIVE_LEVEL` |
| 7 | LED, còi | Cân túi thiếu/dư: LED đỏ + còi; đạt: LED xanh |

Nếu chưa có cảm biến quang, đặt `APP_USE_PHOTO_SENSOR` = 0 để thử: hệ thống coi "có vật" khi khối lượng ≥ 10 g.

Lỗi màn hình hay gặp:
- Đỏ và xanh dương bị đảo màu: đổi `TFT_BGR`.
- Chữ bị lộn ngược: đổi `TFT_FLIP`.
- Có viền rác vài pixel (ST7735 "green tab"): chỉnh `TFT_X_OFFSET` / `TFT_Y_OFFSET` (thường là 2 / 1).
- Màn hình trắng xóa: kiểm tra dây CS, DC, RES, rồi kiểm tra đã chọn đúng `TFT_CONTROLLER` chưa.

## 5. Thao tác người dùng

| Phím | Ở màn hình cân | Ở màn hình nhập |
| --- | --- | --- |
| A | Cài khối lượng mục tiêu (10–800 g) | — |
| B | Cài sai số ± (1–100 g) | — |
| C | Trừ bì (chỉ khi số đo ổn định, không có túi) | — |
| Giữ D 1 s | Mở menu: 1 = hiệu chuẩn, 2 = xóa thống kê | — |
| 0–9 / * / # / D | — | Nhập số / xóa chữ số / xác nhận và lưu flash / hủy |

Ví dụ: `A 2 5 0 #` đặt mục tiêu 250 g; `B 5 #` đặt sai số ±5 g. Khi đang có một lượt cân, các thao tác A, B, C và menu bị khóa, màn hình báo "DANG CAN - LAY TUI RA".

**Hiệu chuẩn** (giữ D → 1):
1. Để trống bàn cân, chờ dòng "SO DO ON DINH" rồi nhấn #.
2. Nhập khối lượng vật chuẩn (mặc định 200 g) rồi nhấn #.
3. Đặt vật chuẩn lên cân, chờ ổn định, nhấn #. Màn hình hiện hệ số (số đếm/g); nhấn # để lưu.

Hiệu chuẩn sẽ xóa bì. Cấu hình (mục tiêu, sai số, hệ số) được lưu ở sector 11 của flash và giữ nguyên sau khi tắt nguồn. Thống kê chỉ nằm trong RAM.

## 6. Logic lượt cân

`CHO TUI` → có tia + ≥ 10 g → `CHO ON DINH` → ổn định → `GHI KET QUA` (ghi đúng 1 lần, gửi UART) → `CHO LAY TUI` → hết tia + |cân| ≤ 3 g → `CHO TUI`.

- **Ổn định:** 8 mẫu liên tiếp (sau lọc) dao động ≤ 0,5 g, giữ đúng 3 lần. Ở 10 SPS mất khoảng 1,5–2 s sau khi đặt túi.
- **Chờ ổn định quá 6 s:** báo "CHO ON DINH QUA LAU", vẫn chưa ghi kết quả.
- **Lỗi** HX711 (không phản hồi 0,5 s, ADC bão hòa), quá tải > 900 g, hoặc MainTask không nhận số đo 1,5 s: không bao giờ ghi kết quả; LED đỏ nhấp nháy.
- **Bộ đếm "PH"** (phát hiện) đếm số lần chắn tia, tách riêng khỏi TỔNG. TỔNG luôn bằng ĐẠT + THIẾU + DƯ.

Các ngưỡng này đều nằm trong `app_config.h`. Nhóm nên chỉnh lại sau khi đo độ ổn định thực tế (mục 11 của đề tài).

## 7. Dữ liệu gửi về máy tính

```csv
ID,Weight_g,Target_g,Tolerance_g,Result
1,250.2,250,5,PASS
2,244.0,250,5,UNDER
```

Dòng bắt đầu bằng `#` là chú thích (khởi động, `# STATS CLEARED`, `# CONFIG ...`). Chạy `pip install pyserial` một lần, sau đó dùng `python tools/serial_logger.py COM5 -o lan1.csv`. Script ghi CSV kèm giờ máy tính, báo mã lượt trùng hoặc bị nhảy, và in tổng kết khi nhấn Ctrl+C.

## 8. Kiểm thử

**Trên máy tính** (không cần kit; cần gcc, ví dụ từ MSYS2 hoặc Git Bash + MinGW):

```sh
sh tests/run_tests.sh
```

96 ca kiểm thử bám theo mục 11 của đề tài: phân loại tại biên và gần biên, giữ túi lâu, chắn tia bằng tay, dao động, lỗi/quá tải, trừ bì, nhập ngoài phạm vi, hủy nhập, hiệu chuẩn, khởi động lại giữ cấu hình, thống kê khớp số bản ghi.

**Trên kit** (ghi kết quả vào báo cáo):

| Mục đề tài | Cách thử | Đạt khi |
| --- | --- | --- |
| Phân loại | Túi 244, 245, 250, 255, 256 g | THIẾU, ĐẠT, ĐẠT, ĐẠT, DƯ |
| Độ lặp | Cân cùng một vật 20 lần, ghi CSV | Độ lệch chuẩn và thời gian chờ ổn định ghi vào báo cáo |
| Giữ túi lâu | Để túi 30 s | Chỉ 1 dòng CSV |
| Chắn tia bằng tay | Che tia, không đặt túi | TỔNG không đổi, PH tăng |
| Dao động | Lắc nhẹ bàn cân | Không ghi kết quả |
| Lỗi | Rút dây DT của HX711 | "LOI: HX711 KHONG PHAN HOI", không ghi |
| Quá tải | Đặt > 900 g | "LOI: QUA TAI", không ghi |
| Khởi động lại | Đổi mục tiêu rồi nhấn RESET | Mục tiêu vẫn giữ giá trị mới |
| Đối chiếu | So TỔNG trên TFT với số dòng CSV | Bằng nhau, script báo 0 lỗi mã lượt |

## 9. Tài nguyên

| Mục | Debug | Release |
| --- | --- | --- |
| Flash | 42,9 KB (4,1 %) | 26,5 KB (2,5 %) |
| RAM (gồm heap FreeRTOS 32 KB) | 40,2 KB / 128 KB | 40,2 KB / 128 KB |

Sector 11 (0x080E0000) dành riêng cho cấu hình; `flash_store.c` từ chối ghi nếu firmware lấn sang vùng này.
