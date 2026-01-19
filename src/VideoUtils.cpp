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
        
        qDebug() << "[VideoUtils] trimVideoMs: input=" << inputFile 
                 << "output=" << outputFile 
                 << "range=" << startMs << "-" << endMs << "ms";
        
        // Open Input
        if ((ret = avformat_open_input(&ifmt_ctx, inputFile.toUtf8().constData(), 0, 0)) < 0) {
             if ((ret = avformat_open_input(&ifmt_ctx, inputFile.toLocal8Bit().constData(), 0, 0)) < 0) {
                 emit processingError("无法打开输入文件");
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

        // Create stream mapping
        int *stream_mapping = new int[ifmt_ctx->nb_streams];
        int stream_index = 0;
        int videoStreamIdx = -1;
        int64_t *start_pts = new int64_t[ifmt_ctx->nb_streams];
        
        for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
            start_pts[i] = AV_NOPTS_VALUE;
            AVStream *in_stream = ifmt_ctx->streams[i];
            AVCodecParameters *in_codecpar = in_stream->codecpar;

            if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
                in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
                in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
                stream_mapping[i] = -1;
                continue;
            }
            
            if (in_codecpar->codec_type == AVMEDIA_TYPE_VIDEO && videoStreamIdx < 0) {
                videoStreamIdx = i;
            }

            stream_mapping[i] = stream_index++;
            AVStream *out_stream = avformat_new_stream(ofmt_ctx, nullptr);
            if (!out_stream) {
                avformat_close_input(&ifmt_ctx);
                avformat_free_context(ofmt_ctx);
                delete[] stream_mapping;
                delete[] start_pts;
                emit processingError("无法创建输出流");
                return;
            }
            avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
            out_stream->codecpar->codec_tag = 0;
            // 复制时间基
            out_stream->time_base = in_stream->time_base;
        }

        if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&ofmt_ctx->pb, outputFile.toUtf8().constData(), AVIO_FLAG_WRITE) < 0) {
                 if (avio_open(&ofmt_ctx->pb, outputFile.toLocal8Bit().constData(), AVIO_FLAG_WRITE) < 0) {
                     avformat_close_input(&ifmt_ctx);
                     avformat_free_context(ofmt_ctx);
                     delete[] stream_mapping;
                     delete[] start_pts;
                     emit processingError("无法打开输出文件");
                     return;
                 }
            }
        }

        // 设置输出格式选项
        AVDictionary *opts = nullptr;
        av_dict_set(&opts, "movflags", "faststart", 0);

        if (avformat_write_header(ofmt_ctx, &opts) < 0) {
             avformat_close_input(&ifmt_ctx);
             if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) avio_closep(&ofmt_ctx->pb);
             avformat_free_context(ofmt_ctx);
             delete[] stream_mapping;
             delete[] start_pts;
             av_dict_free(&opts);
             emit processingError("写入文件头失败");
             return;
        }
        av_dict_free(&opts);

        // Seek to start position (使用视频流的时间基)
        if (videoStreamIdx >= 0) {
            AVStream *video_stream = ifmt_ctx->streams[videoStreamIdx];
            int64_t seek_target = av_rescale_q(startMs, {1, 1000}, video_stream->time_base);
            ret = av_seek_frame(ifmt_ctx, videoStreamIdx, seek_target, AVSEEK_FLAG_BACKWARD);
            qDebug() << "[VideoUtils] Seek to" << startMs << "ms, target pts=" << seek_target << ", ret=" << ret;
        } else {
            // 如果没有视频流，使用AV_TIME_BASE
            int64_t seek_target = startMs * 1000; // to microseconds
            av_seek_frame(ifmt_ctx, -1, seek_target, AVSEEK_FLAG_BACKWARD);
        }

        AVPacket pkt;
        bool videoStarted = false;
        int64_t firstVideoPts = AV_NOPTS_VALUE;
        int packetCount = 0;

        while (1) {
            if (av_read_frame(ifmt_ctx, &pkt) < 0) break;
            
            int in_idx = pkt.stream_index;
            if ((unsigned int)in_idx >= ifmt_ctx->nb_streams || stream_mapping[in_idx] < 0) {
                av_packet_unref(&pkt);
                continue;
            }

            AVStream *in_stream = ifmt_ctx->streams[in_idx];
            
            // 计算当前包的时间（毫秒）
            int64_t pkt_ms = av_rescale_q(pkt.pts, in_stream->time_base, {1, 1000});
            
            // 等待第一个视频关键帧（在开始时间之后）
            if (!videoStarted) {
                if (in_idx == videoStreamIdx) {
                    if (!(pkt.flags & AV_PKT_FLAG_KEY)) {
                        av_packet_unref(&pkt);
                        continue;
                    }
                    if (pkt_ms < startMs) {
                        av_packet_unref(&pkt);
                        continue;
                    }
                    // 找到了第一个有效的视频关键帧
                    videoStarted = true;
                    firstVideoPts = pkt.pts;
                    qDebug() << "[VideoUtils] First video keyframe at" << pkt_ms << "ms, pts=" << firstVideoPts;
                } else {
                    // 在找到视频关键帧之前，跳过音频包
                    av_packet_unref(&pkt);
                    continue;
                }
            }
            
            // 检查结束条件
            if (pkt_ms > endMs) {
                av_packet_unref(&pkt);
                break;
            }
            
            // 跳过开始时间之前的包
            if (pkt_ms < startMs && in_idx != videoStreamIdx) {
                av_packet_unref(&pkt);
                continue;
            }

            // 记录每个流的起始PTS（用于时间戳偏移）
            if (start_pts[in_idx] == AV_NOPTS_VALUE) {
                start_pts[in_idx] = pkt.pts;
            }
            
            // 偏移时间戳，使输出从0开始
            pkt.pts -= start_pts[in_idx];
            pkt.dts -= start_pts[in_idx];
            
            // 确保DTS不大于PTS
            if (pkt.dts > pkt.pts) {
                pkt.dts = pkt.pts;
            }
            
            // 确保非负时间戳
            if (pkt.pts < 0) pkt.pts = 0;
            if (pkt.dts < 0) pkt.dts = 0;

            int out_idx = stream_mapping[in_idx];
            pkt.stream_index = out_idx;
            AVStream *out_stream = ofmt_ctx->streams[out_idx];
            
            // 重新缩放时间戳到输出流的时间基
            av_packet_rescale_ts(&pkt, in_stream->time_base, out_stream->time_base);
            
            ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
            if (ret < 0) {
                qDebug() << "[VideoUtils] Write frame error:" << ret;
                av_packet_unref(&pkt);
                continue; // 继续尝试写入下一帧
            }
            packetCount++;
            av_packet_unref(&pkt);
        }

        qDebug() << "[VideoUtils] Total packets written:" << packetCount;

        av_write_trailer(ofmt_ctx);
        
        avformat_close_input(&ifmt_ctx);
        if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) avio_closep(&ofmt_ctx->pb);
        avformat_free_context(ofmt_ctx);
        
        delete[] stream_mapping;
        delete[] start_pts;
        
        if (packetCount > 0) {
            emit processingFinished(true, outputFile);
        } else {
            emit processingError("剪切失败：未能写入任何数据");
        }
    });
    
    thread->start();
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
}