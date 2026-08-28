@echo off
setlocal EnableExtensions

REM ============================================================================
REM * Module: CMake build entry point for the STM32 firmware.
REM * Usage : build.bat [Debug^|Release] [--clean-first]
REM ! CMake and Ninja must be available in PATH. ARM GCC is auto-detected from
REM ! common installation directories or can be supplied through ARM_GCC_PATH.
REM ============================================================================

cd /d "%~dp0"

set "BUILD_CONFIG=%~1"
set "BUILD_OPTION=%~2"

if not defined BUILD_CONFIG set "BUILD_CONFIG=Debug"

if /I "%BUILD_CONFIG%"=="Debug" (
    set "BUILD_CONFIG=Debug"
) else if /I "%BUILD_CONFIG%"=="Release" (
    set "BUILD_CONFIG=Release"
) else (
    echo [ERROR] Unsupported build configuration: %BUILD_CONFIG%
    echo Usage: %~nx0 [Debug^|Release] [--clean-first]
    exit /b 2
)

if defined BUILD_OPTION if /I not "%BUILD_OPTION%"=="--clean-first" (
    echo [ERROR] Unsupported build option: %BUILD_OPTION%
    echo Usage: %~nx0 [Debug^|Release] [--clean-first]
    exit /b 2
)

REM * Auto-detect standalone Arm GNU Toolchain installations. The explicit
REM * ARM_GCC_PATH environment variable always takes precedence.
if not defined ARM_GCC_PATH for /d %%D in ("C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\*") do if exist "%%~fD\bin\arm-none-eabi-gcc.exe" if not defined ARM_GCC_PATH set "ARM_GCC_PATH=%%~fD\bin"
if not defined ARM_GCC_PATH for /d %%D in ("C:\Program Files\Arm GNU Toolchain arm-none-eabi\*") do if exist "%%~fD\bin\arm-none-eabi-gcc.exe" if not defined ARM_GCC_PATH set "ARM_GCC_PATH=%%~fD\bin"

REM * Add the selected compiler directory only for the lifetime of this script.
if defined ARM_GCC_PATH set "PATH=%ARM_GCC_PATH%;%PATH%"

where cmake >nul 2>&1 || (
    echo [ERROR] CMake was not found in PATH.
    exit /b 3
)

where ninja >nul 2>&1 || (
    echo [ERROR] Ninja was not found in PATH.
    exit /b 3
)

where arm-none-eabi-gcc >nul 2>&1 || (
    echo [ERROR] arm-none-eabi-gcc was not found in PATH.
    echo [INFO]  Add its bin directory to PATH or define ARM_GCC_PATH.
    exit /b 3
)

echo [INFO] Configuring %BUILD_CONFIG%...
cmake --preset "%BUILD_CONFIG%"
if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    exit /b 4
)

echo [INFO] Building %BUILD_CONFIG%...
if /I "%BUILD_OPTION%"=="--clean-first" (
    cmake --build --preset "%BUILD_CONFIG%" --clean-first
) else (
    cmake --build --preset "%BUILD_CONFIG%"
)

if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 5
)

echo [SUCCESS] Firmware built at build\%BUILD_CONFIG%\Src.elf
exit /b 0
