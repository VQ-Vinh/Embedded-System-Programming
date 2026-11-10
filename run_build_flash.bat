@echo off
REM * Build Debug roi nap len kit qua ST-LINK; ghi log vao build\run_log.txt
cd /d "%~dp0"
if not exist build mkdir build
set "LOG=build\run_log.txt"
echo ==== BUILD %date% %time% ==== > "%LOG%"
call build.bat Debug --clean-first >> "%LOG%" 2>&1
set "RC=%errorlevel%"
if not "%RC%"=="0" goto done
echo ==== FLASH ==== >> "%LOG%"
call flash.bat Debug >> "%LOG%" 2>&1
set "RC=%errorlevel%"
:done
echo ==== EXIT %RC% ==== >> "%LOG%"
type "%LOG%"
timeout /t 15
