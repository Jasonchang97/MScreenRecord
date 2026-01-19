#include "LogManager.h"
#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>

LogManager& LogManager::instance() {
    static LogManager instance;
    return instance;
}

LogManager::LogManager() : m_currentFile(nullptr), m_outStream(nullptr) {
    // Default to a sane location if init is not called explicitly
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_logDir = dataPath + "/logs";
}

LogManager::~LogManager() {
    if (m_outStream) delete m_outStream;
    if (m_currentFile) {
        if (m_currentFile->isOpen()) m_currentFile->close();
        delete m_currentFile;
    }
}

void LogManager::init(const QString& logDir) {
    QMutexLocker locker(&m_mutex);
    if (!logDir.isEmpty()) {
        m_logDir = logDir;
    }
    
    QDir dir(m_logDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create log directory:" << m_logDir;
            return;
        }
    }

    // Clean up old logs on startup
    cleanOldLogs();
    
    // Try to find the most recent log file that is not full
    openExistingOrCreateNew();
}

QString LogManager::getLogDir() const {
    return m_logDir;
}

void LogManager::write(const QString& msg, LogType type) {
    QMutexLocker locker(&m_mutex);
    
    // Auto-init if not opened (though main.cpp calls init, good to be safe)
    if (!m_currentFile || !m_currentFile->isOpen()) {
        QDir dir(m_logDir);
        if (!dir.exists()) dir.mkpath(".");
        createNewLogFile();
    }

    if (m_outStream) {
        // Prepend tag based on type
        QString prefix;
        switch (type) {
            case App:      prefix = "[APP] "; break;
            case Recorder: prefix = "[REC] "; break;
            case Player:   prefix = "[PLY] "; break;
            default:       prefix = "[UNK] "; break;
        }

        *m_outStream << prefix << msg << endl;
        m_outStream->flush(); // Ensure written to disk
        
        checkRotate();
    }
}

void LogManager::checkRotate() {
    if (!m_currentFile) return;
    
    // Check size limit (20KB)
    if (m_currentFile->size() >= MAX_LOG_SIZE) {
        createNewLogFile();
        cleanOldLogs();
    }
}

void LogManager::createNewLogFile() {
    // Close existing
    if (m_outStream) {
        delete m_outStream;
        m_outStream = nullptr;
    }
    if (m_currentFile) {
        m_currentFile->close();
        delete m_currentFile;
        m_currentFile = nullptr;
    }

    // Create new filename with timestamp: log_yyyyMMdd_HHmmss.txt
    QString timestamp = QDateTime::currentDateTime().toStringEx("yyyyMMdd_HHmmss");
    QString filename = QString("log_%1.log").arg(timestamp);
    QString filePath = m_logDir + "/" + filename;

    m_currentFile = new QFile(filePath);
    if (m_currentFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_outStream = new QTextStream(m_currentFile);
        // Optional: Write header
        *m_outStream << "=== Log Started: " << timestamp << " ===" << endl;
    } else {
        qWarning() << "LogManager: Failed to open log file" << filePath;
    }
}

void LogManager::openExistingOrCreateNew() {
    // Close existing
    if (m_outStream) {
        delete m_outStream;
        m_outStream = nullptr;
    }
    if (m_currentFile) {
        m_currentFile->close();
        delete m_currentFile;
        m_currentFile = nullptr;
    }
    
    // Find the most recent log file that is not full (< MAX_LOG_SIZE)
    QDir dir(m_logDir);
    QStringList filters;
    filters << "log_*.log";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Name | QDir::Reversed); // Newest first (by timestamp in name)
    
    QFileInfoList fileList = dir.entryInfoList();
    
    for (const QFileInfo &fi : fileList) {
        if (fi.size() < MAX_LOG_SIZE) {
            // Found a file that is not full, use it
            m_currentFile = new QFile(fi.absoluteFilePath());
            if (m_currentFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                m_outStream = new QTextStream(m_currentFile);
                // Add separator for new session
                *m_outStream << endl << "=== Session Resumed: " 
                            << QDateTime::currentDateTime().toStringEx("yyyyMMdd_HHmmss") 
                            << " ===" << endl;
                return;
            } else {
                // Failed to open, try next
                delete m_currentFile;
                m_currentFile = nullptr;
            }
        }
    }
    
    // No suitable file found, create new one
    createNewLogFile();
}

void LogManager::cleanOldLogs() {
    QDir dir(m_logDir);
    QStringList filters;
    filters << "log_*.log";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files);
    // Sort by time, newest first (Time | Reversed would be oldest first? No, default is name. Time sorts by modification time.)
    // QDir::Time: "Sort by time (modification time)."
    // QDir::Reversed: "Reverse the sort order." 
    // Usually QDir::Time puts newest first? Let's verify documentation or safe bet is Sort by Name since we use timestamp in name.
    // Timestamp format yyyyMMdd_HHmmss sorts correctly alphabetically.
    dir.setSorting(QDir::Name | QDir::Reversed); // Z-A (Newest first if named by timestamp)
    
    QFileInfoList fileList = dir.entryInfoList();
    
    if (fileList.size() > MAX_FILES) {
        // Remove files from index MAX_FILES onwards (oldest ones)
        for (int i = MAX_FILES; i < fileList.size(); ++i) {
            QFile::remove(fileList[i].absoluteFilePath());
        }
    }
}
