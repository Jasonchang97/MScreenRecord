#pragma once

#include <QString>
#include <QRect>
#include <QObject>

class VideoUtils : public QObject {
    Q_OBJECT

public:
    explicit VideoUtils(QObject *parent = nullptr);

    // Crop (TODO: Native Implementation)
    void cropVideo(const QString &inputFile, const QString &outputFile, const QRect &rect);

    // Trim (Native Implementation) - String time format
    void trimVideo(const QString &inputFile, const QString &outputFile, const QString &startTime, const QString &endTimeOrDuration);
    
    // Trim (Native Implementation) - Milliseconds
    void trimVideoMs(const QString &inputFile, const QString &outputFile, qint64 startMs, qint64 endMs);

signals:
    void processingFinished(bool success, const QString &outputFile);
    void processingError(const QString &error);
};
