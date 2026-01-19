#pragma once

#include <QObject>
#include <QList>
#include <QVariantMap>
#include <QDateTime>

struct RecordItem {
    QString id;
    QString filePath;
    QString fileName;
    QDateTime createTime;
    qint64 durationSec;
    bool exists; // 文件是否仍存在
};

class HistoryManager : public QObject {
    Q_OBJECT

public:
    explicit HistoryManager(QObject *parent = nullptr);
    
    void loadHistory();
    void addRecord(const QString &filePath, qint64 durationSec);
    QList<RecordItem> getHistory() const;
    void deleteRecord(const QString &id);
    bool renameRecord(const QString &id, const QString &newName); // Added
    void clearHistory();

signals:
    void historyChanged();

private:
    void saveHistory();
    QString getConfigPath() const;

    QList<RecordItem> m_items;
};
