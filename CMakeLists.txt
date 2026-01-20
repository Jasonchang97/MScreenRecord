cmake_minimum_required(VERSION 3.14)

project(MScreenRecord VERSION 1.4.4 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 强制 UTF-8 编码，解决中文乱码
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Configure Version Info
string(REPLACE "." "," APP_VERSION_COMMA "${PROJECT_VERSION},0")
set(APP_VERSION "${PROJECT_VERSION}")
configure_file(app.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/app.rc @ONLY)

# Qt5 Config
set(CMAKE_PREFIX_PATH "D:/master/debug/xwares/3rd/qt5/build_x86/qtbase" "D:/master/debug/xwares/3rd/qt5/build_x86/qtmultimedia")

find_package(Qt5 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets Network Svg)

# 显式包含目录
include_directories(
    include
    D:/master/debug/xwares/3rd/qt5/build_x86/qtbase/include
    D:/master/debug/xwares/3rd/qt5/build_x86/qtmultimedia/include
    E:/VideoPlayer/module/VideoPlayer/lib/win32/ffmpeg/include
    E:/VideoPlayer/module/VideoPlayer/lib/win32/SDL2/include
)

link_directories(
    E:/VideoPlayer/module/VideoPlayer/lib/win32/ffmpeg/lib
    E:/VideoPlayer/module/VideoPlayer/lib/win32/SDL2/lib
)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/SelectionOverlay.cpp
    src/VideoUtils.cpp
    src/FloatingBall.cpp
    src/RecorderController.cpp
    src/HistoryManager.cpp
    src/SettingsDialog.cpp
    src/RangeSlider.cpp
    src/CustomMessageBox.cpp
    src/LogManager.cpp
    src/NativePlayerWidget.cpp
    src/CountdownOverlay.cpp
    src/HotkeyEdit.cpp
    src/GlobalHotkey.cpp
    src/ToastTip.cpp
    app.rc
)

set(HEADERS
    include/MainWindow.h
    include/SelectionOverlay.h
    include/VideoUtils.h
    include/FloatingBall.h
    include/RecorderController.h
    include/HistoryManager.h
    include/SettingsDialog.h
    include/RangeSlider.h
    include/CustomMessageBox.h
    include/NativePlayerWidget.h
    include/CountdownOverlay.h
    include/HotkeyEdit.h
    include/GlobalHotkey.h
    include/ToastTip.h
)

add_executable(MScreenRecord WIN32 MACOSX_BUNDLE ${SOURCES} ${HEADERS})

if(APPLE)
    set_target_properties(MScreenRecord PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "MScreenRecord"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.kso.mscreenrecord"
        # MACOSX_BUNDLE_ICON_FILE "app.icns" # Needs icns file
    )
endif()

# Link Libraries
    if(Qt5MultimediaWidgets_FOUND)
        target_link_libraries(MScreenRecord PRIVATE 
            Qt5::Widgets Qt5::Multimedia Qt5::MultimediaWidgets Qt5::Network Qt5::Svg
            avdevice avcodec avformat avutil swscale swresample SDL2
            dwmapi
        )
    else()
        target_link_libraries(MScreenRecord PRIVATE 
            Qt5::Widgets 
            Qt5::Multimedia 
            Qt5::Svg
            "D:/master/debug/xwares/3rd/qt5/build_x86/qtbase/lib/Qt5MultimediaWidgetsKso.lib"
            "D:/master/debug/xwares/3rd/qt5/build_x86/qtmultimedia/lib/Qt5MultimediaKso.lib"
            "D:/master/debug/xwares/3rd/qt5/build_x86/qtbase/lib/Qt5CoreKso.lib"
            "D:/master/debug/xwares/3rd/qt5/build_x86/qtbase/lib/Qt5GuiKso.lib"
            "D:/master/debug/xwares/3rd/qt5/build_x86/qtbase/lib/Qt5SvgKso.lib"
            avdevice avcodec avformat avutil swscale swresample SDL2
            dwmapi
        )
    endif()

target_include_directories(MScreenRecord PRIVATE include)
