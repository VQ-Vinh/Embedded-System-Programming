@echo off
setlocal EnableExtensions

REM ============================================================================
REM * Module: Flash a CMake-built STM32 firmware through ST-LINK over SWD.
REM * Usage : flash.bat [Debug^|Release]
REM ! The selected firmware must already exist before this script is executed.
REM ============================================================================

cd /d "%~dp0"

set "BUILD_CONFIG=%~1"
if not defined BUILD_CONFIG set "BUILD_CONFIG=Debug"

if /I "%BUILD_CONFIG%"=="Debug" (
    set "BUILD_CONFIG=Debug"
) else if /I "%BUILD_CONFIG%"=="Release" (
    set "BUILD_CONFIG=Release"
) else (
    echo [ERROR] Unsupported build configuration: %BUILD_CONFIG%
    echo Usage: %~nx0 [Debug^|Release]
    exit /b 2
)

set "FIRMWARE=%~dp0build\%BUILD_CONFIG%\Src.elf"
if not exist "%FIRMWARE%" (
    echo [ERROR] Firmware was not found: %FIRMWARE%
    echo [INFO]  Build it first with: build.bat %BUILD_CONFIG%
    exit /b 3
)

set "FLASH_TOOL="
set "FLASH_TOOL_KIND="

REM * Prefer STM32CubeProgrammer when available. STM32_PROGRAMMER_CLI may be
REM * used to provide its full executable path without changing PATH.
if defined STM32_PROGRAMMER_CLI if exist "%STM32_PROGRAMMER_CLI%" (
    set "FLASH_TOOL=%STM32_PROGRAMMER_CLI%"
    set "FLASH_TOOL_KIND=CubeProgrammer"
)

if not defined FLASH_TOOL where STM32_Programmer_CLI.exe >nul 2>&1 && (
    set "FLASH_TOOL=STM32_Programmer_CLI.exe"
    set "FLASH_TOOL_KIND=CubeProgrammer"
)

if not defined FLASH_TOOL if exist "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" (
    set "FLASH_TOOL=C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
    set "FLASH_TOOL_KIND=CubeProgrammer"
)

REM * Fall back to the legacy STM32 ST-LINK Utility CLI when CubeProgrammer is
REM * unavailable. STLINK_CLI may provide a custom executable path.
if not defined FLASH_TOOL if defined STLINK_CLI if exist "%STLINK_CLI%" (
    set "FLASH_TOOL=%STLINK_CLI%"
    set "FLASH_TOOL_KIND=StLinkUtility"
)

if not defined FLASH_TOOL where ST-LINK_CLI.exe >nul 2>&1 && (
    set "FLASH_TOOL=ST-LINK_CLI.exe"
    set "FLASH_TOOL_KIND=StLinkUtility"
)

if not defined FLASH_TOOL (
    echo [ERROR] No supported STM32 flash tool was found.
    echo [INFO]  Install STM32CubeProgrammer or add its CLI to PATH.
    exit /b 4
)

echo [INFO] Flashing %FIRMWARE% through ST-LINK SWD...

if "%FLASH_TOOL_KIND%"=="CubeProgrammer" (
    "%FLASH_TOOL%" -c port=SWD -w "%FIRMWARE%" -v -rst
) else (
    "%FLASH_TOOL%" -c SWD -P "%FIRMWARE%" -V -Rst
)

if errorlevel 1 (
    echo [ERROR] Flash operation failed.
    exit /b 5
)

echo [SUCCESS] Firmware flashed and target reset successfully.
exit /b 0
