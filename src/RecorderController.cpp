#include "RecorderController.h"
#include "LogManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QThread>
#include <QStandardPaths>
#include <QSettings>
#include <QDateTime>
#include <QFileInfo>
#include <QAudioDeviceInfo>
#include <QFile>
#include <QTextStream>
#include <QProcess>

#ifdef Q_OS_WIN
#include <objbase.h> // For CoInitialize
#include <windows.h>
#include <shellapi.h>

// Forward declare trace so it can be used inside helper
static void trace(const QString& msg);

// Use 32-bit regsvr32 on 64-bit Windows to register 32-bit DLLs (our app is x86)
static QString resolveRegsvr32Path() {
    QString sysWow = "C:/Windows/SysWOW64/regsvr32.exe";
    QString sys32 = "C:/Windows/System32/regsvr32.exe";
    if (QFile::exists(sysWow)) return sysWow;
    if (QFile::exists(sys32)) return sys32;
    return "regsvr32";
}

static bool runRegSvrAsAdmin(const QString &dllPath) {
    QString regsvr = resolveRegsvr32Path();
    std::wstring regsvrW = regsvr.toStdWString();

    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.lpVerb = L"runas";
    sei.lpFile = regsvrW.c_str();
    std::wstring param = L"/s \"" + dllPath.toStdWString() + L"\"";
    sei.lpParameters = param.c_str();
    sei.nShow = SW_HIDE;
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    
    if (ShellExecuteExW(&sei)) {
        WaitForSingleObject(sei.hProcess, INFINITE);
        DWORD exitCode = 0;
        GetExitCodeProcess(sei.hProcess, &exitCode);
        CloseHandle(sei.hProcess);
        trace(QString("regsvr32 (%1) exit code: %2").arg(regsvr).arg(exitCode));
        return (exitCode == 0);
    }
    trace(QString("ShellExecuteEx failed for regsvr32 path: %1").arg(regsvr));
    return false;
}
#endif

// TRACE LOGGING
static void trace(const QString& msg) {
    QString formatted = QString("%1 [%2] %3").arg(
        QDateTime::currentDateTime().toStringEx("HH:mm:ss.zzz"),
        QString::number((quintptr)QThread::currentThreadId()),
        msg
    );
    LogManager::instance().write(formatted, LogManager::Recorder);
}

static void audioRecordCallback(void *userdata, Uint8 *stream, int len) {
    AudioBuffer *buf = (AudioBuffer*)userdata;
    if (buf) buf->write(stream, len);
}

// Helper: Match SDL device name
static int findSdlDeviceIndex(bool isCapture, const QString &keyword) {
    int count = SDL_GetNumAudioDevices(isCapture ? 1 : 0);
    for (int i = 0; i < count; ++i) {
        QString name = QString::fromUtf8(SDL_GetAudioDeviceName(i, isCapture ? 1 : 0));
        if (name.contains(keyword, Qt::CaseInsensitive)) return i;
    }
    return -1;
}

// Helper: Match Qt device
static QAudioDeviceInfo findQtDevice(const QString &keyword) {
    auto devs = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    for (const auto& dev : devs) {
        if (dev.deviceName().contains(keyword, Qt::CaseInsensitive)) return dev;
    }
    return QAudioDeviceInfo();
}

RecorderController::RecorderController(QObject *parent)
    : QObject(parent), m_isRecording(false), m_sysVolume(1.0), m_micVolume(1.0), m_state(Stopped), m_fps(30)
{
#ifdef Q_OS_WIN
    CoInitialize(nullptr);
#endif
    // QFile::remove(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/rec_trace.txt"); // Handled by LogManager rotation
    trace("RecorderController Created");

    avdevice_register_all();
    AVInputFormat *fmt = av_find_input_format("wasapi");
    emit logMessage(QString("FFmpeg WASAPI Support: %1").arg(fmt ? "Yes" : "No"));
    avformat_network_init();
    
    // Try to init SDL
    bool sdlInited = false;
    if (SDL_Init(SDL_INIT_AUDIO) == 0) {
        int count = SDL_GetNumAudioDevices(1);
        if (count > 0) {
            trace(QString("SDL Init OK. Found %1 devices.").arg(count));
            sdlInited = true;
        } else {
            trace("SDL Init OK but 0 devices.");
        }
    } else {
        trace(QString("SDL Init Failed: %1").arg(SDL_GetError()));
    }
    
    if (!sdlInited) {
        trace("SDL failed/empty. Will attempt Qt Audio Fallback.");
    }
}

RecorderController::~RecorderController() {
    trace("RecorderController Destructor");
    stopRecording();
    SDL_Quit();
#ifdef Q_OS_WIN
    CoUninitialize();
#endif
}

QString RecorderController::getFFmpegPath() { return ""; }
bool RecorderController::probeAudioDevice(const QString& deviceName) { return true; }

void RecorderController::setRegion(const QRect &rect) { m_recordRegion = rect; }
void RecorderController::setAudioConfig(bool recordSys, double sysVol, bool recordMic, double micVol) {
    m_recordSys = recordSys;
    m_sysVolume = sysVol;
    m_recordMic = recordMic;
    // Cap volumes to avoid extreme clipping; mic up to 5.0 (500%), sys up to 2.0 (200%)
    m_sysVolume = qBound(0.0, m_sysVolume, 2.0);
    m_micVolume = qBound(0.0, micVol, 5.0);
}
// Check and register virtual audio device (Main Thread)
bool RecorderController::checkSystemAudioAvailable() {
#ifdef Q_OS_WIN
    AVFormatContext* ctx = nullptr;
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "list_devices", "false", 0);

    // 1. Try to open
    int ret = avformat_open_input(&ctx, "audio=virtual-audio-capturer", av_find_input_format("dshow"), &opts);
    if (ret == 0) {
        avformat_close_input(&ctx);
        trace("checkSystemAudioAvailable: Open success.");
        return true;
    }

    // 2. If failed, try register
    trace(QString("checkSystemAudioAvailable: Open failed (ret=%1). Attempting registration.").arg(ret));
    
    QString appDir = QCoreApplication::applicationDirPath();
    QString dllPath = appDir + "/3rd/audio_sniffer.dll";
    if (!QFile::exists(dllPath)) {
        dllPath = appDir + "/audio_sniffer.dll";
    }

    if (!QFile::exists(dllPath)) {
        trace("checkSystemAudioAvailable: audio_sniffer.dll not found.");
        emit logMessage("Error: audio_sniffer.dll not found. Cannot register system audio.");
        return false;
    }

    // 3. Register with Admin rights
    emit logMessage("System Audio not registered. Requesting Admin rights...");
    if (runRegSvrAsAdmin(dllPath)) {
        emit logMessage("Registration successful. Verifying...");
        
        // 4. Retry open
        ret = avformat_open_input(&ctx, "audio=virtual-audio-capturer", av_find_input_format("dshow"), &opts);
        if (ret == 0) {
            avformat_close_input(&ctx);
            trace("checkSystemAudioAvailable: Open success after registration.");
            emit logMessage("System Audio is ready.");
            return true;
        } else {
             trace(QString("checkSystemAudioAvailable: Still failed after registration (ret=%1).").arg(ret));
             emit logMessage("Error: System Audio device still not found after registration.");
             return false;
        }
    } else {
        trace("checkSystemAudioAvailable: Registration cancelled or failed.");
        emit logMessage("Warning: System Audio registration failed. System sound will not be recorded.");
        return false;
    }
#else
    return true; // Assume OK on Mac/Linux for now
#endif
}

void RecorderController::setFps(int fps) {
    m_fps = fps;
    if (m_fps < 10) m_fps = 10;
    if (m_fps > 60) m_fps = 60;
}
qint64 RecorderController::getDuration() const { return m_timer.elapsed(); }

void RecorderController::startRecording() {
    if (m_isRecording) return;
    trace("startRecording called");
    trace(QString("Audio Request -> Sys:%1 Mic:%2").arg(m_recordSys ? "on" : "off").arg(m_recordMic ? "on" : "off"));
    
    QString savePath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    QSettings settings("KSO", "MScreenRecord");
    savePath = settings.value("savePath", savePath).toString();
    QDir().mkpath(savePath);
    QString fileName = QString("Rec_%1.mp4").arg(QDateTime::currentDateTime().toStringEx("yyyyMMdd_HHmmss"));
    m_currentFile = QDir(savePath).filePath(fileName);

    // Initialize Audio Buffers (larger to avoid overflow on slow consumers)
    m_bufSys.init(1024 * 1024 * 8);
    m_bufMic.init(1024 * 1024 * 8);
    trace("Buffers Init (8MB per buffer)");

    // Initialize SDL Devices (Main Thread)
    m_devSys = 0;
    m_devMic = 0;
    
    // Ensure SDL is initialized
    if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
        if (SDL_Init(SDL_INIT_AUDIO) != 0) {
            trace(QString("SDL Init Failed in startRecording: %1").arg(SDL_GetError()));
            emit logMessage(QString("警告：SDL 音频初始化失败: %1").arg(SDL_GetError()));
        }
    }
    
    SDL_AudioSpec want;
    memset(&want, 0, sizeof(want));
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 1024;
    
    // --- WASAPI Loopback (System Audio) ---
    if (m_recordSys) {
        m_isSysAudioRunning = true;
        m_sysAudioThread = QThread::create([this](){ sysAudioThreadFunc(); });
        m_sysAudioThread->start();
        emit logMessage("启动系统声音录制 (虚拟声卡模式)...");
        trace("System Audio Thread Started");
    }
    
    int deviceCount = SDL_GetNumAudioDevices(1); // 1 = capture devices
    trace(QString("SDL Audio Capture Devices: %1").arg(deviceCount));
    for (int i = 0; i < deviceCount; ++i) {
        trace(QString("  SDL CapDev[%1]: %2").arg(i).arg(QString::fromUtf8(SDL_GetAudioDeviceName(i, 1))));
    }
    
    if (m_recordMic) {
        if (deviceCount > 0) {
            int idx = findSdlDeviceIndex(true, "Microphone");
            if (idx < 0) idx = findSdlDeviceIndex(true, "麦克风");
            if (idx < 0) idx = findSdlDeviceIndex(true, "Audio");
            if (idx < 0) idx = findSdlDeviceIndex(true, "耳机");
            
            want.callback = audioRecordCallback;
            want.userdata = &m_bufMic;
            const char* name = (idx >= 0) ? SDL_GetAudioDeviceName(idx, 1) : nullptr;
            m_devMic = SDL_OpenAudioDevice(name, 1, &want, nullptr, 0);
            if (m_devMic > 0) {
                SDL_PauseAudioDevice(m_devMic, 0);
                emit logMessage("已连接麦克风 (SDL): " + (name ? QString::fromUtf8(name) : QString("Default")));
                trace(QString("SDL Mic Opened: %1").arg(name ? QString::fromUtf8(name) : "Default"));
            } else {
                trace(QString("SDL Mic Open Failed: %1").arg(SDL_GetError()));
                emit logMessage(QString("警告：SDL 无法打开麦克风: %1").arg(SDL_GetError()));
            }
        } else {
            trace("No SDL capture devices found");
            emit logMessage("警告：未检测到 SDL 音频捕获设备");
        }
    }

    // --- Qt Audio Fallback ---
    QAudioFormat qtFmt;
    qtFmt.setSampleRate(44100);
    qtFmt.setChannelCount(2);
    qtFmt.setSampleSize(16);
    qtFmt.setCodec("audio/pcm");
    qtFmt.setByteOrder(QAudioFormat::LittleEndian);
    qtFmt.setSampleType(QAudioFormat::SignedInt);

    // Qt fallback removed for Sys Audio (Using WASAPI Loopback)
    auto devs = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    trace(QString("Qt Audio Input Devices: %1").arg(devs.size()));
    for (const auto& d : devs) {
        trace(QString("  - %1").arg(d.deviceName()));
    }

    if (m_recordMic && m_devMic == 0) {
        
        QAudioDeviceInfo dev = findQtDevice("Microphone");
        if (dev.isNull()) dev = findQtDevice("麦克风");
        if (dev.isNull()) dev = findQtDevice("Audio"); // Headset
        if (dev.isNull()) dev = findQtDevice("耳机");
        if (dev.isNull()) dev = QAudioDeviceInfo::defaultInputDevice(); // Last resort
        
        if (!dev.isNull()) {
            if (dev.isFormatSupported(qtFmt)) {
                m_qtAudioMic = new QAudioInput(dev, qtFmt, this);
                m_qtWrapMic = new AudioWrapper(&m_bufMic, this);
                m_qtWrapMic->open(QIODevice::WriteOnly);
                m_qtAudioMic->start(m_qtWrapMic);
                emit logMessage("已连接麦克风 (Qt): " + dev.deviceName());
                trace("Qt Mic Started: " + dev.deviceName());
            } else {
                trace(QString("Qt Format Not Supported for: %1").arg(dev.deviceName()));
                emit logMessage(QString("警告：设备 %1 不支持所需音频格式").arg(dev.deviceName()));
            }
        } else {
            emit logMessage("警告：未检测到麦克风设备。");
            trace("Qt Mic Not Found");
        }
    }

    m_isRecording = true;
    m_state = Recording;
    emit logMessage("开始录制 (Native API)...");
    emit stateChanged(Recording);
    m_timer.start();

    m_recordThread = QThread::create([this](){ recordThreadFunc(); });
    m_recordThread->start();
    trace("Thread Started");
}

void RecorderController::stopRecording() {
    if (!m_isRecording) return;
    trace("stopRecording called");
    
    emit logMessage("正在停止录制...");
    m_isRecording = false;
    
    if (m_recordThread) {
        trace("Waiting for thread...");
        m_recordThread->wait();
        delete m_recordThread; 
        m_recordThread = nullptr;
        trace("Thread joined");
    }
    if (m_sysAudioThread) {
        m_isSysAudioRunning = false;
        m_sysAudioThread->wait();
        delete m_sysAudioThread; m_sysAudioThread = nullptr;
        trace("SysAudio Thread Joined");
    }
    
    // Close SDL Devices
    if (m_devSys > 0) {
        SDL_PauseAudioDevice(m_devSys, 1);
        SDL_CloseAudioDevice(m_devSys);
        m_devSys = 0;
    }
    if (m_devMic > 0) {
        SDL_PauseAudioDevice(m_devMic, 1);
        SDL_CloseAudioDevice(m_devMic);
        m_devMic = 0;
    }

    // Stop Qt Audio
    if (m_qtAudioSys) { m_qtAudioSys->stop(); delete m_qtAudioSys; m_qtAudioSys = nullptr; }
    if (m_qtWrapSys) { delete m_qtWrapSys; m_qtWrapSys = nullptr; }
    if (m_qtAudioMic) { m_qtAudioMic->stop(); delete m_qtAudioMic; m_qtAudioMic = nullptr; }
    if (m_qtWrapMic) { delete m_qtWrapMic; m_qtWrapMic = nullptr; }

    // Free Buffers
    trace("Freeing Buffers...");
    m_bufSys.free();
    m_bufMic.free();
    trace("Buffers Freed");
    
    m_state = Stopped;
    emit stateChanged(Stopped);
    emit recordingFinished(m_currentFile);
    trace("stopRecording finished");
}

void RecorderController::recordThreadFunc() {
    trace("Worker Thread Start");
    // Reset Contexts
    m_outFmtCtx = nullptr;
    m_vInFmtCtx = nullptr;
    m_vEncCtx = nullptr;
    m_aEncCtx = nullptr;
    m_swsCtx = nullptr;
    m_swrMicCtx = nullptr;

    bool headerWritten = false;

    // 1. Open Output
    avformat_alloc_output_context2(&m_outFmtCtx, nullptr, "mp4", m_currentFile.toUtf8().constData());
    if (!m_outFmtCtx) { emit errorOccurred("无法创建输出文件"); trace("Err: alloc output"); return; }

    // 2. Open Video Input
    AVDictionary *opts = nullptr;
    // Use user-configured frame rate from settings
    QString fpsStr = QString::number(m_fps);
    av_dict_set(&opts, "framerate", fpsStr.toUtf8().constData(), 0);
    av_dict_set(&opts, "probesize", "50M", 0);
    trace(QString("Requesting FPS from gdigrab: %1").arg(m_fps));
    av_dict_set(&opts, "draw_mouse", "1", 0); // Enable cursor capture

    const char* inputFormat = "gdigrab";
    const char* inputDevice = "desktop";
    
    #ifdef Q_OS_MAC
    inputFormat = "avfoundation";
    inputDevice = "1:none";
    av_dict_set(&opts, "capture_cursor", "1", 0);
    av_dict_set(&opts, "capture_mouse_clicks", "1", 0);
    #endif

    if (!m_recordRegion.isNull()) {
        #ifdef Q_OS_WIN
        // 确保宽高是偶数（H.264 编码器要求）
        int w = m_recordRegion.width();
        int h = m_recordRegion.height();
        if (w % 2 != 0) w -= 1;
        if (h % 2 != 0) h -= 1;
        // 确保最小尺寸
        if (w < 64) w = 64;
        if (h < 64) h = 64;
        
        av_dict_set(&opts, "video_size", QString("%1x%2").arg(w).arg(h).toUtf8().constData(), 0);
        av_dict_set(&opts, "offset_x", QString::number(m_recordRegion.x()).toUtf8().constData(), 0);
        av_dict_set(&opts, "offset_y", QString::number(m_recordRegion.y()).toUtf8().constData(), 0);
        trace(QString("Recording region: %1x%2 at (%3,%4)").arg(w).arg(h).arg(m_recordRegion.x()).arg(m_recordRegion.y()));
        #endif
    }
    
    if (avformat_open_input(&m_vInFmtCtx, inputDevice, av_find_input_format(inputFormat), &opts) < 0) {
        emit errorOccurred("无法打开屏幕捕获设备"); trace("Err: open gdigrab"); return;
    }
    avformat_find_stream_info(m_vInFmtCtx, nullptr);
    int vInStreamIdx = -1;
    for(int i=0; i < static_cast<int>(m_vInFmtCtx->nb_streams); i++) {
        if(m_vInFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) vInStreamIdx = i;
    }
    if (vInStreamIdx < 0) {
        emit errorOccurred("未找到视频流"); trace("Err: no video stream"); return;
    }

    // 2.5 Video Decoder
    const AVCodec *vDec = avcodec_find_decoder(m_vInFmtCtx->streams[vInStreamIdx]->codecpar->codec_id);
    AVCodecContext *vDecCtx = avcodec_alloc_context3(vDec);
    avcodec_parameters_to_context(vDecCtx, m_vInFmtCtx->streams[vInStreamIdx]->codecpar);
    avcodec_open2(vDecCtx, vDec, nullptr);

    // Get input stream frame rate (use r_frame_rate or avg_frame_rate)
    AVRational inputFps = m_vInFmtCtx->streams[vInStreamIdx]->r_frame_rate;
    if (inputFps.num == 0 || inputFps.den == 0) {
        inputFps = m_vInFmtCtx->streams[vInStreamIdx]->avg_frame_rate;
    }
    // Use user-configured FPS if input stream doesn't provide valid FPS
    if (inputFps.num == 0 || inputFps.den == 0) {
        inputFps = {m_fps, 1}; // Use user-configured FPS
        trace(QString("Input stream FPS not available, using configured FPS: %1").arg(m_fps));
    } else {
        trace(QString("Input Stream FPS: %1/%2 = %3").arg(inputFps.num).arg(inputFps.den).arg(av_q2d(inputFps)));
        // If input FPS differs significantly from configured FPS, use configured FPS
        double inputFpsValue = av_q2d(inputFps);
        if (qAbs(inputFpsValue - m_fps) > 2.0) {
            trace(QString("Input FPS (%1) differs from configured FPS (%2), using configured FPS").arg(inputFpsValue).arg(m_fps));
            inputFps = {m_fps, 1};
        }
    }

    // 3. Video Encoder
    AVStream *vOutStream = avformat_new_stream(m_outFmtCtx, nullptr);
    const AVCodec *vEnc = avcodec_find_encoder(AV_CODEC_ID_H264);
    m_vEncCtx = avcodec_alloc_context3(vEnc);
    m_vEncCtx->width = m_vInFmtCtx->streams[vInStreamIdx]->codecpar->width;
    m_vEncCtx->height = m_vInFmtCtx->streams[vInStreamIdx]->codecpar->height;
    
    // Use input FPS for encoder time_base to ensure correct timing
    m_vEncCtx->time_base = {inputFps.den, inputFps.num}; // time_base = 1/fps
    m_vEncCtx->framerate = inputFps; // Set framerate for encoder
    m_vEncCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    m_vEncCtx->bit_rate = 3000000;
    // GOP size: keyframe every 1 second (round to ensure integer)
    int gopSize = (int)(inputFps.num / (double)inputFps.den + 0.5);
    if (gopSize < 1) gopSize = 30; // Minimum 1 second
    m_vEncCtx->gop_size = gopSize;
    m_vEncCtx->thread_count = 1; // Single thread to avoid crash
    if (m_outFmtCtx->oformat->flags & AVFMT_GLOBALHEADER) m_vEncCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    avcodec_open2(m_vEncCtx, vEnc, nullptr);
    avcodec_parameters_from_context(vOutStream->codecpar, m_vEncCtx);
    // CRITICAL: Set output stream time_base to match encoder time_base
    vOutStream->time_base = m_vEncCtx->time_base;
    vOutStream->avg_frame_rate = inputFps;
    vOutStream->r_frame_rate = inputFps;
    trace(QString("Output Stream time_base: %1/%2, fps: %3/%4").arg(vOutStream->time_base.num).arg(vOutStream->time_base.den).arg(inputFps.num).arg(inputFps.den));

    // 4. Audio Setup
    // Check if ANY device was opened (SysThread, SDL or Qt)
    bool hasAudio = (m_isSysAudioRunning.load() || m_devMic > 0 || m_qtAudioMic);
    
    AVStream *aOutStream = nullptr;
    if (hasAudio) {
        aOutStream = avformat_new_stream(m_outFmtCtx, nullptr);
        const AVCodec *aEnc = avcodec_find_encoder(AV_CODEC_ID_AAC);
        m_aEncCtx = avcodec_alloc_context3(aEnc);
        m_aEncCtx->sample_rate = 44100;
        m_aEncCtx->channel_layout = AV_CH_LAYOUT_STEREO;
        m_aEncCtx->channels = 2;
        m_aEncCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
        m_aEncCtx->time_base = {1, 44100};
        m_aEncCtx->thread_count = 1; // Single thread to avoid crash
        if (m_outFmtCtx->oformat->flags & AVFMT_GLOBALHEADER) m_aEncCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        avcodec_open2(m_aEncCtx, aEnc, nullptr);
        avcodec_parameters_from_context(aOutStream->codecpar, m_aEncCtx);
        // CRITICAL: Set output stream time_base to match encoder time_base
        aOutStream->time_base = m_aEncCtx->time_base;
        
        m_swrMicCtx = swr_alloc_set_opts(nullptr, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_FLTP, 44100,
                                         AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100, 0, nullptr);
        swr_init(m_swrMicCtx);
    }
    trace("Encoders Setup Done");

    // 5. Write Header
    if (!(m_outFmtCtx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&m_outFmtCtx->pb, m_currentFile.toUtf8().constData(), AVIO_FLAG_WRITE) < 0) {
            emit errorOccurred("无法打开输出文件"); trace("Err: avio_open"); return;
        }
    }
    if (avformat_write_header(m_outFmtCtx, nullptr) >= 0) {
        headerWritten = true;
        trace("Header Written");
    } else {
        trace("Err: write_header failed");
    }

    // 6. Loop
    AVPacket pkt; av_init_packet(&pkt);
    AVFrame *rawFrame = av_frame_alloc();
    AVFrame *yuvFrame = av_frame_alloc();
    yuvFrame->format = AV_PIX_FMT_YUV420P;
    yuvFrame->width = m_vEncCtx->width;
    yuvFrame->height = m_vEncCtx->height;
    av_frame_get_buffer(yuvFrame, 32);
    
    AVFrame *aFrame = av_frame_alloc();
    aFrame->nb_samples = 1024;
    aFrame->format = m_aEncCtx ? m_aEncCtx->sample_fmt : AV_SAMPLE_FMT_FLTP;
    aFrame->channel_layout = AV_CH_LAYOUT_STEREO;
    if (hasAudio) av_frame_get_buffer(aFrame, 0);
    
    int64_t vPts = 0;
    int64_t aPts = 0;
    int lastW = 0, lastH = 0, lastFmt = -1;
    double actualFps = av_q2d(inputFps); // Actual FPS from input stream
    trace(QString("Recording with FPS: %1").arg(actualFps));
    
    // Use actual recording time for PTS calculation
    QElapsedTimer frameTimer;
    frameTimer.start();
    int64_t startTimeUs = -1; // Start time in microseconds (-1 means not initialized)

    uint8_t rawSys[4096];
    uint8_t rawMic[4096];
    int16_t mixBuf[4096];

    trace("Enter Loop");
    
    QElapsedTimer levelTimer;
    levelTimer.start();

    while (m_isRecording) {
        // Video
        if (av_read_frame(m_vInFmtCtx, &pkt) >= 0) {
            if (pkt.stream_index == vInStreamIdx) {
                if (avcodec_send_packet(vDecCtx, &pkt) == 0) {
                    while (avcodec_receive_frame(vDecCtx, rawFrame) == 0) {
                        if (!m_swsCtx || rawFrame->width != lastW || rawFrame->height != lastH || rawFrame->format != lastFmt) {
                            if (m_swsCtx) sws_freeContext(m_swsCtx);
                            m_swsCtx = sws_getContext(rawFrame->width, rawFrame->height, (AVPixelFormat)rawFrame->format,
                                                      m_vEncCtx->width, m_vEncCtx->height, AV_PIX_FMT_YUV420P,
                                                      SWS_BICUBIC, nullptr, nullptr, nullptr);
                            lastW = rawFrame->width; lastH = rawFrame->height; lastFmt = rawFrame->format;
                        }
                        if (m_swsCtx) {
                            uint8_t *dst[4] = { yuvFrame->data[0], yuvFrame->data[1], yuvFrame->data[2], nullptr };
                            int lines[4] = { yuvFrame->linesize[0], yuvFrame->linesize[1], yuvFrame->linesize[2], 0 };
                            sws_scale(m_swsCtx, rawFrame->data, rawFrame->linesize, 0, rawFrame->height, dst, lines);
                            
                            // Calculate PTS based on actual elapsed time
                            if (startTimeUs < 0) {
                                startTimeUs = frameTimer.elapsed() * 1000LL; // Convert to microseconds
                                yuvFrame->pts = 0; // First frame starts at PTS 0
                            } else {
                                int64_t elapsedUs = frameTimer.elapsed() * 1000LL - startTimeUs;
                                // Convert microseconds to PTS: PTS = elapsed_us * fps / 1000000
                                // Since time_base = {den, num} = {1, fps}, PTS = elapsed_us * fps / 1000000
                                // inputFps = {num, den}, so fps = num/den
                                // PTS = elapsedUs * num / (1000000 * den)
                                yuvFrame->pts = (elapsedUs * inputFps.num) / (1000000LL * inputFps.den);
                            }
                            
                            avcodec_send_frame(m_vEncCtx, yuvFrame);
                            AVPacket encPkt; av_init_packet(&encPkt);
                            while (avcodec_receive_packet(m_vEncCtx, &encPkt) == 0) {
                                encPkt.stream_index = vOutStream->index;
                                av_packet_rescale_ts(&encPkt, m_vEncCtx->time_base, vOutStream->time_base);
                                if (headerWritten) av_interleaved_write_frame(m_outFmtCtx, &encPkt);
                                av_packet_unref(&encPkt);
                            }
                        }
                    }
                }
            }
            av_packet_unref(&pkt);
        }
        
        // Audio Mixing (decouple from video FPS; fill based on elapsed wall clock)
        if (hasAudio && startTimeUs >= 0) {
            int64_t elapsedUs = frameTimer.elapsed() * 1000LL - startTimeUs;
            int64_t targetSamples = (elapsedUs * 44100) / 1000000LL;
            // Produce audio until catching up to target (allow small lead of 2048 samples)
            while (aPts + 1024 <= targetSamples + 2048) {
                int sysAvail = m_bufSys.available();
                int micAvail = m_bufMic.available();
                
                bool sysActive = m_isSysAudioRunning.load();
                bool micActive = (m_devMic > 0 || m_qtAudioMic);
                
                memset(rawSys, 0, 4096);
                memset(rawMic, 0, 4096);
                
                if (sysActive && sysAvail > 0) {
                    int toRead = qMin(sysAvail, 4096);
                    m_bufSys.read(rawSys, toRead);
                }
                if (micActive && micAvail > 0) {
                    int toRead = qMin(micAvail, 4096);
                    m_bufMic.read(rawMic, toRead);
                }
                
                int16_t* s = (int16_t*)rawSys;
                int16_t* m = (int16_t*)rawMic;
                
                // Boost mic gain significantly as raw PCM from some mics is very low
                double micBoost = 10.0; // Further increased boost for microphone (from 5.0 to 10.0)

                // Calculate Levels (RMS) periodically
                if (levelTimer.elapsed() > 100) {
                    double sumSys = 0;
                    double sumMic = 0;
                    // Note: rawSys/rawMic are 4096 bytes = 2048 int16 samples.
                    // If stereo, it's 1024 frames of 2 channels.
                    // The loop below iterates 2048 times, which covers all samples.
                    // This calculates RMS over all samples (L+R mixed). Good enough.
                    
                    for (int i = 0; i < 2048; i++) {
                        if (sysActive && sysAvail > 0) sumSys += (double)s[i] * s[i];
                        if (micActive && micAvail > 0) {
                            // Use raw mic input for metering, but respect boost?
                            // Let's use the boosted value to match what's recorded.
                            // But clamp it to avoid overflow in sumMic if needed (double is huge, it's fine).
                            double val = (double)m[i] * micBoost; 
                            sumMic += val * val;
                        }
                    }
                    
                    double rmsSys = sqrt(sumSys / 2048.0) / 32768.0;
                    double rmsMic = sqrt(sumMic / 2048.0) / 32768.0;
                    
                    if (rmsSys > 1.0) rmsSys = 1.0;
                    if (rmsMic > 1.0) rmsMic = 1.0;
                    
                    emit audioLevelsCalculated(rmsSys, rmsMic);
                    levelTimer.restart();
                }

                for (int i = 0; i < 2048; i++) {
                    // Apply per-source volume with simple soft clip
                    int32_t val = (int32_t)(s[i] * m_sysVolume) + (int32_t)(m[i] * m_micVolume * micBoost);
                    if (val > 32767) val = 32767;
                    if (val < -32768) val = -32768;
                    mixBuf[i] = (int16_t)val;
                }
                
                const uint8_t *inData[1] = { (uint8_t*)mixBuf };
                swr_convert(m_swrMicCtx, aFrame->data, 1024, inData, 1024);
                
                aFrame->pts = aPts;
                aPts += 1024;
                
                avcodec_send_frame(m_aEncCtx, aFrame);
                AVPacket aPkt; av_init_packet(&aPkt);
                while (avcodec_receive_packet(m_aEncCtx, &aPkt) == 0) {
                    aPkt.stream_index = aOutStream->index;
                    av_packet_rescale_ts(&aPkt, m_aEncCtx->time_base, aOutStream->time_base);
                    if (headerWritten) av_interleaved_write_frame(m_outFmtCtx, &aPkt);
                    av_packet_unref(&aPkt);
                }
            }
        }
        
        QThread::msleep(1); 
    }

    trace("Exit Loop");

    // Flush Video Encoder (Safe with thread_count=1)
    if (m_vEncCtx) {
        trace("Flushing Video Encoder");
        avcodec_send_frame(m_vEncCtx, nullptr);
        AVPacket encPkt; av_init_packet(&encPkt);
        while (avcodec_receive_packet(m_vEncCtx, &encPkt) == 0) {
            encPkt.stream_index = vOutStream->index;
            av_packet_rescale_ts(&encPkt, m_vEncCtx->time_base, vOutStream->time_base);
            if (headerWritten) av_interleaved_write_frame(m_outFmtCtx, &encPkt);
            av_packet_unref(&encPkt);
        }
    }

    // Flush Audio Encoder
    if (hasAudio && m_aEncCtx) {
        trace("Flushing Audio Encoder");
        avcodec_send_frame(m_aEncCtx, nullptr);
        AVPacket aPkt; av_init_packet(&aPkt);
        while (avcodec_receive_packet(m_aEncCtx, &aPkt) == 0) {
            aPkt.stream_index = aOutStream->index;
            av_packet_rescale_ts(&aPkt, m_aEncCtx->time_base, aOutStream->time_base);
            if (headerWritten) av_interleaved_write_frame(m_outFmtCtx, &aPkt);
            av_packet_unref(&aPkt);
        }
    }

    if (m_outFmtCtx && headerWritten) {
        trace("Write Trailer");
        av_write_trailer(m_outFmtCtx);
    }
    
    trace("Free Video Enc");
    if (m_vEncCtx) {
        avcodec_free_context(&m_vEncCtx);
        m_vEncCtx = nullptr;
    }
    trace("Free Audio Enc");
    if (m_aEncCtx) {
        avcodec_free_context(&m_aEncCtx);
        m_aEncCtx = nullptr;
    }

    if (m_outFmtCtx) {
        if (!(m_outFmtCtx->oformat->flags & AVFMT_NOFILE)) {
            trace("Close PB");
            avio_closep(&m_outFmtCtx->pb);
        }
        trace("Free OutCtx");
        avformat_free_context(m_outFmtCtx);
        m_outFmtCtx = nullptr;
    }
    
    // Note: SDL/Qt Closed in stopRecording()
    
    trace("Free Video Dec");
    if (vDecCtx) avcodec_free_context(&vDecCtx);
    
    trace("Close Input");
    if (m_vInFmtCtx) {
        avformat_close_input(&m_vInFmtCtx);
        m_vInFmtCtx = nullptr;
    }
    
    trace("Free Sws");
    if (m_swsCtx) {
        sws_freeContext(m_swsCtx);
        m_swsCtx = nullptr;
    }
    trace("Free Swr");
    if (m_swrMicCtx) {
        swr_free(&m_swrMicCtx);
        m_swrMicCtx = nullptr;
    }
    
    trace("Free Frames");
    av_frame_free(&rawFrame);
    av_frame_free(&yuvFrame);
    av_frame_free(&aFrame);
    
    trace("Worker Cleanup Done");
}

void RecorderController::sysAudioThreadFunc() {
    trace("SysAudio Thread Start");
    m_aSysInFmtCtx = nullptr;
    m_swrSysCtx = nullptr;
    m_aSysDecCtx = nullptr;
    
    // 1. Open Input
    bool opened = false;
    
#ifdef Q_OS_WIN
    AVDictionary *opts = nullptr;
    av_dict_set(&opts, "loopback", "1", 0);

    // Log DLL existence and regsvr32 path
    QString appDir = QCoreApplication::applicationDirPath();
    QString dllPath = appDir + "/3rd/audio_sniffer.dll";
    if (!QFile::exists(dllPath)) {
        dllPath = appDir + "/audio_sniffer.dll";
    }

    int retOpen = avformat_open_input(&m_aSysInFmtCtx, "audio=virtual-audio-capturer", av_find_input_format("dshow"), &opts);
    trace(QString("SysAudio: avformat_open_input ret = %1").arg(retOpen));
    if (retOpen == 0) {
        opened = true;
        emit logMessage("Using Virtual Audio Capturer");
    } else {
         emit logMessage("Virtual Audio Capturer failed to open (Code: " + QString::number(retOpen) + ")");
         // Note: Registration logic moved to checkSystemAudioAvailable().
         // If we are here, it means checkSystemAudioAvailable wasn't called or failed, or something else is wrong.
         // We won't try to register inside the thread anymore to avoid blocking/UI issues.
         trace("SysAudio: Failed to open. Assuming checkSystemAudioAvailable handled registration or user cancelled.");
    }
#elif defined(Q_OS_MAC)
    AVInputFormat *fmt = av_find_input_format("avfoundation");
    if (avformat_open_input(&m_aSysInFmtCtx, ":BlackHole 16ch", fmt, nullptr) == 0) { 
        opened = true; 
        emit logMessage("Connected to BlackHole 16ch");
    } else if (avformat_open_input(&m_aSysInFmtCtx, ":Soundflower (2ch)", fmt, nullptr) == 0) {
        opened = true;
        emit logMessage("Connected to Soundflower (2ch)");
    } else {
        emit logMessage("Mac System Audio: Requires 'BlackHole' or 'Soundflower' driver.");
    }
#endif

    if (!opened) {
        emit errorOccurred("无法启动系统声音录制 (Virtual Audio Capturer 失败)");
        trace("Err: Virtual Audio Capturer Open Failed");
        m_isSysAudioRunning = false;
        return;
    }
    avformat_find_stream_info(m_aSysInFmtCtx, nullptr);
    
    int streamIdx = -1;
    for(int i=0; i < static_cast<int>(m_aSysInFmtCtx->nb_streams); i++) {
        if(m_aSysInFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            streamIdx = i; break;
        }
    }
    if (streamIdx < 0) {
        avformat_close_input(&m_aSysInFmtCtx);
        m_isSysAudioRunning = false;
        return;
    }
    
    // 2. Decoder
    AVCodec *dec = avcodec_find_decoder(m_aSysInFmtCtx->streams[streamIdx]->codecpar->codec_id);
    m_aSysDecCtx = avcodec_alloc_context3(dec);
    avcodec_parameters_to_context(m_aSysDecCtx, m_aSysInFmtCtx->streams[streamIdx]->codecpar);
    if (avcodec_open2(m_aSysDecCtx, dec, nullptr) < 0) {
        avformat_close_input(&m_aSysInFmtCtx);
        m_isSysAudioRunning = false;
        return;
    }
    
    // 3. Resampler
    m_swrSysCtx = swr_alloc_set_opts(nullptr,
        AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
        m_aSysDecCtx->channel_layout ? m_aSysDecCtx->channel_layout : av_get_default_channel_layout(m_aSysDecCtx->channels),
        m_aSysDecCtx->sample_fmt, m_aSysDecCtx->sample_rate,
        0, nullptr);
    swr_init(m_swrSysCtx);
    
    AVPacket pkt;
    av_init_packet(&pkt);
    AVFrame *frame = av_frame_alloc();
    
    while (m_isSysAudioRunning) {
        av_init_packet(&pkt); pkt.data = nullptr; pkt.size = 0;
        if (av_read_frame(m_aSysInFmtCtx, &pkt) >= 0) {
            if (pkt.stream_index == streamIdx) {
                if (avcodec_send_packet(m_aSysDecCtx, &pkt) == 0) {
                    while (avcodec_receive_frame(m_aSysDecCtx, frame) == 0) {
                         int out_samples = av_rescale_rnd(swr_get_delay(m_swrSysCtx, m_aSysDecCtx->sample_rate) + frame->nb_samples, 44100, m_aSysDecCtx->sample_rate, AV_ROUND_UP);
                         int out_size = out_samples * 2 * 2; 
                         
                         uint8_t *buf = (uint8_t*)av_malloc(out_size);
                         uint8_t *out[1] = { buf };
                         
                         int len = swr_convert(m_swrSysCtx, out, out_samples, (const uint8_t**)frame->data, frame->nb_samples);
                         if (len > 0) {
                             m_bufSys.write(buf, len * 2 * 2);
                             // trace(QString("SysAudio: Wrote %1 bytes").arg(len*4)); 
                         }
                         av_free(buf);
                    }
                }
            }
            av_packet_unref(&pkt);
        } else {
             // trace("SysAudio: av_read_frame failed or EOF");
             QThread::msleep(10);
        }
    }
    
    av_frame_free(&frame);
    if (m_aSysDecCtx) { avcodec_free_context(&m_aSysDecCtx); m_aSysDecCtx = nullptr; }
    if (m_swrSysCtx) { swr_free(&m_swrSysCtx); m_swrSysCtx = nullptr; }
    if (m_aSysInFmtCtx) { avformat_close_input(&m_aSysInFmtCtx); m_aSysInFmtCtx = nullptr; }
    trace("SysAudio Thread Exit");
}