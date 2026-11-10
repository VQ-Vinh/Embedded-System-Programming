@AGENTS.md

# Ghi chú dự án

- Đề tài: cân kiểm tra khối lượng STM32F4 Discovery + FreeRTOS. Hướng dẫn đầy đủ: docs/HUONG_DAN.md
- Mã ứng dụng nằm trong App/ (Drivers, Services, Tasks); thông số chỉnh trong App/Inc/app_config.h
- Build: .\build.bat Debug | Nạp: .\flash.bat Debug | Build + nạp + ghi log: .\run_build_flash.bat (log ở build\run_log.txt)
- Kiểm thử logic trên máy tính: sh tests/run_tests.sh (cần gcc)
