#pragma once

#include <QString>
#include <QFile>
#include <QMutex>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QTextStream>

class LogManager {
public:
    enum LogType {
        App,
        Recorder,
        Player
    };

    static LogManager& instance();

    void init(const QString& logDir = "");
    void write(const QString& msg, LogType type = App);
    QString getLogDir() const;

private:
    LogManager();
    ~LogManager();

    // Prevent copy
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    void checkRotate();
    void createNewLogFile();
    void openExistingOrCreateNew();
    void cleanOldLogs();

    QString m_logDir;
    QFile* m_currentFile;
    QTextStream* m_outStream;
    QMutex m_mutex;
    
    // Limits
    const qint64 MAX_LOG_SIZE = 2 * 1024 * 1024; // 2MB
    const int MAX_FILES = 10;
};
