@echo off
setlocal enabledelayedexpansion

REM Optional flags:
REM   --no-run : do not auto launch after successful package
set "NO_RUN=0"
for %%A in (%*) do (
    if /I "%%~A"=="--no-run" set "NO_RUN=1"
)

REM Kill running app to avoid file locks / running old binary
taskkill /f /im MScreenRecord.exe >nul 2>nul

echo =======================================================
echo    MScreenRecord Installer Generator
echo =======================================================

REM --- 1. CONFIGURATION ---
set "BUILD_TYPE=Release"
set "PROJECT_ROOT=%cd%"
set "BUILD_DIR=%PROJECT_ROOT%\build"
set "DIST_ROOT=%PROJECT_ROOT%\dist"
set "TARGET_NAME=MScreenRecord"
set "VERSION=v1.4.1"
set "ARCH=x86"
set "TARGET_DIR_NAME=%TARGET_NAME%_%VERSION%_%ARCH%"
set "TARGET=%DIST_ROOT%\%TARGET_DIR_NAME%"

REM Path Config
set "QT_ROOT=D:\master\debug\xwares\3rd\qt5\build_x86\qtbase"
set "QT_MM_DIR=D:\master\debug\xwares\3rd\qt5\build_x86\qtmultimedia"
set "FFMPEG_X86_BIN=E:\VideoPlayer\bin32"

REM Clean Build
echo [STEP 1/4] Building application...
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
if exist "%DIST_ROOT%" rmdir /s /q "%DIST_ROOT%"
if exist "%PROJECT_ROOT%\MScreenRecord_v1.4_x86" (
    echo [INFO] Removing redundant root output directory...
    rmdir /s /q "%PROJECT_ROOT%\MScreenRecord_v1.4_x86"
)
mkdir "%BUILD_DIR%"
mkdir "%DIST_ROOT%"
mkdir "%TARGET%"

REM Compiler Config
echo [STEP 1] Checking compiler environment...
where cl.exe >nul 2>&1
if %errorlevel% neq 0 (
    echo [INFO] cl.exe not found. Searching for Visual Studio...
    set "VS_PATH="
    set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "!vswhere!" (
        for /f "usebackq tokens=*" %%i in (`"!vswhere!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
            set "VS_PATH=%%i"
        )
    )
    
    if defined VS_PATH (
        echo [INFO] Found VS at: !VS_PATH!
        if exist "!VS_PATH!\VC\Auxiliary\Build\vcvars32.bat" (
            call "!VS_PATH!\VC\Auxiliary\Build\vcvars32.bat"
        ) else (
            echo [ERROR] vcvars32.bat not found!
            pause
            exit /b 1
        )
    ) else (
        echo [ERROR] Visual Studio 2019/2022 not found!
        pause
        exit /b 1
    )
)
echo [INFO] Compiler (cl.exe) is ready.

REM Check Qt
echo [STEP 2] Verifying Qt paths...
if not exist "%QT_ROOT%\bin\qmake.exe" (
    echo [ERROR] Qt not found at: %QT_ROOT%
    pause
    exit /b 1
)

REM Check FFmpeg
echo [STEP 3] Verifying FFmpeg...
if not exist "%FFMPEG_X86_BIN%\avcodec-58.dll" (
    echo [ERROR] FFmpeg x86 binaries not found at: %FFMPEG_X86_BIN%
    pause
    exit /b 1
)

echo [STEP 4] Preparing build...
cd /d "%BUILD_DIR%"

echo [STEP 5] Configuring CMake...
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DQt5_DIR="%QT_ROOT%\lib\cmake\Qt5" -DQT_BASE_DIR="%QT_ROOT%" -DQT_MM_DIR="%QT_MM_DIR%" "%PROJECT_ROOT%"
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed.
    pause
    exit /b 1
)

echo [STEP 6] Building...
nmake
if %errorlevel% neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b 1
)

echo [STEP 7] Installing to dist...
copy /y "%BUILD_DIR%\MScreenRecord.exe" "%TARGET%\"
for %%F in ("%BUILD_DIR%\MScreenRecord.exe") do echo [INFO] build exe time: %%~tF
for %%F in ("%TARGET%\MScreenRecord.exe") do echo [INFO] dist  exe time: %%~tF

echo [STEP 8] Deploying dependencies...

echo [COPY] Copying resources...
if not exist "%TARGET%\resources" mkdir "%TARGET%\resources"
xcopy /s /y "%PROJECT_ROOT%\resources\*.*" "%TARGET%\resources\"
if exist "%PROJECT_ROOT%\launcher.bat" copy /y "%PROJECT_ROOT%\launcher.bat" "%TARGET%\"

echo [COPY] Copying FFmpeg x86 libraries...
copy /y "%FFMPEG_X86_BIN%\*.dll" "%TARGET%\"
REM Ensure ffmpeg.exe is available for RecorderController (assuming it's in bin32 too)
if not exist "%TARGET%\ffmpeg.exe" (
    if exist "%FFMPEG_X86_BIN%\ffmpeg.exe" copy /y "%FFMPEG_X86_BIN%\ffmpeg.exe" "%TARGET%\"
)

echo [COPY] Copying Virtual Audio Capturer DLL...
if exist "%PROJECT_ROOT%\3rd\virtual-audio\audio_sniffer.dll" copy /y "%PROJECT_ROOT%\3rd\virtual-audio\audio_sniffer.dll" "%TARGET%\"

echo [COPY] Copying Qt Libraries...
for %%F in (Qt5CoreKso.dll Qt5GuiKso.dll Qt5WidgetsKso.dll Qt5NetworkKso.dll Qt5MultimediaKso.dll Qt5MultimediaWidgetsKso.dll Qt5SvgKso.dll) do (
    if exist "%QT_ROOT%\bin\%%F" copy /y "%QT_ROOT%\bin\%%F" "%TARGET%\"
    if exist "%QT_MM_DIR%\bin\%%F" copy /y "%QT_MM_DIR%\bin\%%F" "%TARGET%\"
)
REM OpenGL support (for QOpenGLWidget in NativePlayerWidget)
REM Only copy software renderer for compatibility
if exist "%QT_ROOT%\bin\opengl32sw.dll" copy /y "%QT_ROOT%\bin\opengl32sw.dll" "%TARGET%\" 
if exist "%QT_ROOT%\bin\d3dcompiler_47.dll" copy /y "%QT_ROOT%\bin\d3dcompiler_47.dll" "%TARGET%\" 
REM libEGL and libGLESv2 are needed for software OpenGL renderer
if exist "%QT_ROOT%\bin\libEGL.dll" copy /y "%QT_ROOT%\bin\libEGL.dll" "%TARGET%\" 
if exist "%QT_ROOT%\bin\libGLESv2.dll" copy /y "%QT_ROOT%\bin\libGLESv2.dll" "%TARGET%\"

echo [COPY] Copying Plugins (Release only, essential plugins)...
if not exist "%TARGET%\plugins" mkdir "%TARGET%\plugins"

REM Copy only essential plugins (Release versions only, no debug)
REM Platforms: Only qwindows.dll (Windows platform plugin)
if not exist "%TARGET%\plugins\platforms" mkdir "%TARGET%\plugins\platforms"
if exist "%QT_ROOT%\plugins\platforms\qwindows.dll" copy /y "%QT_ROOT%\plugins\platforms\qwindows.dll" "%TARGET%\plugins\platforms\"

REM Audio: Only release versions
if not exist "%TARGET%\plugins\audio" mkdir "%TARGET%\plugins\audio"
if exist "%QT_ROOT%\plugins\audio\qtaudio_wasapi.dll" copy /y "%QT_ROOT%\plugins\audio\qtaudio_wasapi.dll" "%TARGET%\plugins\audio\"
if exist "%QT_ROOT%\plugins\audio\qtaudio_windows.dll" copy /y "%QT_ROOT%\plugins\audio\qtaudio_windows.dll" "%TARGET%\plugins\audio\"

REM Image formats: Only essential formats (jpeg, png, svg, ico)
if not exist "%TARGET%\plugins\imageformats" mkdir "%TARGET%\plugins\imageformats"
if exist "%QT_ROOT%\plugins\imageformats\qjpeg.dll" copy /y "%QT_ROOT%\plugins\imageformats\qjpeg.dll" "%TARGET%\plugins\imageformats\"
if exist "%QT_ROOT%\plugins\imageformats\qsvg.dll" copy /y "%QT_ROOT%\plugins\imageformats\qsvg.dll" "%TARGET%\plugins\imageformats\"
if exist "%QT_ROOT%\plugins\imageformats\qico.dll" copy /y "%QT_ROOT%\plugins\imageformats\qico.dll" "%TARGET%\plugins\imageformats\"

REM Icon engines: SVG icon support
if not exist "%TARGET%\plugins\iconengines" mkdir "%TARGET%\plugins\iconengines"
if exist "%QT_ROOT%\plugins\iconengines\qsvgicon.dll" copy /y "%QT_ROOT%\plugins\iconengines\qsvgicon.dll" "%TARGET%\plugins\iconengines\"

REM Media service: Only release versions (required for QMediaPlayer/QAudioInput)
if not exist "%TARGET%\plugins\mediaservice" mkdir "%TARGET%\plugins\mediaservice"
if exist "%QT_ROOT%\plugins\mediaservice\dsengine.dll" copy /y "%QT_ROOT%\plugins\mediaservice\dsengine.dll" "%TARGET%\plugins\mediaservice\"
if exist "%QT_ROOT%\plugins\mediaservice\wmfengine.dll" copy /y "%QT_ROOT%\plugins\mediaservice\wmfengine.dll" "%TARGET%\plugins\mediaservice\"
REM qtmedia_audioengine is in qtmultimedia plugins directory
if exist "%QT_MM_DIR%\plugins\mediaservice\qtmedia_audioengine.dll" (
    copy /y "%QT_MM_DIR%\plugins\mediaservice\qtmedia_audioengine.dll" "%TARGET%\plugins\mediaservice\"
) else if exist "%QT_ROOT%\plugins\mediaservice\qtmedia_audioengine.dll" (
    copy /y "%QT_ROOT%\plugins\mediaservice\qtmedia_audioengine.dll" "%TARGET%\plugins\mediaservice\"
)

REM Clean up any debug files, development files
del /s /q "%TARGET%\plugins\*.pdb" 2>nul
del /s /q "%TARGET%\plugins\*.exp" 2>nul
del /s /q "%TARGET%\plugins\*.lib" 2>nul
del /s /q "%TARGET%\plugins\*.ilk" 2>nul
del /s /q "%TARGET%\plugins\*d.dll" 2>nul

echo [CLEANUP] Removing unnecessary files...
REM Remove any remaining debug/development files from root
del /q "%TARGET%\*.exp" 2>nul
del /q "%TARGET%\*.lib" 2>nul
del /q "%TARGET%\*.ilk" 2>nul
del /q "%TARGET%\*.pdb" 2>nul

echo [COPY] Copying VC++ Runtime (x86 SAFE MODE)...
set "VC_REDIST_DIR="
for /f "tokens=*" %%i in ('where cl.exe') do set "VC_BIN=%%~dpi"
if exist "%VC_BIN%vcruntime140.dll" copy /y "%VC_BIN%vcruntime140.dll" "%TARGET%\"
if exist "%VC_BIN%msvcp140.dll" copy /y "%VC_BIN%msvcp140.dll" "%TARGET%\"
if exist "C:\Windows\SysWOW64\concrt140.dll" copy /y "C:\Windows\SysWOW64\concrt140.dll" "%TARGET%\"

echo [INFO] Creating qt.conf...
(
echo [Paths]
echo Plugins=plugins
) > "%TARGET%\qt.conf"

echo.
echo ==========================================
echo  SUCCESS. Software in: %TARGET_DIR_NAME%
echo ==========================================
echo.

if "%NO_RUN%"=="0" (
    echo [INFO] Launching %TARGET%\MScreenRecord.exe ...
    start "" "%TARGET%\MScreenRecord.exe"
)

cd /d "%PROJECT_ROOT%"
