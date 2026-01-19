#include "VideoUtils.h"
#include <QDebug>
#include <QTime>
#include <QThread>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
}

VideoUtils::VideoUtils(QObject *parent) : QObject(parent) {
    avformat_network_init();
}

void VideoUtils::cropVideo(const QString &inputFile, const QString &outputFile, const QRect &rect) {
    emit processingError("Native cropping not yet implemented");
}

void VideoUtils::trimVideo(const QString &inputFile, const QString &outputFile, const QString &startTime, const QString &endTimeOrDuration) {
    // 1. Parse Times
    QTime start = QTime::fromString(startTime, "HH:mm:ss.z");
    QTime end = QTime::fromString(endTimeOrDuration, "HH:mm:ss.z");
    
    int64_t startMs = 0;
    int64_t endMs = 0;
    
    if (start.isValid()) startMs = QTime(0,0).msecsTo(start);
    
    if (start.isValid() && end.isValid()) {
        int diff = start.msecsTo(end);
        if (diff > 0) endMs = QTime(0,0).msecsTo(end);
        else endMs = startMs + 10000; // Default 10s if invalid
    } else {
        // Fallback or Duration parsing
        // For simplicity, assume endTimeOrDuration is Duration string if < start?
        // Let's just trust valid range for now or default
        if (endMs == 0) endMs = startMs + 10000;
    }
    
    int64_t durationMs = endMs - startMs;

    QThread *thread = QThread::create([=]() {
        AVFormatContext *ifmt_ctx = nullptr;
        AVFormatContext *ofmt_ctx = nullptr;
        int ret = 0;
        
        // Open Input
        if ((ret = avformat_open_input(&ifmt_ctx, inputFile.toUtf8().constData(), 0, 0)) < 0) {
             // Try local 8bit
             if ((ret = avformat_open_input(&ifmt_ctx, inputFile.toLocal8Bit().constData(), 0, 0)) < 0) {
                 emit processingError("Could not open input: " + inputFile);
                 return;
             }
        }
        
        if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
             avformat_close_input(&ifmt_ctx);
             emit processingError("Failed to retrieve input info");
             return;
        }

        // Open Output
        avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, outputFile.toUtf8().constData());
        if (!ofmt_ctx) {
             avformat_close_input(&ifmt_ctx);
             emit processingError("Could not create output context");
             return;
        }

        // Streams
        int *stream_mapping = new int[ifmt_ctx->nb_streams];
        int stream_index = 0;
        int64_t *dts_start_from = new int64_t[ifmt_ctx->nb_streams];
        memset(dts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
        
        for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
            AVStream *in_stream = ifmt_ctx->streams[i];
            AVCodecParameters *in_codecpar = in_stream->codecpar;

            if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
                in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
                in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
                stream_mapping[i] = -1;
                continue;
            }

            stream_mapping[i] = stream_index++;
            AVStream *out_stream = avformat_new_stream(ofmt_ctx, nullptr);
            avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
            out_stream->codecpar->codec_tag = 0;
        }

        if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&ofmt_ctx->pb, outputFile.toUtf8().constData(), AVIO_FLAG_WRITE) < 0) {
                 avformat_close_input(&ifmt_ctx);
                 avformat_free_context(ofmt_ctx);
                 delete[] stream_mapping;
                 delete[] dts_start_from;
                 emit processingError("Could not open output file");
                 return;
            }
        }

        if (avformat_write_header(ofmt_ctx, nullptr) < 0) {
             // Cleanup...
             emit processingError("Error writing header");
             return;
        }

        // Seek
        int64_t seekTarget = startMs * 1000; // microseconds
        av_seek_frame(ifmt_ctx, -1, seekTarget, AVSEEK_FLAG_BACKWARD);

        AVPacket pkt;
        int64_t global_start_us = AV_NOPTS_VALUE;

        while (1) {
            if (av_read_frame(ifmt_ctx, &pkt) < 0) break;
            
            AVStream *in_stream = ifmt_ctx->streams[pkt.stream_index];
            if ((unsigned int)pkt.stream_index >= ifmt_ctx->nb_streams || stream_mapping[pkt.stream_index] < 0) {
                av_packet_unref(&pkt);
                continue;
            }

            // Check Time
            AVRational tb_q; 
            tb_q.num = 1; tb_q.den = AV_TIME_BASE;
            int64_t pts_us = av_rescale_q(pkt.pts, in_stream->time_base, tb_q);
            
            // Set Global Start Time to REQUESTED Start Time
            // This forces packets before start time to have negative timestamps (Pre-roll)
            if (global_start_us == AV_NOPTS_VALUE) {
                global_start_us = startMs * 1000;
            }

            // Stop condition
            if (pts_us > endMs * 1000) {
                av_packet_unref(&pkt);
                break;
            }

            // Shift Timestamps relative to Global Start
            int64_t stream_start_pts = av_rescale_q(global_start_us, tb_q, in_stream->time_base);
            
            pkt.pts -= stream_start_pts;
            pkt.dts -= stream_start_pts;
            // Allow negative timestamps (Pre-roll)
            // if (pkt.pts < 0) pkt.pts = 0; 
            // if (pkt.dts < 0) pkt.dts = 0;

            pkt.stream_index = stream_mapping[pkt.stream_index];
            AVStream *out_stream = ofmt_ctx->streams[pkt.stream_index];
            
            av_packet_rescale_ts(&pkt, in_stream->time_base, out_stream->time_base);
            
            if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
                 av_packet_unref(&pkt);
                 break;
            }
            av_packet_unref(&pkt);
        }

        av_write_trailer(ofmt_ctx);
        
        avformat_close_input(&ifmt_ctx);
        if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) avio_closep(&ofmt_ctx->pb);
        avformat_free_context(ofmt_ctx);
        
        delete[] stream_mapping;
        delete[] dts_start_from;
        
        emit processingFinished(true, outputFile);
    });
    
    thread->start();
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
}

void VideoUtils::trimVideoMs(const QString &inputFile, const QString &outputFile, qint64 startMs, qint64 endMs) {
    QThread *thread = QThread::create([=]() {
        AVFormatContext *ifmt_ctx = nullptr;
        AVFormatContext *ofmt_ctx = nullptr;
        int ret = 0;
        
        // Open Input
        if ((ret = avformat_open_input(&ifmt_ctx, inputFile.toUtf8().constData(), 0, 0)) < 0) {
             // Try local 8bit
             if ((ret = avformat_open_input(&ifmt_ctx, inputFile.toLocal8Bit().constData(), 0, 0)) < 0) {
                 emit processingError("无法打开输入文件: " + inputFile);
                 return;
             }
        }
        
        if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
             avformat_close_input(&ifmt_ctx);
             emit processingError("无法获取输入文件信息");
             return;
        }

        // Open Output
        avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, outputFile.toUtf8().constData());
        if (!ofmt_ctx) {
             avformat_close_input(&ifmt_ctx);
             emit processingError("无法创建输出上下文");
             return;
        }

        // Streams
        int *stream_mapping = new int[ifmt_ctx->nb_streams];
        int stream_index = 0;
        
        for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
            AVStream *in_stream = ifmt_ctx->streams[i];
            AVCodecParameters *in_codecpar = in_stream->codecpar;

            if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
                in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
                in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
                stream_mapping[i] = -1;
                continue;
            }

            stream_mapping[i] = stream_index++;
            AVStream *out_stream = avformat_new_stream(ofmt_ctx, nullptr);
            avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
            out_stream->codecpar->codec_tag = 0;
        }

        if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&ofmt_ctx->pb, outputFile.toUtf8().constData(), AVIO_FLAG_WRITE) < 0) {
                 if (avio_open(&ofmt_ctx->pb, outputFile.toLocal8Bit().constData(), AVIO_FLAG_WRITE) < 0) {
                     avformat_close_input(&ifmt_ctx);
                     avformat_free_context(ofmt_ctx);
                     delete[] stream_mapping;
                     emit processingError("无法打开输出文件");
                     return;
                 }
            }
        }

        if (avformat_write_header(ofmt_ctx, nullptr) < 0) {
             avformat_close_input(&ifmt_ctx);
             if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) avio_closep(&ofmt_ctx->pb);
             avformat_free_context(ofmt_ctx);
             delete[] stream_mapping;
             emit processingError("写入文件头失败");
             return;
        }

        // Seek to start position
        int64_t seekTarget = startMs * 1000; // to microseconds
        av_seek_frame(ifmt_ctx, -1, seekTarget, AVSEEK_FLAG_BACKWARD);

        AVPacket pkt;
        int64_t global_start_us = startMs * 1000;
        int64_t end_us = endMs * 1000;
        bool hasVideoKeyframe = false;
        int videoStreamIdx = -1;
        
        // Find video stream index
        for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
            if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStreamIdx = i;
                break;
            }
        }

        while (1) {
            if (av_read_frame(ifmt_ctx, &pkt) < 0) break;
            
            AVStream *in_stream = ifmt_ctx->streams[pkt.stream_index];
            if ((unsigned int)pkt.stream_index >= ifmt_ctx->nb_streams || stream_mapping[pkt.stream_index] < 0) {
                av_packet_unref(&pkt);
                continue;
            }

            // Calculate PTS in microseconds
            AVRational tb_q = {1, AV_TIME_BASE};
            int64_t pts_us = av_rescale_q(pkt.pts, in_stream->time_base, tb_q);
            
            // Wait for first video keyframe after seek
            if (!hasVideoKeyframe && pkt.stream_index == videoStreamIdx) {
                if (!(pkt.flags & AV_PKT_FLAG_KEY) || pts_us < global_start_us) {
                    av_packet_unref(&pkt);
                    continue;
                }
                hasVideoKeyframe = true;
                global_start_us = pts_us; // Align to actual keyframe
            }
            
            // Skip packets before actual start
            if (pts_us < global_start_us && pkt.stream_index != videoStreamIdx) {
                av_packet_unref(&pkt);
                continue;
            }

            // Stop condition
            if (pts_us > end_us) {
                av_packet_unref(&pkt);
                break;
            }

            // Shift Timestamps relative to Global Start
            int64_t stream_start_pts = av_rescale_q(global_start_us, tb_q, in_stream->time_base);
            
            pkt.pts -= stream_start_pts;
            pkt.dts -= stream_start_pts;
            
            // Ensure non-negative timestamps
            if (pkt.pts < 0) pkt.pts = 0;
            if (pkt.dts < 0) pkt.dts = 0;

            pkt.stream_index = stream_mapping[pkt.stream_index];
            AVStream *out_stream = ofmt_ctx->streams[pkt.stream_index];
            
            av_packet_rescale_ts(&pkt, in_stream->time_base, out_stream->time_base);
            
            if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
                 av_packet_unref(&pkt);
                 break;
            }
            av_packet_unref(&pkt);
        }

        av_write_trailer(ofmt_ctx);
        
        avformat_close_input(&ifmt_ctx);
        if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) avio_closep(&ofmt_ctx->pb);
        avformat_free_context(ofmt_ctx);
        
        delete[] stream_mapping;
        
        emit processingFinished(true, outputFile);
    });
    
    thread->start();
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
}