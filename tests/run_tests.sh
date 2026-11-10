#!/bin/sh
# Biên dịch và chạy kiểm thử logic trên máy tính (Linux/macOS/MSYS2/Git Bash).
# Cần gcc. Trên Windows có thể dùng: gcc từ MSYS2 hoặc MinGW.
set -e
cd "$(dirname "$0")/.."
gcc -std=c11 -Wall -Wextra -Werror -O1 -IApp/Inc -IApp/Services \
    App/Services/fmt.c App/Services/scale_filter.c App/Services/weigh_fsm.c \
    App/Services/num_input.c App/Services/config.c App/Services/app_logic.c \
    tests/test_logic.c -o tests/test_logic
./tests/test_logic
