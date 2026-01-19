@echo off
setlocal EnableDelayedExpansion

echo =======================================================
echo     MScreenRecord Installer Generator
echo =======================================================
echo.

set "DIST_DIR=dist\MScreenRecord_v1.4.2_x86"
set "ISS_FILE=setup_script.iss"
set "ISCC_EXE="

:: ---------------------------------------------------------
:: 1. Build Application
:: ---------------------------------------------------------
echo [STEP 1/4] Building application...
call package_win.bat
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Application build failed.
    pause
    exit /b 1
)

:: ---------------------------------------------------------
:: 2. Diagnostic Run
:: ---------------------------------------------------------
echo.
echo [STEP 2/4] Diagnostic Run...
echo.
echo ---------------------------------------------------------------
echo  Please watch for any SYSTEM ERROR POPUPS (Missing DLLs).
echo  If the application starts successfully, please CLOSE IT manually
echo  to continue the packaging process.
echo ---------------------------------------------------------------
echo.
echo Running %DIST_DIR%\MScreenRecord.exe ...
start "" "%DIST_DIR%\MScreenRecord.exe"
echo.
echo [INFO] Application launched. Please close it to continue...
pause

:: ---------------------------------------------------------
:: 3. Find Inno Setup Compiler
:: ---------------------------------------------------------
echo.
echo [STEP 3/4] Searching for Inno Setup Compiler (ISCC.exe)...

if exist "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" (
    set "ISCC_EXE=C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
    goto :FOUND_ISCC
)
if exist "C:\Program Files\Inno Setup 6\ISCC.exe" (
    set "ISCC_EXE=C:\Program Files\Inno Setup 6\ISCC.exe"
    goto :FOUND_ISCC
)

for /f "tokens=2*" %%A in ('reg query "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Inno Setup 6_is1" /v "InstallLocation" 2^>nul') do (
    set "ISCC_DIR=%%B"
    if exist "!ISCC_DIR!\ISCC.exe" (
        set "ISCC_EXE=!ISCC_DIR!\ISCC.exe"
        goto :FOUND_ISCC
    )
)

:NOT_FOUND_ISCC
echo [WARNING] Inno Setup Compiler (ISCC.exe) NOT found!
echo.
echo Please install Inno Setup 6 from: https://jrsoftware.org/isdl.php
echo The raw application files are ready in: %DIST_DIR%
pause
exit /b 1

:FOUND_ISCC
echo [INFO] Found ISCC at: "!ISCC_EXE!"

:: ---------------------------------------------------------
:: 4. Compile Installer
:: ---------------------------------------------------------
echo.
echo [STEP 4/4] Compiling installer script...
"!ISCC_EXE!" "%ISS_FILE%"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo =======================================================
    echo [SUCCESS] Installer generated successfully!
    echo File: MScreenRecord_Setup_v1.4.2.exe
    echo.
    echo NOTE: If desktop icon is generic, it's because app.ico 
    echo       is missing in source directory.
    echo =======================================================
    echo.
) else (
    echo.
    echo [ERROR] Inno Setup compilation failed.
)

pause
