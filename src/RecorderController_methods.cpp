
bool RecorderController::isSystemAudioAvailable() {
#ifdef Q_OS_WIN
    AVFormatContext *fmtCtx = nullptr;
    // Try to open silently to check existence
    // Use list_devices=true doesn't really help programmatically easily without parsing logs
    // Just try open
    int ret = avformat_open_input(&fmtCtx, "audio=virtual-audio-capturer", av_find_input_format("dshow"), nullptr);
    if (ret == 0) {
        avformat_close_input(&fmtCtx);
        return true;
    }
    return false;
#else
    return true; // Mac/Linux assume ok or handled differently
#endif
}

bool RecorderController::checkSystemAudioReady() {
    if (isSystemAudioAvailable()) return true;

#ifdef Q_OS_WIN
    // Not available, try to register
    QString appDir = QCoreApplication::applicationDirPath();
    QString dllPath = appDir + "/3rd/audio_sniffer.dll";
    if (!QFile::exists(dllPath)) {
        dllPath = appDir + "/audio_sniffer.dll";
    }
    
    if (QFile::exists(dllPath)) {
        emit logMessage("System Audio not registered. Requesting Admin rights...");
        trace("SysAudio: Triggering UAC registration for " + dllPath);
        if (runRegSvrAsAdmin(dllPath)) {
            emit logMessage("Registration successful.");
            // Verify again
            return isSystemAudioAvailable();
        } else {
            emit logMessage("Registration failed or cancelled.");
            return false;
        }
    } else {
        emit logMessage("audio_sniffer.dll not found.");
        return false;
    }
#else
    return false;
#endif
}
