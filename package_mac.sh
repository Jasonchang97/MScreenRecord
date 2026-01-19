#!/bin/bash
# Mac Build Script for MScreenRecord

BUILD_DIR="build_mac"
APP_NAME="MScreenRecord"

# Clean
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR

# Check dependencies
if ! command -v qmake &> /dev/null; then
    echo "Qt (qmake) not found. Please install Qt."
    exit 1
fi

# CMake (Assuming standard paths or brew)
# Adjust CMAKE_PREFIX_PATH to your Qt installation
QT_PATH=$(dirname $(dirname $(which qmake)))
cmake .. -DCMAKE_PREFIX_PATH=$QT_PATH -DCMAKE_BUILD_TYPE=Release

make -j$(sysctl -n hw.ncpu)

# Deploy (macdeployqt)
$QT_PATH/bin/macdeployqt $APP_NAME.app -dmg

echo "Build complete. DMG should be in $BUILD_DIR"
