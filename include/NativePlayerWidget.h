#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QImage>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore> // Added
#include <deque>
#include <atomic>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#define SDL_MAIN_HANDLED // Prevent SDL from hijacking main
#include <SDL.h>
}

struct PacketQueue {
    std::deque<AVPacket> queue;
    int size = 0;
    QMutex mutex;
    QWaitCondition cond;
    bool abort_request = false;
    void put(AVPacket *pkt);
    int get(AVPacket *pkt, bool block);
    void clear();
    int count();
    void abort();
    void start();
};

class NativePlayerWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit NativePlayerWidget(QWidget *parent = nullptr);
    ~NativePlayerWidget();

    void setRange(qint64 startMs, qint64 endMs);
    void startPlay(const QString &filePath);
    void seek(const QString &filePath, qint64 ms); // Seek and show single frame
    void stopPlay();
    bool isPlaying() const { return m_isRunning; }

signals:
    void playbackFinished();
    void positionChanged(qint64 ms);
    void frameReady(const QByteArray &data, int width, int height);
    void previewImageReady(const QImage &img);
    void logMessage(const QString &msg);
    void errorOccurred(const QString &err);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private slots:
    void onFrameReady(const QByteArray &data, int width, int height);

private:
    void readThreadFunc();
    void videoThreadFunc();
    static void sdlAudioCallback(void *opaque, Uint8 *stream, int len); // New
    void freeResources();
    void freePreviewResources();
    double getAudioClock();

    QString m_filePath;
    int64_t m_startMs = 0;
    int64_t m_endMs = 0;
    
    // Preview Context
    QString m_previewPath;
    QString m_currentOpenPath; // Added
    AVFormatContext *m_previewFmtCtx = nullptr;
    AVCodecContext *m_previewCodecCtx = nullptr;
    SwsContext *m_previewSwsCtx = nullptr;
    int m_previewStreamIdx = -1;
    
    // Preview Threading
    QThread *m_previewThread = nullptr;
    QSemaphore m_semPreview;
    QMutex m_previewMutex;
    std::atomic<int64_t> m_reqPreviewMs {-1};
    std::atomic<bool> m_isPreviewRunning {false};
    std::atomic<bool> m_isPreviewActive {false}; // Flag to indicate preview is active
    void previewThreadFunc();

    std::atomic<bool> m_isRunning;
    
    QThread *m_readThread = nullptr;
    QThread *m_videoThread = nullptr;

    AVFormatContext *m_fmtCtx = nullptr;
    
    // Video
    AVCodecContext *m_vCodecCtx = nullptr;
    int m_vStreamIdx = -1;
    SwsContext *m_swsCtx = nullptr;
    PacketQueue m_videoQ;
    
    // Audio
    AVCodecContext *m_aCodecCtx = nullptr;
    int m_aStreamIdx = -1;
    SwrContext *m_swrCtx = nullptr;
    PacketQueue m_audioQ;
    SDL_AudioDeviceID m_audioDevID = 0;
    double m_audioClock; 
    
    // Audio Buffer for SDL
    uint8_t *m_audioBuf = nullptr;
    unsigned int m_audioBufSize = 0;
    unsigned int m_audioBufIndex = 0;
    unsigned int m_audioBufCapacity = 0; // Capacity to avoid repeated alloc
    
    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLTexture *m_texY = nullptr;
    QOpenGLTexture *m_texU = nullptr;
    QOpenGLTexture *m_texV = nullptr;
    GLuint m_posAttr = 0;
    GLuint m_texAttr = 0;
    GLuint m_texYLoc = 0;
    GLuint m_texULoc = 0;
    GLuint m_texVLoc = 0;

    QByteArray m_currentYUV;
    int m_videoW = 0;
    int m_videoH = 0;

    // CPU preview conversion (YUV420P -> BGRA), for reliable scrubbing display via QLabel
    SwsContext *m_rgbSwsCtx = nullptr;
    int m_rgbW = 0;
    int m_rgbH = 0;
};
