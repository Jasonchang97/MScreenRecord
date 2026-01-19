@echo off
setlocal EnableDelayedExpansion

set "DIST_DIR=QtScreenRecorder_v1.2_x86"
set "LOG_FILE=diagnostic_report.txt"

echo ========================================== > "%LOG_FILE%"
echo      QtScreenRecorder Diagnostic Tool      >> "%LOG_FILE%"
echo ========================================== >> "%LOG_FILE%"
echo Date: %DATE% %TIME% >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

echo [1] Checking Files in %DIST_DIR%... >> "%LOG_FILE%"
if not exist "%DIST_DIR%" (
    echo [ERROR] Dist directory not found! >> "%LOG_FILE%"
    goto :END
)

dir "%DIST_DIR%" >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

echo [2] Checking Plugins... >> "%LOG_FILE%"
dir "%DIST_DIR%\plugins\platforms" >> "%LOG_FILE%"
dir "%DIST_DIR%\plugins\mediaservice" >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

echo [3] Testing FFmpeg... >> "%LOG_FILE%"
"%DIST_DIR%\ffmpeg.exe" -version >> "%LOG_FILE%" 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] FFmpeg execution failed! Exit code: %ERRORLEVEL% >> "%LOG_FILE%"
) else (
    echo [OK] FFmpeg is working. >> "%LOG_FILE%"
)
echo. >> "%LOG_FILE%"

echo [4] Testing Main Application... >> "%LOG_FILE%"
echo Running %DIST_DIR%\QtScreenRecorder.exe...
echo (If it crashes or shows an error dialog, please describe it)
echo. >> "%LOG_FILE%"

:: 尝试运行应用程序，不阻塞
start "" "%DIST_DIR%\QtScreenRecorder.exe"

echo.
echo ==========================================
echo Diagnostic completed. 
echo Please check the file: %LOG_FILE%
echo And tell me if the application window appeared.
echo ==========================================
pause
:END
