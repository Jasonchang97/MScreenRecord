@echo off
setlocal enabledelayedexpansion
echo =================================================
echo   Unregister Virtual Audio Capturer (Smart Search)
echo =================================================

:: 1. Check Admin Rights
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [INFO] Requesting Administrator privileges...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

:: 2. Setup 32-bit regsvr32
set "REGSVR=%SystemRoot%\SysWOW64\regsvr32.exe"
if not exist "!REGSVR!" (
    set "REGSVR=regsvr32.exe"
)
echo [INFO] Using regsvr32: !REGSVR!

:: 3. Find and Unregister
set "SCRIPT_DIR=%~dp0"
set "FOUND_ANY=0"

echo [SEARCH] Looking for audio_sniffer.dll in %SCRIPT_DIR% ...

for /r "%SCRIPT_DIR%" %%f in (audio_sniffer.dll) do (
    echo.
    echo [FOUND] %%f
    set "FOUND_ANY=1"
    
    echo   Executing unregister...
    "!REGSVR!" /u /s "%%f"
    
    if !errorlevel! equ 0 (
        echo   [OK] Command successful.
    ) else (
        echo   [ERR] Command failed with code !errorlevel!.
    )
)

if "!FOUND_ANY!"=="0" (
    echo.
    echo [WARN] audio_sniffer.dll not found in any subdirectory!
    echo        Cannot perform standard unregister without the DLL file.
    echo        Please make sure the project is built or the 3rd folder exists.
    goto :VERIFY
)

:: 4. Verify
:VERIFY
echo.
echo [VERIFICATION] Checking DirectShow device list...

:: Try typical ffmpeg locations
set "FFMPEG_PATH=E:\VideoPlayer\bin32\ffmpeg.exe"
if not exist "!FFMPEG_PATH!" (
    for %%i in (ffmpeg.exe) do set "FFMPEG_PATH=%%~$PATH:i"
)

if "!FFMPEG_PATH!"=="" (
    echo [WARN] FFmpeg not found. Cannot verify automatically.
    goto :END
)

"!FFMPEG_PATH!" -list_devices true -f dshow -i dummy 2> temp_devices.txt
findstr /C:"virtual-audio-capturer" temp_devices.txt >nul
if !errorlevel! equ 0 (
    echo.
    echo [FAILURE] "virtual-audio-capturer" is STILL detected!
    echo           It might be registered from a path outside this project.
) else (
    echo.
    echo [SUCCESS] "virtual-audio-capturer" is NO LONGER detected.
)
if exist temp_devices.txt del temp_devices.txt

:END
echo.
pause
