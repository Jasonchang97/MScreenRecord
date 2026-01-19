@echo off
set "DIST_DIR=QtScreenRecorder_v1.2_x86"

echo ==========================================
echo        File Integrity Check
echo ==========================================
echo.

if not exist "%DIST_DIR%" (
    echo [ERROR] Directory %DIST_DIR% does not exist!
    pause
    exit /b
)

echo [1] Listing all .dll files in %DIST_DIR%...
echo ------------------------------------------
dir /b "%DIST_DIR%\*.dll"
echo ------------------------------------------
echo.

echo [2] Listing plugins\platforms...
echo ------------------------------------------
dir /b "%DIST_DIR%\plugins\platforms\*.dll" 2>nul
if %ERRORLEVEL% NEQ 0 echo [WARNING] platforms dir empty or missing!
echo ------------------------------------------
echo.

echo [3] Checking key files...
if exist "%DIST_DIR%\QtScreenRecorder.exe" (echo [OK] EXE found) else (echo [FAIL] EXE missing)
if exist "%DIST_DIR%\qt.conf" (echo [OK] qt.conf found) else (echo [FAIL] qt.conf missing)

pause
