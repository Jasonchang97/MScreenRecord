@echo off
setlocal

set "DIST_DIR=QtScreenRecorder_v1.1_x86"
set "EXE_PATH=%DIST_DIR%\QtScreenRecorder.exe"

echo ==========================================
echo      QtScreenRecorder 启动诊断工具
echo ==========================================
echo.

if not exist "%EXE_PATH%" (
    echo [错误] 找不到文件: %EXE_PATH%
    echo 请先运行 package_win.bat 进行编译!
    pause
    exit /b
)

echo [1] 检查关键文件...
if exist "%DIST_DIR%\Qt5Core.dll" ( echo   [OK] Qt5Core.dll ) else ( 
    if exist "%DIST_DIR%\Qt5CoreKso.dll" ( echo   [OK] Qt5CoreKso.dll ) else ( echo   [FAIL] 缺失 Qt5Core*.dll )
)

if exist "%DIST_DIR%\plugins\platforms\qwindows.dll" ( echo   [OK] platforms\qwindows.dll ) else ( echo   [FAIL] 缺失 plugins\platforms\qwindows.dll )
if exist "%DIST_DIR%\qt.conf" ( echo   [OK] qt.conf ) else ( echo   [FAIL] 缺失 qt.conf )
if exist "%DIST_DIR%\opengl32sw.dll" ( echo   [OK] opengl32sw.dll ) else ( echo   [警告] 缺失 opengl32sw.dll )

echo.
echo [2] 正在尝试运行程序 (捕获输出)...
echo --------------------------------------------------

:: 设置 Qt 调试环境变量
set QT_DEBUG_PLUGINS=1
set QT_LOGGING_RULES=*.debug=true

cd "%DIST_DIR%"
QtScreenRecorder.exe > debug_log.txt 2>&1

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [严重] 程序异常退出! 退出代码: %ERRORLEVEL%
) else (
    echo.
    echo [信息] 程序运行结束 (或已关闭)。
)

echo.
echo --------------------------------------------------
echo [3] 日志内容 (最后 20 行):
echo --------------------------------------------------
if exist debug_log.txt (
    powershell -command "Get-Content debug_log.txt -Tail 20"
) else (
    echo 无日志文件生成。这通常意味着程序在加载任何 Qt 代码前就崩溃了（缺少依赖 DLL）。
)

echo.
echo ==========================================
echo 诊断完成。请将以上信息（特别是日志部分）反馈给我。
echo ==========================================
pause
