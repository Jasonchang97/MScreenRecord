@echo off
setlocal
echo Starting MScreenRecord in Debug Mode...
echo Log file: %LOCALAPPDATA%\KSO\MScreenRecord\app_debug.log

:: 设置插件调试
set QT_DEBUG_PLUGINS=1

:: 尝试运行并将标准输出和错误输出重定向到 debug_console.txt
echo Running... > debug_console.txt
MScreenRecord.exe >> debug_console.txt 2>&1

echo Exit Code: %ERRORLEVEL% >> debug_console.txt
echo.
echo ==========================================
echo Application exited.
echo Please send 'debug_console.txt' to the developer.
echo ==========================================
pause
