# Embedded System Programming

Firmware FreeRTOS cho STM32F407VGT6, được cấu hình bằng STM32CubeMX và build bằng CMake trên Windows.

## 1. Công cụ cần thiết

Mỗi thành viên cần cài đặt các công cụ sau trước khi làm việc với repository:

| Công cụ | Phiên bản khuyến nghị | Mục đích |
| --- | --- | --- |
| Git | Bản ổn định mới | Quản lý source code và branch |
| CMake | 3.22 trở lên | Cấu hình project và điều phối build |
| Ninja | Bản ổn định mới | Build project từ cấu hình CMake |
| Arm GNU Toolchain | 14.2.Rel1 | Biên dịch firmware cho Cortex-M4F |
| STM32CubeMX | 6.16.1 | Chỉnh sửa cấu hình trong `Src.ioc` |
| STM32CubeF4 | 1.28.3 | HAL, CMSIS và FreeRTOS middleware |
| STM32CubeProgrammer | Bản ổn định mới | Flash firmware qua ST-LINK/SWD |

`flash.bat` cũng hỗ trợ STM32 ST-LINK Utility CLI cũ nếu máy chưa có STM32CubeProgrammer.

Kiểm tra các công cụ build trong PowerShell:

```powershell
git --version
cmake --version
ninja --version
```

`build.bat` sẽ kiểm tra ARM GCC và tự tìm toolchain trong các thư mục cài đặt chuẩn của Windows. Nếu compiler được cài ở vị trí khác, khai báo thư mục `bin` trước khi build:

```powershell
$env:ARM_GCC_PATH = "C:\Tools\ArmGNU\bin"
```

## 2. Lấy source code và tạo branch làm việc

Clone repository:

```powershell
git clone https://github.com/VQ-Vinh/Embedded-System-Programming.git
cd Embedded-System-Programming
```

Không làm việc trực tiếp trên branch `main`. Trước mỗi task, cập nhật `main` rồi tạo branch mới:

```powershell
git switch main
git pull origin main
git switch -c feature/<ten-chuc-nang>
```

Quy ước tên branch:

- `feature/<ten>`: thêm chức năng.
- `fix/<ten>`: sửa lỗi.
- `docs/<ten>`: cập nhật tài liệu.
- `chore/<ten>`: build script, công cụ hoặc công việc bảo trì.

Ví dụ:

```powershell
git switch -c feature/uart-console
```

Chia thay đổi thành các commit nhỏ theo từng cấu hình, module hoặc chức năng độc lập:

```powershell
git add <cac-file-lien-quan>
git commit -m "feat: add UART console task"
git push -u origin feature/uart-console
```

Sau khi push, tạo Pull Request để review và merge vào `main`.

## 3. Thay đổi cấu hình STM32CubeMX

Khi thay đổi clock, GPIO, peripheral, DMA, interrupt hoặc cấu hình FreeRTOS:

1. Mở `Src.ioc` bằng STM32CubeMX.
2. Thực hiện cấu hình trong CubeMX.
3. Generate lại source code.
4. Kiểm tra thay đổi trước khi viết phần logic ứng dụng.
5. Chỉ thêm code vào các vùng `USER CODE BEGIN` / `USER CODE END` trong file do CubeMX quản lý.

Không chỉnh tay cấu hình phần cứng trong code nếu thay đổi đó có thể thực hiện bằng CubeMX.

## 4. Build firmware

Mở PowerShell tại thư mục gốc repository.

Build Debug mặc định:

```powershell
.\build.bat
```

Chọn rõ cấu hình build:

```powershell
.\build.bat Debug
.\build.bat Release
```

Xóa kết quả cũ trước khi build lại:

```powershell
.\build.bat Debug --clean-first
```

Firmware sau khi build nằm tại:

```text
build\Debug\Src.elf
build\Debug\Src.hex
build\Debug\Src.bin
build\Release\Src.elf
build\Release\Src.hex
build\Release\Src.bin
```

Thư mục `build` là output cục bộ và không được commit lên Git.

## 5. Flash firmware lên kit

Kết nối ST-LINK với kit qua SWD và bảo đảm target đã được cấp nguồn. Build firmware trước, sau đó flash đúng cấu hình:

```powershell
.\build.bat Debug
.\flash.bat Debug
```

Với bản Release:

```powershell
.\build.bat Release
.\flash.bat Release
```

Script sẽ thực hiện các bước:

1. Kiểm tra file `Src.elf` tương ứng.
2. Tìm STM32CubeProgrammer CLI hoặc ST-LINK Utility CLI.
3. Kết nối qua SWD, ghi firmware và verify.
4. Reset vi điều khiển sau khi flash thành công.

Nếu flash không kết nối được, kiểm tra nguồn target, dây `SWDIO`, `SWCLK`, `GND`, `NRST` và driver ST-LINK.

## 6. Cấu trúc chính của repository

```text
Core/                       Mã khởi tạo và mã ứng dụng
Drivers/                    CMSIS và STM32 HAL
Middlewares/                FreeRTOS và CMSIS-RTOS v2
cmake/                      Toolchain và CMake sinh bởi CubeMX
Src.ioc                     Cấu hình STM32CubeMX
CMakeLists.txt              Cấu hình CMake cấp project
CMakePresets.json           Preset Debug và Release
build.bat                   Build firmware
flash.bat                   Flash firmware qua ST-LINK
```
