#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QStringList>
#include <QRect>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>
#include <QAudioInput>
#include <QIODevice>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
}

// Simple Ring Buffer for Audio
struct AudioBuffer {
    uint8_t *data = nullptr;
    int size = 0;
    int writePos = 0;
    int readPos = 0;
    int capacity = 0;
    QMutex mutex;

    void init(int cap) {
        capacity = cap;
        data = (uint8_t*)av_malloc(capacity);
        size = 0; writePos = 0; readPos = 0;
    }
    void free() { if(data) av_free(data); data = nullptr; }
    
    void write(const uint8_t* src, int len) {
        QMutexLocker lock(&mutex);
        if (!data || len <= 0) return;
        // If overflow, drop oldest data to keep latest audio
        if (size + len > capacity) {
            int drop = (size + len) - capacity;
            readPos = (readPos + drop) % capacity;
            size -= drop;
            if (size < 0) size = 0;
        }
        for (int i=0; i<len; i++) {
            data[writePos] = src[i];
            writePos = (writePos + 1) % capacity;
        }
        size += len;
    }
    
    int read(uint8_t* dst, int len) {
        QMutexLocker lock(&mutex);
        if (!data || size <= 0 || len <= 0) return 0; // Check data validity
        if (len > size) len = size; // allow partial read
        for (int i=0; i<len; i++) {
            dst[i] = data[readPos];
            readPos = (readPos + 1) % capacity;
        }
        size -= len;
        return len;
    }
    
    int available() { QMutexLocker lock(&mutex); return size; }
    void clear() { QMutexLocker lock(&mutex); size = 0; writePos = 0; readPos = 0; }
};

// Adapter for QAudioInput
class AudioWrapper : public QIODevice {
    Q_OBJECT
public:
    AudioWrapper(AudioBuffer *buf, QObject *parent) : QIODevice(parent), m_buf(buf) {}
    qint64 readData(char *, qint64) override { return 0; }
    qint64 writeData(const char *data, qint64 len) override {
        if (m_buf) m_buf->write((const uint8_t*)data, (int)len);
        return len;
    }
private:
    AudioBuffer *m_buf;
};

class RecorderController : public QObject {
    Q_OBJECT

public:
    enum State {
        Stopped,
        Recording,
        Paused
    };

    explicit RecorderController(QObject *parent = nullptr);
    ~RecorderController();

    State state() const { return m_state; }
    
    void setRegion(const QRect &rect);
    void setAudioConfig(bool recordSys, double sysVol, bool recordMic, double micVol);
    void setFps(int fps); // Set recording frame rate
    bool checkSystemAudioAvailable(); // Pre-check and register if needed

    qint64 getDuration() const;

public slots:
    void startRecording();
    void stopRecording();

signals:
    void stateChanged(State newState);
    void recordingFinished(const QString &path);
    void errorOccurred(const QString &errorMsg);
    void logMessage(const QString &msg);
    void audioLevelsCalculated(double sysLevel, double micLevel); // 0.0 - 1.0 (RMS)
    void systemAudioMissing(); // New Signal

private:
    QString getFFmpegPath(); 
    bool probeAudioDevice(const QString& deviceName); 
    
    State m_state = Stopped;

    // Native FFmpeg Members
    void recordThreadFunc();
    void sysAudioThreadFunc();
    std::atomic<bool> m_isRecording;
    std::atomic<bool> m_isSysAudioRunning;
    QThread *m_recordThread = nullptr;
    QThread *m_sysAudioThread = nullptr;
    
    // FFmpeg Contexts
    AVFormatContext *m_outFmtCtx = nullptr;
    AVFormatContext *m_vInFmtCtx = nullptr;
    AVFormatContext *m_aSysInFmtCtx = nullptr;
    
    AVCodecContext *m_vEncCtx = nullptr;
    AVCodecContext *m_aEncCtx = nullptr;
    AVCodecContext *m_aSysDecCtx = nullptr;
    
    SwsContext *m_swsCtx = nullptr;
    SwrContext *m_swrMicCtx = nullptr;
    SwrContext *m_swrSysCtx = nullptr;
    
    // Audio Capture Members
    SDL_AudioDeviceID m_devSys = 0;
    SDL_AudioDeviceID m_devMic = 0;
    AudioBuffer m_bufSys;
    AudioBuffer m_bufMic;
    
    // Qt Audio Fallback
    QAudioInput *m_qtAudioSys = nullptr;
    QAudioInput *m_qtAudioMic = nullptr;
    AudioWrapper *m_qtWrapSys = nullptr;
    AudioWrapper *m_qtWrapMic = nullptr;

    // Config
    QRect m_recordRegion;
    bool m_recordMic;
    double m_micVolume;
    bool m_recordSys;
    double m_sysVolume;
    int m_fps; // Recording frame rate (from settings)
    
    QElapsedTimer m_timer;
    QString m_currentFile;
};
