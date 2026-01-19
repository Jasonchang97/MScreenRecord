# MScreenRecord

一款基于 Qt5 和 FFmpeg 的跨平台屏幕录制工具，支持多种录制模式、视频预览和快捷键操作。

![Version](https://img.shields.io/badge/version-1.4.2-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)
![Qt](https://img.shields.io/badge/Qt-5.x-green.svg)
![License](https://img.shields.io/badge/license-MIT-orange.svg)

## ✨ 功能特性

### 录制功能
- 🖥️ **智能窗口检测** - 自动识别鼠标下的窗口，一键选择录制区域
- 🌐 **全屏录制** - 支持多显示器全屏录制
- 🎯 **区域录制** - 自定义选择任意屏幕区域
- ⏱️ **倒计时启动** - 可配置的录制前倒计时（1-10秒）
- 🔊 **音频录制** - 支持系统声音和麦克风同时录制

### 视频预览
- ▶️ **内置播放器** - 基于 OpenGL 的高性能视频预览
- 📊 **进度条控制** - 双滑块选择播放范围，支持点击跳转
- ✂️ **视频剪辑** - 快速裁剪视频片段

### 界面与交互
- 🎨 **多主题支持** - 6种精美主题（暗夜黑、明亮白、赛博蓝、樱花粉、深邃紫、森林绿）
- ⌨️ **全局快捷键** - 自定义快捷键，支持冲突检测
- 📁 **历史记录** - 自动保存录制历史，方便管理
- 🔔 **系统托盘** - 最小化到托盘，后台运行

## 🚀 快速开始

### 系统要求
- Windows 10/11
- Visual Studio 2019 或更高版本
- Qt 5.12 或更高版本
- FFmpeg 4.x/5.x

### 编译构建

```bash
# 克隆仓库
git clone https://github.com/Jasonchang97/MScreenRecord.git
cd MScreenRecord

# 运行构建脚本
package_win.bat
```

### 目录结构

```
MScreenRecord/
├── include/          # 头文件
├── src/              # 源代码
├── resources/        # 资源文件（图标等）
├── CMakeLists.txt    # CMake 构建配置
├── package_win.bat   # Windows 打包脚本
├── package_mac.sh    # macOS 打包脚本
└── setup_script.iss  # Inno Setup 安装包脚本
```

## ⌨️ 默认快捷键

| 功能 | 快捷键 |
|------|--------|
| 显示/隐藏主界面 | `Ctrl+Alt+S` |
| 开始/停止录制 | `Ctrl+Alt+O` |

*快捷键可在设置中自定义*

## 🎨 主题预览

支持 6 种界面主题：
- **暗夜黑** - 默认深色主题，护眼舒适
- **明亮白** - 清爽的浅色主题
- **赛博蓝** - 科技感十足的蓝色主题
- **樱花粉** - 温馨可爱的粉色主题
- **深邃紫** - 优雅神秘的紫色主题
- **森林绿** - 自然清新的绿色主题

## 📦 依赖项

- **Qt5** - 跨平台 GUI 框架
  - Qt5::Widgets
  - Qt5::Multimedia
  - Qt5::MultimediaWidgets
  - Qt5::Network

- **FFmpeg** - 音视频处理
  - libavcodec
  - libavformat
  - libavutil
  - libswscale
  - libswresample
  - libavdevice

- **SDL2** - 音频播放

## 🔧 配置说明

应用配置存储在 Windows 注册表中：
```
HKEY_CURRENT_USER\Software\KSO\MScreenRecord
```

主要配置项：
- `savePath` - 视频保存路径
- `fps` - 录制帧率（10-60）
- `bitrateLevel` - 视频质量（0=高, 1=中, 2=低）
- `theme` - 界面主题
- `hotkeyShowWindow` - 显示主界面快捷键
- `hotkeyStartRecord` - 开始录制快捷键

## 📝 更新日志

### v1.4.2
- ✨ Toast 提示替代弹窗，3秒自动消失
- 🎨 Toast 提示多皮肤适配，队列管理避免重叠
- 🐛 修复视频剪切功能，正确处理时间戳
- 🐛 修复剪切后视频无法播放的问题
- 🔧 优化剪切按钮状态反馈

### v1.4.1
- ✨ 新增全局快捷键功能
- ✨ 快捷键冲突检测和抢注提示
- 🎨 快捷键编辑器多皮肤适配
- 🐛 修复视频预览 seek 问题
- 🐛 修复设置界面布局问题

### v1.4.0
- ✨ 智能窗口检测录制
- ✨ 集成式选择覆盖层
- 🎨 优化倒计时 UI
- 🐛 多项 bug 修复

### v1.3.0
- ✨ 多主题支持
- ✨ 视频预览和剪辑功能
- ✨ 系统托盘支持

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## 📧 联系方式

如有问题或建议，请通过 GitHub Issues 联系。
