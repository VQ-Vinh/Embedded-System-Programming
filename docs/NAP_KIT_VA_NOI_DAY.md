# Nạp firmware và nối dây cho kit STM32F4 Discovery

Tài liệu này dành cho người mới lấy code về. Nó trả lời hai câu hỏi: **nạp file nào lên kit** và **nối các module vào chân nào**. Phần vận hành, hiệu chuẩn và kiểm thử xem ở [HUONG_DAN.md](HUONG_DAN.md).

Kit: **STM32F407G-DISC1** (chip STM32F407VGT6, ST-LINK/V2 có sẵn trên board).

---

## 1. Từ lúc clone đến lúc nạp

```powershell
git clone https://github.com/VQ-Vinh/Embedded-System-Programming.git
cd Embedded-System-Programming
git checkout <nhánh của nhóm>

.\build.bat Debug      # sinh build\Debug\Src.elf / Src.hex / Src.bin
.\flash.bat Debug      # nạp Src.elf qua ST-LINK rồi reset kit
```

Hoặc gộp cả hai bước: `.\run_build_flash.bat`. Log nằm ở `build\run_log.txt`.

> ! Thư mục `build/` **không** có trên git. Sau khi clone, mỗi người phải tự build một lần. Lần build này cũng sinh ra `build/Debug/compile_commands.json`, file mà VS Code IntelliSense dùng (xem mục 5).

Công cụ cần cài (chi tiết trong [README.md](../README.md)): CMake ≥ 3.22, Ninja, Arm GNU Toolchain (`arm-none-eabi-gcc`), và STM32CubeProgrammer để nạp.

---

## 2. Nên nạp file nào?

Sau khi build, `build\Debug\` (hoặc `build\Release\`) có ba file firmware. **Cả ba chứa cùng một chương trình**; chúng chỉ khác định dạng.

| File | Có địa chỉ nạp bên trong? | Dùng khi nào | Khuyên dùng |
| --- | --- | --- | --- |
| `Src.elf` | Có | `flash.bat`, debug bằng VS Code / CubeIDE, STM32CubeProgrammer | ✅ **Mặc định** (script dùng file này) |
| `Src.hex` | Có | Nạp tay bằng STM32CubeProgrammer GUI hoặc ST-LINK Utility; gửi cho bạn khác nạp | ✅ Khi nạp bằng tay |
| `Src.bin` | **Không** | Chỉ khi công cụ bắt buộc dùng file nhị phân | ⚠️ Phải tự nhập địa chỉ **`0x08000000`** |

**Debug hay Release?**

- **Debug** (`.\build.bat Debug`): tối ưu `-O0`, có thông tin debug, đặt breakpoint được. Dùng khi phát triển và thử trên kit.
- **Release** (`.\build.bat Release`): nhỏ và nhanh hơn. Dùng khi demo hoặc nộp bài.

> * Nếu bạn không cài toolchain, có thể nhờ người khác gửi file `Src.hex` rồi nạp bằng STM32CubeProgrammer theo mục 3.2.

---

## 3. Cách nạp

Trước khi nạp:

1. Cắm cáp **mini-USB** vào cổng **CN1 (USB ST-LINK)** ở phía trên của kit. Không dùng cổng micro-USB phía dưới, vì cổng đó là USB OTG của chip.
2. Hai jumper ở **CN3** phải được **cắm cả hai**, để ST-LINK nạp cho chip trên board.
3. Jumper **JP1 (IDD)** phải được cắm, nếu không chip sẽ không có nguồn.
4. LED **LD1 (COM)** sáng đỏ/xanh là ST-LINK đã nhận.

### 3.1. Bằng script (khuyên dùng)

```powershell
.\flash.bat Debug
```

Script tự tìm STM32CubeProgrammer CLI (hoặc ST-LINK Utility), nạp `Src.elf`, kiểm tra lại (`-v`) và reset kit. Khi thành công sẽ thấy dòng `[SUCCESS] Firmware flashed and target reset successfully.`

### 3.2. Bằng STM32CubeProgrammer GUI

1. Mở STM32CubeProgrammer. Ở góc phải, chọn **ST-LINK**, Port **SWD**, rồi bấm **Connect**.
2. Chọn tab **Erasing & Programming** (biểu tượng mũi tên xuống).
3. **File path**: chọn `build\Debug\Src.hex` (hoặc `Src.elf`).
   - Nếu chọn `Src.bin`, phải điền **Start address = `0x08000000`**.
4. Tích **Verify programming** và **Run after programming**, rồi bấm **Start Programming**.
5. Bấm **Disconnect** trước khi chạy `flash.bat` hoặc mở debug, vì hai chương trình không dùng chung ST-LINK được.

> ! **Đừng bấm "Full chip erase"** nếu không cần. Cấu hình (mục tiêu, sai số, hệ số hiệu chuẩn) được lưu ở **sector 11 (0x080E0000)**. Xóa toàn chip sẽ mất các giá trị này và phải hiệu chuẩn lại. Nạp bình thường chỉ xóa các sector mà firmware dùng (khoảng 43 KB đầu), nên cấu hình vẫn được giữ.

### 3.3. Lỗi nạp hay gặp

| Hiện tượng | Cách xử lý |
| --- | --- |
| `No STLink detected` / `No debug probe` | Cắm lại cáp mini-USB, thử cáp khác (nhiều cáp chỉ có dây sạc), cài driver ST-LINK (có sẵn trong bộ cài STM32CubeProgrammer) |
| `ST-LINK is already in use` | Đóng STM32CubeProgrammer GUI hoặc phiên debug đang mở |
| `Target not found` / `Cannot connect to target` | Kiểm tra jumper CN3 và JP1; giữ nút **RESET (đen)** rồi bấm nạp, và thả nút khi CubeProgrammer bắt đầu kết nối. Hoặc chọn **Mode: Under reset** trong GUI |
| `Firmware was not found` | Chưa build: chạy `.\build.bat Debug` trước |
| Nạp xong nhưng không chạy | Bấm nút RESET đen trên kit |

---

## 4. Sơ đồ nối dây

Các chân dưới đây khớp với `Src.ioc` / `Core/Inc/main.h`. **Đừng đổi chân trong code**. Nếu cần đổi chân, sửa trong CubeMX rồi sinh lại code (xem `AGENTS.md`).

### 4.1. Nguồn trên kit

Hai hàng header lớn **P1** (bên trái) và **P2** (bên phải) đều có chân **`3V`**, **`5V`** và **`GND`** (in trên lụa của board).

- **3V**: cấp cho các module logic (TFT, HX711, cảm biến quang).
- **5V**: chỉ dùng cho còi, và cho cảm biến quang nếu module đó yêu cầu 5 V.
- ! **Mọi module phải chung GND với kit**, kể cả USB-UART.

### 4.2. Bảng nối dây đầy đủ

| Module | Chân trên module | Nối tới | Ghi chú |
| --- | --- | --- | --- |
| **TFT SPI** (ST7735 1,8" hoặc ILI9341 2,4–2,8") | VCC | 3V | |
| | GND | GND | |
| | CS | **PB12** | |
| | RESET / RES / RST | **PB1** | |
| | A0 / DC / RS | **PB11** | |
| | SDA / SDI / MOSI | **PB15** | SPI2 MOSI |
| | SCK / SCL / CLK | **PB13** | SPI2 SCK |
| | LED / BL | 3V | Đèn nền |
| | SDO / MISO (nếu có) | *để trống* | Firmware chỉ ghi, không đọc |
| **HX711** | VCC | 3V | Cấp 3,3 V để mức logic của DT là 3,3 V |
| | GND | GND | |
| | DT / DOUT | **PE7** | Vào |
| | SCK / PD_SCK | **PE8** | Ra |
| | E+ / E− / A− / A+ | Load cell: Đỏ / Đen / Trắng / Xanh lá | Màu dây có thể khác tùy load cell. Nếu cân ra số âm khi đặt vật, đổi A+ ↔ A− |
| **Cảm biến quang** | VCC | 3V (hoặc 5V nếu module yêu cầu) | PE9 chịu được 5 V |
| | GND | GND | |
| | OUT / DO | **PE9** | Đã bật pull-up nội. Mặc định mức **thấp = có vật** (`PHOTO_ACTIVE_LEVEL = 0`) |
| **LED xanh** (ĐẠT) | Anode (+, chân dài) | **PE10** qua điện trở **330 Ω** | Cathode (−) → GND |
| **LED đỏ** (LỖI / THIẾU / DƯ) | Anode (+, chân dài) | **PE11** qua điện trở **330 Ω** | Cathode (−) → GND |
| **Còi chủ động 5 V** qua transistor **S8050** | Base S8050 | **PE12** qua điện trở **1 kΩ** | |
| | Emitter S8050 | GND | |
| | Collector S8050 | Chân (−) của còi | |
| | Chân (+) của còi | 5V | |
| **Keypad 4×4** | R1 / R2 / R3 / R4 (hàng) | **PD0 / PD1 / PD2 / PD3** | Ra, quét mức thấp |
| | C1 / C2 / C3 / C4 (cột) | **PD6 / PD7 / PD8 / PD9** | Vào, đã bật pull-up nội |
| **USB-UART** (CP2102, CH340, PL2303…) | RX | **PA2** (USART2 TX) | 115200 8N1 |
| | TX | **PA3** (USART2 RX) | |
| | GND | GND | |
| | VCC / 5V / 3V3 | ! **KHÔNG nối** | Kit đã có nguồn từ cáp ST-LINK |

> * **Keypad:** với loại keypad màng 8 chân phổ biến, nhìn từ mặt phím, 4 chân bên trái là **R1–R4** và 4 chân bên phải là **C1–C4**. Nếu bấm phím mà ra ký tự sai (ví dụ hàng và cột bị đổi chỗ), đảo thứ tự dây cho khớp.

> * **UART nối chéo:** chân RX của module nối vào chân TX của kit (PA2), và ngược lại.

### 4.3. Sơ đồ tóm tắt

```text
                        STM32F407G-DISC1
                 ┌──────────────────────────────┐
  TFT  CS  ──────┤ PB12                    PE7  ├────── DT   HX711
       RES ──────┤ PB1                     PE8  ├────── SCK  HX711
       DC  ──────┤ PB11                    PE9  ├────── OUT  Cảm biến quang
       SDA ──────┤ PB15 (SPI2 MOSI)        PE10 ├──330Ω──▶|── GND  LED xanh
       SCK ──────┤ PB13 (SPI2 SCK)         PE11 ├──330Ω──▶|── GND  LED đỏ
                 │                         PE12 ├──1kΩ── B S8050 (còi 5V)
  Keypad R1..R4 ─┤ PD0  PD1  PD2  PD3           │
         C1..C4 ─┤ PD6  PD7  PD8  PD9           │
                 │                         PA2  ├────── RX  USB-UART
                 │                         PA3  ├────── TX  USB-UART
                 │  3V   5V   GND               │
                 └──┬────┬────┬─────────────────┘
                    │    │    └── GND chung cho TẤT CẢ module
                    │    └─────── chỉ cấp cho còi (và cảm biến quang loại 5 V)
                    └──────────── VCC TFT, LED nền TFT, VCC HX711, VCC cảm biến quang
```

### 4.4. Chân KHÔNG được dùng

Các chân sau đã nối với linh kiện có sẵn trên Discovery (MEMS, audio DAC, USB OTG, LED on-board, SWD...), nên không cắm module vào:

`PA0, PA4–PA7, PA9–PA14, PB2, PB3, PB6, PB9, PB10, PC0, PC3, PC7, PC10, PC12, PD4, PD5, PD12–PD15, PE3`

> ! **PA13 / PA14** là SWDIO / SWCLK để nạp. Nếu cắm module vào hai chân này, kit sẽ không nạp được nữa.

### 4.5. Kiểm tra trước khi cắm nguồn

- [ ] Không có dây nào nối **5V** vào chân logic của TFT hoặc HX711.
- [ ] Mọi module đều có **GND chung** với kit.
- [ ] USB-UART **không** nối VCC.
- [ ] LED có điện trở 330 Ω, đúng chiều (chân dài vào phía PE10/PE11).
- [ ] Còi đi qua transistor, **không** nối thẳng vào PE12.
- [ ] Đã chọn đúng loại màn hình trong `App/Inc/app_config.h` (`TFT_CONTROLLER_ST7735` hoặc `TFT_CONTROLLER_ILI9341`).

Sau đó làm theo bảng "Chạy lần đầu" ở mục 4 của [HUONG_DAN.md](HUONG_DAN.md), cắm từng module một.

---

## 5. VS Code báo đỏ include sau khi clone?

Repo đã có sẵn `.vscode/c_cpp_properties.json`, file này trỏ IntelliSense tới `build/Debug/compile_commands.json`. Nếu vẫn báo đỏ:

1. Chạy `.\build.bat Debug` một lần để sinh `compile_commands.json`.
2. `Ctrl+Shift+P` → **Developer: Reload Window**.
3. Nếu vẫn còn đỏ: `Ctrl+Shift+P` → **C/C++: Select a Configuration** → chọn `STM32F407 (CMake Debug)`.

Mỗi khi thêm file `.c` mới vào `CMakeLists.txt`, build lại để IntelliSense cập nhật.
