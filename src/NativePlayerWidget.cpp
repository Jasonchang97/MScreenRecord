#include "NativePlayerWidget.h"
#include "LogManager.h"
#include <QDebug>
#include <QDateTime>
#include <QElapsedTimer>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>

// TRACE LOGGING
static void trace(const QString& msg) {
    QString formatted = QString("%1 [%2] %3").arg(
        QDateTime::currentDateTime().toStringEx("HH:mm:ss.zzz"),
        QString::number((quintptr)QThread::currentThreadId()),
        msg
    );
    LogManager::instance().write(formatted, LogManager::Player);
}

// --- PacketQueue Implementation ---
void PacketQueue::put(AVPacket *pkt) {
    QMutexLocker lock(&mutex);
    if (abort_request) return;
    AVPacket pkt1;
    av_init_packet(&pkt1);
    // Refcounted copy so caller can safely unref its pkt
    // Special case: EOF/flush packet (data=null, size=0) doesn't carry buffers.
    if (pkt && pkt->data == nullptr && pkt->size == 0) {
        pkt1.data = nullptr;
        pkt1.size = 0;
        pkt1.stream_index = pkt->stream_index;
        pkt1.pts = pkt->pts;
        pkt1.dts = pkt->dts;
        pkt1.duration = pkt->duration;
        pkt1.flags = pkt->flags;
    } else {
        if (av_packet_ref(&pkt1, pkt) < 0) {
            return;
        }
    }
    queue.push_back(pkt1);
    size += pkt1.size;
    cond.wakeOne();
}
int PacketQueue::get(AVPacket *pkt, bool block) {
    QMutexLocker lock(&mutex);
    while (true) {
        if (abort_request) return -1;
        if (!queue.empty()) {
            *pkt = queue.front();
            queue.pop_front();
            size -= pkt->size;
            return 1;
        }
        if (!block) return 0;
        cond.wakeOne(); // Wake wait to check abort
        cond.wait(&mutex);
    }
}
void PacketQueue::clear() {
    QMutexLocker lock(&mutex);
    while (!queue.empty()) {
        AVPacket pkt = queue.front();
        queue.pop_front();
        av_packet_unref(&pkt);
    }
    size = 0;
}
int PacketQueue::count() {
    QMutexLocker lock(&mutex);
    return queue.size();
}
void PacketQueue::abort() {
    QMutexLocker lock(&mutex);
    abort_request = true;
    cond.wakeAll();
}
void PacketQueue::start() {
    QMutexLocker lock(&mutex);
    abort_request = false;
    // ensure packets are unref'd
    while (!queue.empty()) {
        AVPacket pkt = queue.front();
        queue.pop_front();
        av_packet_unref(&pkt);
    }
    size = 0;
}

// --- Shader Sources ---
static const char *vertexShaderSource =
    "attribute vec4 vertexIn;\n"
    "attribute vec2 textureIn;\n"
    "varying vec2 textureOut;\n"
    "void main(void) {\n"
    "    gl_Position = vertexIn;\n"
    "    textureOut = textureIn;\n"
    "}\n";

static const char *fragmentShaderSource =
    "varying vec2 textureOut;\n"
    "uniform sampler2D tex_y;\n"
    "uniform sampler2D tex_u;\n"
    "uniform sampler2D tex_v;\n"
    "void main(void) {\n"
    "    vec3 yuv;\n"
    "    vec3 rgb;\n"
    "    yuv.x = texture2D(tex_y, textureOut).r;\n"
    "    yuv.y = texture2D(tex_u, textureOut).r - 0.5;\n"
    "    yuv.z = texture2D(tex_v, textureOut).r - 0.5;\n"
    "    rgb = mat3( 1,       1,         1,\n"
    "                0,       -0.39465,  2.03211,\n"
    "                1.13983, -0.58060,  0) * yuv;\n"
    "    gl_FragColor = vec4(rgb, 1);\n"
    "}\n";

// --- NativePlayerWidget Implementation ---

NativePlayerWidget::NativePlayerWidget(QWidget *parent) 
    : QOpenGLWidget(parent), m_isRunning(false), m_isPreviewRunning(true)
{
    // QFile::remove(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/player_trace.txt"); // Handled by LogManager
    trace("NativePlayerWidget Created");
    trace("BUILD_MARKER: 2026-01-14 eof-flush-v2 + no-audio-pts-sync-v2");
    avformat_network_init();
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        qDebug() << "Could not initialize SDL -" << SDL_GetError();
        trace("SDL Init Failed");
    }
    connect(this, &NativePlayerWidget::frameReady, this, &NativePlayerWidget::onFrameReady, Qt::QueuedConnection);
    
    m_previewThread = QThread::create([this](){ previewThreadFunc(); });
    m_previewThread->start();
}

NativePlayerWidget::~NativePlayerWidget() {
    trace("Destructor");
    stopPlay();
    m_isPreviewRunning = false;
    m_semPreview.release();
    if (m_previewThread) { m_previewThread->wait(); delete m_previewThread; }
    
    freePreviewResources();
    SDL_Quit();
    if (m_rgbSwsCtx) { sws_freeContext(m_rgbSwsCtx); m_rgbSwsCtx = nullptr; }
    makeCurrent();
    delete m_texY; delete m_texU; delete m_texV; delete m_program;
    doneCurrent();
}

void NativePlayerWidget::setRange(qint64 startMs, qint64 endMs) {
    m_startMs = startMs;
    m_endMs = endMs;
}

void NativePlayerWidget::startPlay(const QString &filePath) {
    trace(QString("startPlay: %1, Range: %2-%3").arg(filePath).arg(m_startMs).arg(m_endMs));
    stopPlay();
    m_filePath = filePath;
    m_isRunning = true;
    m_isPreviewActive = false; // Clear preview flag when starting playback
    m_audioClock = 0; // Fix: Reset audio clock to prevent fast playback
    m_currentYUV.clear();
    
    m_videoQ.start();
    m_audioQ.start(); // Audio Queue Start
    
    m_readThread = QThread::create([this](){ readThreadFunc(); });
    m_readThread->start();
}

void NativePlayerWidget::freePreviewResources() {
    if (m_previewSwsCtx) { sws_freeContext(m_previewSwsCtx); m_previewSwsCtx = nullptr; }
    if (m_previewCodecCtx) { avcodec_free_context(&m_previewCodecCtx); m_previewCodecCtx = nullptr; }
    if (m_previewFmtCtx) { avformat_close_input(&m_previewFmtCtx); m_previewFmtCtx = nullptr; }
    m_previewStreamIdx = -1;
    m_currentOpenPath.clear();
}

void NativePlayerWidget::seek(const QString &filePath, qint64 ms) {
    // Don't call stopPlay() here - Preview is independent from playback
    // stopPlay() would clear resources and block, interfering with preview
    
    trace(QString("seek called: file=%1, ms=%2").arg(filePath).arg(ms));
    
    m_isPreviewActive = true; // Mark preview as active
    
    QMutexLocker lock(&m_previewMutex);
    m_previewPath = filePath;
    m_reqPreviewMs = ms;
    m_semPreview.release(); // Wake up preview thread
}

void NativePlayerWidget::previewThreadFunc() {
    trace("Preview Thread Start");
    
    while (m_isPreviewRunning) {
        m_semPreview.acquire();
        if (!m_isPreviewRunning) break;
        
        int64_t ms = m_reqPreviewMs.load();
        
        // Debounce
        while (m_semPreview.tryAcquire()) {
             ms = m_reqPreviewMs.load();
        }
        
        if (ms < 0) continue;
        
        QString filePath;
        {
            QMutexLocker lock(&m_previewMutex);
            filePath = m_previewPath;
        }
        if (filePath.isEmpty()) continue;

        // 1. Ensure Preview Context
        if (m_currentOpenPath != filePath || !m_previewFmtCtx) {
            freePreviewResources();
            m_currentOpenPath = filePath;
            
            if (avformat_open_input(&m_previewFmtCtx, filePath.toUtf8().constData(), nullptr, nullptr) < 0) {
                 if (avformat_open_input(&m_previewFmtCtx, filePath.toLocal8Bit().constData(), nullptr, nullptr) < 0) continue;
            }
            if (avformat_find_stream_info(m_previewFmtCtx, nullptr) < 0) { freePreviewResources(); continue; }
            
            m_previewStreamIdx = -1;
            for(unsigned int i=0; i<m_previewFmtCtx->nb_streams; i++) {
                if(m_previewFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) { m_previewStreamIdx=i; break; }
            }
            if(m_previewStreamIdx < 0) { freePreviewResources(); continue; }
            
            AVCodec *dec = avcodec_find_decoder(m_previewFmtCtx->streams[m_previewStreamIdx]->codecpar->codec_id);
            m_previewCodecCtx = avcodec_alloc_context3(dec);
            avcodec_parameters_to_context(m_previewCodecCtx, m_previewFmtCtx->streams[m_previewStreamIdx]->codecpar);
            avcodec_open2(m_previewCodecCtx, dec, nullptr);
        }
        
        if (!m_previewFmtCtx || !m_previewCodecCtx) continue;

        // 2. Seek (align to stream start_time if present)
        AVStream *st = m_previewFmtCtx->streams[m_previewStreamIdx];
        int64_t startTs = (st && st->start_time != AV_NOPTS_VALUE) ? st->start_time : 0;
        int64_t ts = ms * 1000;
        ts = av_rescale_q(ts, AVRational{1, 1000000}, st->time_base) + startTs;
        
        auto decodeNearestFrame = [&](int seekFlags, int64_t seekTargetTs) -> bool {
            // Robust Seek: Use avformat_seek_file which is generally better than av_seek_frame for complex containers
            // Try seeking to [INT64_MIN, seekTargetTs, seekTargetTs] with BACKWARD flag to ensure we land before target
            int seekRet = avformat_seek_file(m_previewFmtCtx, m_previewStreamIdx, INT64_MIN, seekTargetTs, seekTargetTs, seekFlags);
            if (seekRet < 0) {
                 // Fallback: relax boundaries
                 seekRet = avformat_seek_file(m_previewFmtCtx, m_previewStreamIdx, INT64_MIN, seekTargetTs, INT64_MAX, seekFlags);
            }
            if (seekRet < 0) return false;
            
            avcodec_flush_buffers(m_previewCodecCtx);

            AVPacket pkt; av_init_packet(&pkt);
            AVFrame *frame = av_frame_alloc();
            AVFrame *candidateFrame = av_frame_alloc(); // Holds the best "previous" frame seen so far
            bool hasCandidate = false;
            
            int maxPackets = 1000; // Cap to avoid freezing on bad files
            bool foundTarget = false;

            while (maxPackets-- > 0 && av_read_frame(m_previewFmtCtx, &pkt) >= 0) {
                if (pkt.stream_index == m_previewStreamIdx) {
                    if (avcodec_send_packet(m_previewCodecCtx, &pkt) == 0) {
                        while (avcodec_receive_frame(m_previewCodecCtx, frame) == 0) {
                            int64_t pts = frame->best_effort_timestamp;
                            if (pts == AV_NOPTS_VALUE) pts = startTs;
                            
                            // Check if this frame is "past" the target
                            // Use a small tolerance for "approx match" if needed, but strictly <= is safer for "previous"
                            // However, we convert to ms for comparison to be consistent
                            int64_t adjPts = pts - startTs;
                            if (adjPts < 0) adjPts = 0;
                            double sec = adjPts * av_q2d(st->time_base);
                            int64_t frameMs = (int64_t)(sec * 1000);
                            
                            if (frameMs > ms) {
                                // We passed the target. 
                                // If we have a candidate (a frame <= ms), use it.
                                // If not (e.g. seek landed after target), we might have to use this one or stop.
                                if (hasCandidate) {
                                    foundTarget = true; // Stop loop, we have our "nearest previous"
                                } else {
                                    // Seeking landed us after target, and this is the first frame we see.
                                    // Use it as fallback.
                                    av_frame_unref(candidateFrame);
                                    av_frame_ref(candidateFrame, frame);
                                    hasCandidate = true;
                                    foundTarget = true;
                                }
                            } else {
                                // Frame is <= target. This is a good candidate.
                                // Since receive_frame usually returns frames in order, this new one is "closer" to target than previous candidate.
                                av_frame_unref(candidateFrame);
                                av_frame_ref(candidateFrame, frame);
                                hasCandidate = true;
                                // If exact match, we can stop early (optional, but robust to keep going until pass or EOF to be sure of order)
                                // But usually decoding order is monotonic.
                                if (frameMs == ms) foundTarget = true;
                            }
                            
                            if (foundTarget) break;
                        }
                    }
                }
                av_packet_unref(&pkt);
                if (foundTarget) break;
            }
            av_packet_unref(&pkt); // cleanup last packet

            // If we ran out of packets (EOF) but have a candidate, use it.
            // If we didn't find "target passed" but have candidate, it means we reached end of file/stream and candidate is the last frame. Use it.
            
            trace(QString("PreviewTry: ms=%1, found=%2, hasCand=%3").arg(ms).arg(foundTarget).arg(hasCandidate));
            
            if (!hasCandidate && !foundTarget) {
                 // Severe Fallback: If no packets read (e.g. seek to end of file where no frames exist),
                 // try to decode the LAST few frames of the file by seeking a bit earlier.
                 // Only do this if we haven't already tried a broad seek.
                 if (seekFlags != 0) { // If we were being specific (backward), try relaxed.
                      // Already handled by caller's fallback loop? No, caller toggles flags.
                      // Here we might be at EOF.
                      // Let's rely on the caller's fallback to decodeNearestFrame(0, ...) or similar.
                 }
            }

            bool emitted = false;
            if (hasCandidate && candidateFrame->width > 0) {
                if (!m_previewSwsCtx) {
                    m_previewSwsCtx = sws_getContext(candidateFrame->width, candidateFrame->height, (AVPixelFormat)candidateFrame->format,
                                                     candidateFrame->width, candidateFrame->height, AV_PIX_FMT_YUV420P,
                                                     SWS_BICUBIC, nullptr, nullptr, nullptr);
                } else if (m_previewCodecCtx->width != candidateFrame->width || m_previewCodecCtx->height != candidateFrame->height) {
                    sws_freeContext(m_previewSwsCtx);
                    m_previewSwsCtx = sws_getContext(candidateFrame->width, candidateFrame->height, (AVPixelFormat)candidateFrame->format,
                                                     candidateFrame->width, candidateFrame->height, AV_PIX_FMT_YUV420P,
                                                     SWS_BICUBIC, nullptr, nullptr, nullptr);
                }

                int w = candidateFrame->width;
                int h = candidateFrame->height;
                QByteArray yuv; yuv.resize(w * h * 3 / 2);
                uint8_t *dst[4] = { (uint8_t*)yuv.data(), (uint8_t*)yuv.data() + w*h, (uint8_t*)yuv.data() + w*h*5/4, nullptr };
                int lines[4] = { w, w/2, w/2, 0 };
                sws_scale(m_previewSwsCtx, candidateFrame->data, candidateFrame->linesize, 0, h, dst, lines);
                trace(QString("Preview emit: ms=%1 w=%2 h=%3").arg(ms).arg(w).arg(h));
                emit frameReady(yuv, w, h);
                emitted = true;
            }

            av_frame_free(&frame);
            av_frame_free(&candidateFrame);
            return emitted;
        };

        bool ok = decodeNearestFrame(AVSEEK_FLAG_BACKWARD, ts);
        if (!ok) {
            // Fallback: try default seek if backward fails
            ok = decodeNearestFrame(0, ts);
        }
        if (!ok) {
            trace(QString("Preview failed to decode any frame for ms=%1").arg(ms));
        }

        // Preview stays active until next startPlay() (to prevent any stale playback frames from overriding)
    }
    trace("Preview Thread End");
}

void NativePlayerWidget::stopPlay() {
    trace("stopPlay called");
    m_isRunning = false;
    m_videoQ.abort();
    m_audioQ.abort(); // Abort audio
    
    if (m_audioDevID != 0) {
        SDL_CloseAudioDevice(m_audioDevID);
        m_audioDevID = 0;
    }

    if (m_readThread) { m_readThread->wait(); delete m_readThread; m_readThread = nullptr; }
    if (m_videoThread) { m_videoThread->wait(); delete m_videoThread; m_videoThread = nullptr; }
    
    freeResources();
    // m_currentYUV.clear(); // Fix: Don't clear last frame to prevent black flash/screen on seek
    update();
    trace("stopPlay finished");
}

void NativePlayerWidget::freeResources() {
    trace("freeResources");
    m_videoQ.clear();
    m_audioQ.clear();
    m_audioClock = 0; // Fix: Ensure clock is reset
    if (m_swsCtx) { sws_freeContext(m_swsCtx); m_swsCtx = nullptr; }
    if (m_swrCtx) { swr_free(&m_swrCtx); m_swrCtx = nullptr; }
    if (m_vCodecCtx) { avcodec_free_context(&m_vCodecCtx); m_vCodecCtx = nullptr; }
    if (m_aCodecCtx) { avcodec_free_context(&m_aCodecCtx); m_aCodecCtx = nullptr; }
    if (m_fmtCtx) { avformat_close_input(&m_fmtCtx); m_fmtCtx = nullptr; }
    if (m_audioBuf) { av_free(m_audioBuf); m_audioBuf = nullptr; }
    m_audioBufCapacity = 0;
    m_audioBufSize = 0;
    m_audioBufIndex = 0;
}

// --- Audio Callback ---
void NativePlayerWidget::sdlAudioCallback(void *opaque, Uint8 *stream, int len) {
    NativePlayerWidget *is = (NativePlayerWidget *)opaque;
    if (!is) { memset(stream, 0, len); return; } // Paranoia check

    // CRITICAL: Protect against race conditions with stopPlay/freeResources
    // We cannot use a QMutex here because it might block real-time audio thread for too long.
    // Instead, we rely on m_isRunning flag and atomic checks if possible, or just be very careful.
    if (!is->m_isRunning) {
        memset(stream, 0, len);
        return;
    }

    int len1, audio_size;

    while (len > 0) {
        if (is->m_audioBufIndex >= is->m_audioBufSize) {
            audio_size = -1;
            
            // Check m_isRunning again inside loop
            if (is->m_isRunning) {
                AVPacket pkt;
                av_init_packet(&pkt); pkt.data = nullptr; pkt.size = 0;
                AVFrame *frame = av_frame_alloc();
                
                while (audio_size < 0) {
                    int ret = is->m_audioQ.get(&pkt, false);
                    if (ret <= 0) break; // 0=Empty, -1=Aborted. Handles garbage pkt issue.
                    
                    // Double check context validity
                    if (is->m_aCodecCtx && avcodec_send_packet(is->m_aCodecCtx, &pkt) == 0) {
                        while (avcodec_receive_frame(is->m_aCodecCtx, frame) == 0) {
                            if (!is->m_swrCtx) {
                                int64_t channel_layout = frame->channel_layout;
                                if (channel_layout == 0) channel_layout = av_get_default_channel_layout(frame->channels);
                                
                                is->m_swrCtx = swr_alloc_set_opts(nullptr,
                                    AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, frame->sample_rate,
                                    channel_layout, (AVSampleFormat)frame->format, frame->sample_rate,
                                    0, nullptr);
                                swr_init(is->m_swrCtx);
                            }
                            
                            if (is->m_swrCtx) {
                                int out_count = (int64_t)frame->nb_samples * frame->sample_rate / frame->sample_rate + 256;
                                int out_size = av_samples_get_buffer_size(nullptr, 2, out_count, AV_SAMPLE_FMT_S16, 0);
                                
                                if (out_size < 0) break;

                                // Ensure buffer is large enough
                                // Race condition risk: m_audioBuf access. 
                                // Ideally we should lock, but assuming single consumer/producer if start/stop is serialized.
            if (out_size > (int)is->m_audioBufCapacity) {
                                    av_free(is->m_audioBuf);
                                    is->m_audioBuf = (uint8_t *)av_malloc(out_size);
                                    is->m_audioBufCapacity = out_size;
                                }
                                
                                if (is->m_audioBuf) {
                                    int len2 = swr_convert(is->m_swrCtx, &is->m_audioBuf, out_count, 
                                                         (const uint8_t **)frame->data, frame->nb_samples);
                                    if (len2 > 0) {
                                        is->m_audioBufSize = len2 * 2 * 2;
                                        audio_size = is->m_audioBufSize;
                                        
                                        double pts = frame->best_effort_timestamp;
                                        if (pts == AV_NOPTS_VALUE) pts = 0;
                                        if (is->m_fmtCtx && is->m_aStreamIdx >= 0 && is->m_aStreamIdx < (int)is->m_fmtCtx->nb_streams) {
                                            double frameTime = pts * av_q2d(is->m_fmtCtx->streams[is->m_aStreamIdx]->time_base);
                                            is->m_audioClock = frameTime;
                                            is->m_audioClock += (double)audio_size / (2 * 2 * frame->sample_rate);
                                            
                                            // Pre-roll Audio
                                            if (is->m_startMs > 0 && frameTime * 1000 < is->m_startMs - 50) {
                                                audio_size = -1; // Reset to loop again
                                                // Don't free m_audioBuf here as it's reused/resized
                                                // Just ignore this data
                                                continue;
                                            }
                                        }
                                    }
                                }
                            }
                            break; 
                        }
                    }
                    av_packet_unref(&pkt);
                    if (audio_size >= 0) break;
                }
                av_frame_free(&frame);
            }

            if (audio_size < 0) {
                is->m_audioBufSize = 1024;
                if (is->m_audioBufCapacity < 1024) {
                    av_free(is->m_audioBuf);
                    is->m_audioBuf = (uint8_t*)av_malloc(1024);
                    is->m_audioBufCapacity = 1024;
                }
                if (is->m_audioBuf) memset(is->m_audioBuf, 0, 1024);
                audio_size = 1024;
                
                // Fix: Update clock during silence to prevent video sync stall
                if (is->m_aCodecCtx && is->m_aCodecCtx->sample_rate > 0) {
                     is->m_audioClock += (double)audio_size / (is->m_aCodecCtx->sample_rate * 2 * 2);
                }
            }
            is->m_audioBufIndex = 0;
        }
        
        len1 = is->m_audioBufSize - is->m_audioBufIndex;
        if (len1 > len) len1 = len;
        
        if (is->m_audioBuf) 
            memcpy(stream, (uint8_t *)is->m_audioBuf + is->m_audioBufIndex, len1);
        else 
            memset(stream, 0, len1);
            
        len -= len1;
        stream += len1;
        is->m_audioBufIndex += len1;
    }
}

double NativePlayerWidget::getAudioClock() {
    // Current audio clock = clock when frame loaded + bytes consumed since then
    double pts = m_audioClock;
    int hw_buf_size = m_audioBufSize - m_audioBufIndex;
    int bytes_per_sec = 44100 * 2 * 2; // Approximate if not dynamic
    // Better: store sample rate in context
    if (m_aCodecCtx) bytes_per_sec = m_aCodecCtx->sample_rate * 2 * 2;
    if (bytes_per_sec > 0)
        pts -= (double)hw_buf_size / bytes_per_sec;
    return pts;
}

void NativePlayerWidget::readThreadFunc() {
    trace("readThreadFunc Start");
    if (avformat_open_input(&m_fmtCtx, m_filePath.toUtf8().constData(), nullptr, nullptr) < 0) {
        if (avformat_open_input(&m_fmtCtx, m_filePath.toLocal8Bit().constData(), nullptr, nullptr) < 0) {
            emit errorOccurred("无法打开文件: " + m_filePath);
            trace("Open Input Failed");
            return;
        }
    }
    if (avformat_find_stream_info(m_fmtCtx, nullptr) < 0) return;

    // Find Video Stream
    m_vStreamIdx = -1;
    for (unsigned int i = 0; i < m_fmtCtx->nb_streams; i++) {
        if (m_fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && m_vStreamIdx < 0) m_vStreamIdx = i;
    }

    // Find Audio Stream
    m_aStreamIdx = -1;
    for (unsigned int i = 0; i < m_fmtCtx->nb_streams; i++) {
        if (m_fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && m_aStreamIdx < 0) m_aStreamIdx = i;
    }

    // Open Video
    if (m_vStreamIdx >= 0) {
        AVCodec *dec = avcodec_find_decoder(m_fmtCtx->streams[m_vStreamIdx]->codecpar->codec_id);
        m_vCodecCtx = avcodec_alloc_context3(dec);
        avcodec_parameters_to_context(m_vCodecCtx, m_fmtCtx->streams[m_vStreamIdx]->codecpar);
        m_vCodecCtx->thread_count = 1; // Single thread
        avcodec_open2(m_vCodecCtx, dec, nullptr);
        m_videoThread = QThread::create([this](){ videoThreadFunc(); });
        m_videoThread->start();
    }
    
    // Open Audio & SDL
    if (m_aStreamIdx >= 0) {
        AVCodec *dec = avcodec_find_decoder(m_fmtCtx->streams[m_aStreamIdx]->codecpar->codec_id);
        m_aCodecCtx = avcodec_alloc_context3(dec);
        avcodec_parameters_to_context(m_aCodecCtx, m_fmtCtx->streams[m_aStreamIdx]->codecpar);
        m_aCodecCtx->thread_count = 1; // Single thread
        avcodec_open2(m_aCodecCtx, dec, nullptr);
        
        SDL_AudioSpec wanted_spec, spec;
        wanted_spec.freq = m_aCodecCtx->sample_rate;
        wanted_spec.format = AUDIO_S16SYS;
        wanted_spec.channels = 2; // Force stereo
        wanted_spec.silence = 0;
        wanted_spec.samples = 1024;
        wanted_spec.callback = sdlAudioCallback;
        wanted_spec.userdata = this;
        
        m_audioDevID = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
        if (m_audioDevID > 0) {
            SDL_PauseAudioDevice(m_audioDevID, 0);
            trace("SDL Playback Device Opened");
        } else {
             emit logMessage("SDL OpenAudioDevice failed");
             trace(QString("SDL Playback Open Failed: %1").arg(SDL_GetError()));
             // If audio device can't be opened, treat as NO-AUDIO playback:
             // - avoid accumulating audio packets forever
             // - keep logic consistent with hasAudioClock==false
             if (m_aCodecCtx) { avcodec_free_context(&m_aCodecCtx); m_aCodecCtx = nullptr; }
             m_aStreamIdx = -1;
             m_audioQ.clear();
        }
    }

    // Seek Initial (align with stream start_time if present)
    if (m_startMs > 0 && m_vStreamIdx >= 0 && m_vStreamIdx < (int)m_fmtCtx->nb_streams) {
        AVStream *st = m_fmtCtx->streams[m_vStreamIdx];
        int64_t startTs = (st && st->start_time != AV_NOPTS_VALUE) ? st->start_time : 0;
        trace(QString("readThreadFunc Initial Seek: %1 (start_time=%2)").arg(m_startMs).arg(startTs));
        int64_t ts = av_rescale_q(m_startMs * 1000, AVRational{1, 1000000}, st->time_base) + startTs;
        avformat_seek_file(m_fmtCtx, m_vStreamIdx, INT64_MIN, ts, INT64_MAX, 0);
    }

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = nullptr; pkt.size = 0; // Ensure clean init

    bool reachedEof = false;
    while (m_isRunning) {
        bool videoFull = (m_vStreamIdx >= 0 && m_videoQ.count() > 100);
        bool audioFull = (m_aStreamIdx >= 0 && m_audioQ.count() > 100);

        if ((m_vStreamIdx < 0 || videoFull) && (m_aStreamIdx < 0 || audioFull)) {
            av_usleep(10000);
            continue;
        }
        int rr = av_read_frame(m_fmtCtx, &pkt);
        if (rr < 0) {
            char errbuf[256] = {0};
            av_strerror(rr, errbuf, sizeof(errbuf));
            trace(QString("readThreadFunc: av_read_frame failed rr=%1 err=%2").arg(rr).arg(QString::fromUtf8(errbuf)));
            reachedEof = (rr == AVERROR_EOF);
            break;
        }
        
        if (pkt.stream_index == m_vStreamIdx) {
            m_videoQ.put(&pkt);
        } else if (pkt.stream_index == m_aStreamIdx) {
            m_audioQ.put(&pkt);
        }
        // Always unref caller-owned packet (queue holds its own ref)
        av_packet_unref(&pkt);
    }
    // while (m_isRunning) av_usleep(100000); // Removed to allow natural thread exit
    trace(QString("readThreadFunc Exit (EOF=%1, running=%2)").arg(reachedEof ? 1 : 0).arg(m_isRunning ? 1 : 0));

    // If reached EOF, push flush packets so decoders can drain and finish naturally.
    if (reachedEof && m_isRunning) {
        if (m_vStreamIdx >= 0) {
            AVPacket flushPkt;
            av_init_packet(&flushPkt);
            flushPkt.data = nullptr;
            flushPkt.size = 0;
            flushPkt.stream_index = m_vStreamIdx;
            m_videoQ.put(&flushPkt);
        }
        if (m_aStreamIdx >= 0) {
            AVPacket flushPkt;
            av_init_packet(&flushPkt);
            flushPkt.data = nullptr;
            flushPkt.size = 0;
            flushPkt.stream_index = m_aStreamIdx;
            m_audioQ.put(&flushPkt);
        }
    }
}

void NativePlayerWidget::videoThreadFunc() {
    trace(QString("videoThreadFunc Start, m_startMs: %1").arg(m_startMs));
    AVPacket pkt;
    av_init_packet(&pkt); pkt.data = nullptr; pkt.size = 0;
    AVFrame *frame = av_frame_alloc();
    double pts = 0; // seconds from file 0 (after start_time adjustment)

    // If no audio device/stream, drive playback purely by video timing vs wall clock
    const bool hasAudioClock = (m_aStreamIdx >= 0 && m_audioDevID > 0);
    trace(QString("videoThreadFunc: hasAudioClock=%1, endMs=%2").arg(hasAudioClock ? 1 : 0).arg(m_endMs));
    QElapsedTimer wallTimer;
    bool wallStarted = false;
    double basePts = 0.0;
    double lastPts = 0.0;
    bool hasLastPts = false;
    // Estimate frame duration for monotonic fallback
    double frameDur = 1.0 / 30.0;
    if (m_fmtCtx && m_vStreamIdx >= 0 && m_vStreamIdx < (int)m_fmtCtx->nb_streams) {
        AVRational fr = m_fmtCtx->streams[m_vStreamIdx]->avg_frame_rate;
        if (fr.num == 0 || fr.den == 0) fr = m_fmtCtx->streams[m_vStreamIdx]->r_frame_rate;
        double fps = (fr.num > 0 && fr.den > 0) ? av_q2d(fr) : 0.0;
        if (fps > 1.0 && fps < 240.0) frameDur = 1.0 / fps;
    }
    
    auto computePtsSecFromFrame = [&](AVFrame *f) -> double {
        if (!m_fmtCtx || m_vStreamIdx < 0 || m_vStreamIdx >= (int)m_fmtCtx->nb_streams) return 0.0;
        AVStream *st = m_fmtCtx->streams[m_vStreamIdx];
        int64_t startTs = (st && st->start_time != AV_NOPTS_VALUE) ? st->start_time : 0;
        int64_t bestPts = f->best_effort_timestamp;
        if (bestPts == AV_NOPTS_VALUE) bestPts = startTs;
        int64_t adjPts = bestPts - startTs;
        if (adjPts < 0) adjPts = 0;
        return adjPts * av_q2d(st->time_base);
    };

    // For "no-audio" playback, some files have broken/sparse PTS which causes speed wobble/early stop.
    // We detect "bad PTS" and switch to a stable frame-clock (based on estimated FPS).
    bool useFrameClock = false;
    int ptsSamples = 0;
    int ptsBad = 0;
    int64_t playedFrames = 0; // counts frames AFTER preroll

    while (m_isRunning) {
        if (m_videoQ.get(&pkt, true) < 0) break; // Correctly handle abort (-1)
        if (!m_isRunning) break;

        // Flush packet indicates EOF
        if (pkt.data == nullptr && pkt.size == 0) {
            trace("videoThreadFunc: got flush packet, draining decoder");
            if (m_vCodecCtx) {
                avcodec_send_packet(m_vCodecCtx, nullptr);
                while (avcodec_receive_frame(m_vCodecCtx, frame) == 0) {
                    pts = computePtsSecFromFrame(frame);
                    if (!hasAudioClock) {
                        // Collect PTS quality stats (before any correction)
                        if (hasLastPts) {
                            double delta = pts - lastPts;
                            ptsSamples++;
                            if (delta <= 0.0 || delta > (frameDur * 10.0)) ptsBad++;
                            if (!useFrameClock && ptsSamples >= 30 && ptsBad * 5 >= ptsSamples) { // >=20%
                                useFrameClock = true;
                                trace(QString("no-audio: switch to frameClock (bad=%1/%2, frameDur=%3)")
                                          .arg(ptsBad).arg(ptsSamples).arg(frameDur));
                            }
                        }
                        lastPts = pts;
                        hasLastPts = true;
                    }
                    // Convert to ms (absolute)
                    int64_t videoMsAbs = (int64_t)(pts * 1000.0);

                    if (!m_swsCtx) {
                        m_swsCtx = sws_getContext(m_vCodecCtx->width, m_vCodecCtx->height, m_vCodecCtx->pix_fmt,
                                                  m_vCodecCtx->width, m_vCodecCtx->height, AV_PIX_FMT_YUV420P,
                                                  SWS_BICUBIC, nullptr, nullptr, nullptr);
                    }
                    int w = m_vCodecCtx->width;
                    int h = m_vCodecCtx->height;
                    QByteArray yuv; yuv.resize(w * h * 3 / 2);
                    uint8_t *dst[4] = { (uint8_t*)yuv.data(), (uint8_t*)yuv.data() + w*h, (uint8_t*)yuv.data() + w*h*5/4, nullptr };
                    int lines[4] = { w, w/2, w/2, 0 };
                    sws_scale(m_swsCtx, frame->data, frame->linesize, 0, h, dst, lines);

                    if (!m_isPreviewActive) {
                        emit frameReady(yuv, w, h);
                        emit positionChanged((qint64)videoMsAbs);
                    }
                }
            }
            m_isRunning = false;
            emit playbackFinished();
            av_packet_unref(&pkt);
            break;
        }

        if (avcodec_send_packet(m_vCodecCtx, &pkt) == 0) {
            while (avcodec_receive_frame(m_vCodecCtx, frame) == 0) {
                pts = computePtsSecFromFrame(frame);

                // No-audio mode: stabilize bad timestamps to avoid speed wobble / early end.
                if (!hasAudioClock) {
                    // Assess PTS quality; switch to frame clock if too many bad deltas
                    if (!hasLastPts) {
                        hasLastPts = true;
                        lastPts = pts;
                    } else {
                        double delta = pts - lastPts;
                        ptsSamples++;
                        if (delta <= 0.0 || delta > (frameDur * 10.0)) ptsBad++;
                        if (!useFrameClock && ptsSamples >= 30 && ptsBad * 5 >= ptsSamples) { // >=20%
                            useFrameClock = true;
                            trace(QString("no-audio: switch to frameClock (bad=%1/%2, frameDur=%3)")
                                      .arg(ptsBad).arg(ptsSamples).arg(frameDur));
                        }
                        lastPts = pts;
                    }
                }

                // Pre-roll Video (Skip frames before start time) - use PTS-derived absolute ms
                int64_t videoMsAbsPts = (int64_t)(pts * 1000.0);
                if (m_startMs > 0 && videoMsAbsPts < m_startMs - 50) {
                    continue;
                }

                // After preroll, advance frame counter
                playedFrames++;

                // Choose the time used for sync/progress/end:
                // - If PTS is sane: use PTS-derived ms
                // - If PTS is bad: use stable frame clock based on FPS estimate
                int64_t videoMsAbs = videoMsAbsPts;
                int64_t relMs = videoMsAbs - m_startMs;
                if (!hasAudioClock && useFrameClock) {
                    double relSec = (double)(playedFrames - 1) * frameDur; // first displayed frame -> 0
                    relMs = (int64_t)(relSec * 1000.0);
                    videoMsAbs = m_startMs + relMs;
                }
                if (relMs < 0) relMs = 0;

                if (m_endMs > 0 && videoMsAbs > m_endMs) {
                    trace(QString("Playback Finished: ms=%1, End=%2").arg(videoMsAbs).arg(m_endMs));
                    m_isRunning = false;
                    emit playbackFinished();
                    break;
                }

                // Sync (before expensive sws_scale): avoid "catch-up bursts" in no-audio mode.
                bool dropThisFrame = false;
                if (hasAudioClock) {
                    double audioClock = getAudioClock();
                    double diff = (relMs / 1000.0) - audioClock;
                    if (diff > 0.01) {
                        av_usleep((int64_t)(diff * 1000000));
                    } else if (diff < -0.20) {
                        // We're far behind audio clock -> drop this video frame to catch up
                        dropThisFrame = true;
                    }
                } else {
                    // No-audio: sync to wall clock driven by relative timeline
                    double relSec = (relMs / 1000.0);
                    if (!wallStarted) {
                        wallTimer.start();
                        basePts = relSec; // Base our wall clock on the FIRST encountered frame's timestamp
                        wallStarted = true;
                    }
                    // Calculate target elapsed time since start (based on PTS delta from first frame)
                    int64_t targetElapsedUs = (int64_t)((relSec - basePts) * 1000000.0);
                    // Actual wall time elapsed since we started playing
                    int64_t wallElapsedUs = (int64_t)(wallTimer.nsecsElapsed() / 1000);
                    
                    int64_t sleepUs = targetElapsedUs - wallElapsedUs;
                    if (sleepUs > 0) {
                        av_usleep(sleepUs);
                    } else {
                        // If we're too late, DO NOT "play fast" to catch up.
                        // Drop frames until we're near real-time again.
                        int64_t lateUs = -sleepUs;
                        if (lateUs > 200000) { // 200ms late => drop to keep perceived speed stable
                            dropThisFrame = true;
                            // If we drop, we don't reset wall clock. We just skip rendering.
                            // Next frame's targetElapsedUs will be larger, hopefully matching wallElapsedUs better.
                            // If we are chronically behind, we drop frames.
                        }
                    }
                }
                if (dropThisFrame) {
                    // Skip rendering this frame; time will jump on next rendered frame, avoiding fast/slow wobble.
                    continue;
                }

                if (!m_swsCtx) {
                     m_swsCtx = sws_getContext(m_vCodecCtx->width, m_vCodecCtx->height, m_vCodecCtx->pix_fmt,
                                               m_vCodecCtx->width, m_vCodecCtx->height, AV_PIX_FMT_YUV420P,
                                               SWS_BICUBIC, nullptr, nullptr, nullptr);
                }
                
                int w = m_vCodecCtx->width;
                int h = m_vCodecCtx->height;
                QByteArray yuv; yuv.resize(w * h * 3 / 2);
                uint8_t *dst[4] = { (uint8_t*)yuv.data(), (uint8_t*)yuv.data() + w*h, (uint8_t*)yuv.data() + w*h*5/4, nullptr };
                int lines[4] = { w, w/2, w/2, 0 };
                sws_scale(m_swsCtx, frame->data, frame->linesize, 0, h, dst, lines);
                
                // Don't send playback frames if preview is active (preview takes priority)
                if (!m_isPreviewActive) {
                    emit frameReady(yuv, w, h);
                    emit positionChanged((qint64)videoMsAbs);
                }
            }
        }
        av_packet_unref(&pkt);
    }
    av_frame_free(&frame);
    trace("videoThreadFunc Exit");
}

void NativePlayerWidget::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_posAttr = m_program->attributeLocation("vertexIn");
    m_texAttr = m_program->attributeLocation("textureIn");
    m_texYLoc = m_program->uniformLocation("tex_y");
    m_texULoc = m_program->uniformLocation("tex_u");
    m_texVLoc = m_program->uniformLocation("tex_v");
    m_texY = new QOpenGLTexture(QOpenGLTexture::Target2D); m_texY->create();
    m_texU = new QOpenGLTexture(QOpenGLTexture::Target2D); m_texU->create();
    m_texV = new QOpenGLTexture(QOpenGLTexture::Target2D); m_texV->create();
}
void NativePlayerWidget::resizeGL(int w, int h) { glViewport(0, 0, w, h); }
void NativePlayerWidget::onFrameReady(const QByteArray &data, int width, int height) {
    // Accept all frames: preview thread only sends when preview is active,
    // playback thread skips sending when preview is active (see videoThreadFunc)
    trace(QString("onFrameReady: bytes=%1 w=%2 h=%3 previewActive=%4 running=%5")
              .arg(data.size()).arg(width).arg(height)
              .arg(m_isPreviewActive.load() ? 1 : 0)
              .arg(m_isRunning.load() ? 1 : 0));
    m_currentYUV = data;
    m_videoW = width;
    m_videoH = height;
    update();
    // For preview scrubbing: also emit CPU image for QLabel preview (avoids OpenGL repaint issues on some machines)
    if (!m_isRunning && width > 0 && height > 0 && data.size() >= (width * height * 3 / 2)) {
        if (!m_rgbSwsCtx || m_rgbW != width || m_rgbH != height) {
            if (m_rgbSwsCtx) sws_freeContext(m_rgbSwsCtx);
            m_rgbSwsCtx = sws_getContext(width, height, AV_PIX_FMT_YUV420P,
                                         width, height, AV_PIX_FMT_BGRA,
                                         SWS_BILINEAR, nullptr, nullptr, nullptr);
            m_rgbW = width;
            m_rgbH = height;
        }
        if (m_rgbSwsCtx) {
            QImage img(width, height, QImage::Format_ARGB32);
            const uint8_t *base = (const uint8_t*)data.constData();
            uint8_t *src[4] = {
                (uint8_t*)base,
                (uint8_t*)(base + width * height),
                (uint8_t*)(base + width * height + (width * height) / 4),
                nullptr
            };
            int srcLines[4] = { width, width / 2, width / 2, 0 };
            uint8_t *dst[4] = { (uint8_t*)img.bits(), nullptr, nullptr, nullptr };
            int dstLines[4] = { img.bytesPerLine(), 0, 0, 0 };
            sws_scale(m_rgbSwsCtx, src, srcLines, 0, height, dst, dstLines);
            emit previewImageReady(img);
        }
    }
    // For preview scrubbing, try to paint ASAP
    if (!m_isRunning) repaint();
}
void NativePlayerWidget::paintGL() {
    // Clear whole widget with black background
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_currentYUV.isEmpty()) return;
    
    if (m_videoW > 0 && m_videoH > 0) {
        float widgetW = width();
        float widgetH = height();
        float videoW = m_videoW;
        float videoH = m_videoH;

        float widgetRatio = widgetW / widgetH;
        float videoRatio = videoW / videoH;

        int vpX = 0, vpY = 0, vpW = (int)widgetW, vpH = (int)widgetH;

        if (widgetRatio > videoRatio) {
            // Widget is wider than video -> paddings on left/right
            vpW = (int)(widgetH * videoRatio);
            vpX = (int)((widgetW - vpW) / 2);
        } else {
            // Widget is taller than video -> paddings on top/bottom
            vpH = (int)(widgetW / videoRatio);
            vpY = (int)((widgetH - vpH) / 2);
        }
        glViewport(vpX, vpY, vpW, vpH);
    } else {
        glViewport(0, 0, width(), height());
    }
    
    m_program->bind();
    
    static const GLfloat ver[] = {-1,-1, 1,-1, -1,1, 1,1};
    static const GLfloat tex[] = {0,1, 1,1, 0,0, 1,0};
    
    glVertexAttribPointer(m_posAttr, 2, GL_FLOAT, GL_FALSE, 0, ver);
    glEnableVertexAttribArray(m_posAttr);
    glVertexAttribPointer(m_texAttr, 2, GL_FLOAT, GL_FALSE, 0, tex);
    glEnableVertexAttribArray(m_texAttr);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const uint8_t *d = (const uint8_t*)m_currentYUV.constData();
    int w = m_videoW, h = m_videoH;
    int y = w*h;
    
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, m_texY->textureId());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, d);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, m_texU->textureId());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w/2, h/2, 0, GL_RED, GL_UNSIGNED_BYTE, d+y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, m_texV->textureId());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w/2, h/2, 0, GL_RED, GL_UNSIGNED_BYTE, d+y+y/4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glUniform1i(m_texYLoc, 0); glUniform1i(m_texULoc, 1); glUniform1i(m_texVLoc, 2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_program->release();
}
