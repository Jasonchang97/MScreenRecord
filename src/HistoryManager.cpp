#include "HistoryManager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QUuid>
#include <QFileInfo>
#include <QDebug>
#include <algorithm> // For std::sort

HistoryManager::HistoryManager(QObject *parent) : QObject(parent) {
    loadHistory();
}

QString HistoryManager::getConfigPath() const {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) dir.mkpath(".");
    return dir.filePath("history.json");
}

void HistoryManager::loadHistory() {
    m_items.clear();
    QFile file(getConfigPath());
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray array = doc.array();
        
        for (const auto &val : array) {
            QJsonObject obj = val.toObject();
            RecordItem item;
            item.id = obj["id"].toString();
            item.filePath = obj["filePath"].toString();
            item.fileName = QFileInfo(item.filePath).fileName();
            item.createTime = QDateTime::fromString(obj["createTime"].toString(), Qt::ISODate);
            item.durationSec = obj["duration"].toInt();
            item.exists = QFileInfo::exists(item.filePath);
            m_items.append(item);
        }
    }
    // 按时间倒序
    std::sort(m_items.begin(), m_items.end(), [](const RecordItem &a, const RecordItem &b){
        return a.createTime > b.createTime;
    });
    emit historyChanged();
}

void HistoryManager::saveHistory() {
    QJsonArray array;
    for (const auto &item : m_items) {
        QJsonObject obj;
        obj["id"] = item.id;
        obj["filePath"] = item.filePath;
        obj["createTime"] = item.createTime.toString(Qt::ISODate);
        obj["duration"] = item.durationSec;
        array.append(obj);
    }
    
    QFile file(getConfigPath());
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(array);
        file.write(doc.toJson());
    }
}

void HistoryManager::addRecord(const QString &filePath, qint64 durationSec) {
    // Deduplicate by file path
    m_items.erase(std::remove_if(m_items.begin(), m_items.end(), [&](const RecordItem &item){
        return item.filePath == filePath;
    }), m_items.end());

    RecordItem item;
    item.id = QUuid::createUuid().toString();
    item.filePath = filePath;
    item.fileName = QFileInfo(filePath).fileName();
    item.createTime = QDateTime::currentDateTime();
    item.durationSec = durationSec;
    item.exists = true;
    
    m_items.prepend(item);
    saveHistory();
    emit historyChanged();
}

void HistoryManager::deleteRecord(const QString &id) {
    auto it = std::remove_if(m_items.begin(), m_items.end(), [&](const RecordItem &item){
        return item.id == id;
    });
    if (it != m_items.end()) {
        m_items.erase(it, m_items.end());
        saveHistory();
        emit historyChanged();
    }
}

bool HistoryManager::renameRecord(const QString &id, const QString &newName) {
    for (auto &item : m_items) {
        if (item.id == id) {
            QFile file(item.filePath);
            QFileInfo fi(item.filePath);
            QString newPath = fi.dir().filePath(newName);
            
            // Ensure extension exists
            if (!newName.endsWith(".mp4", Qt::CaseInsensitive)) {
                newPath += ".mp4";
            }
            
            if (file.rename(newPath)) {
                item.filePath = newPath;
                item.fileName = QFileInfo(newPath).fileName();
                saveHistory();
                emit historyChanged();
                return true;
            }
            return false;
        }
    }
    return false;
}

void HistoryManager::clearHistory() {
    m_items.clear();
    saveHistory();
    emit historyChanged();
}

QList<RecordItem> HistoryManager::getHistory() const {
    return m_items;
}
