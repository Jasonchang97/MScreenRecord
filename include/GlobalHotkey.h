#pragma once

#include <QObject>
#include <QKeySequence>
#include <QAbstractNativeEventFilter>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class GlobalHotkey : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT

public:
    enum HotkeyId {
        ShowMainWindow = 1,
        StartStopRecording = 2
    };
    
    static GlobalHotkey* instance();
    
    // 注册快捷键，返回是否成功
    bool registerHotkey(HotkeyId id, quint32 modifiers, quint32 virtualKey);
    
    // 注销快捷键
    void unregisterHotkey(HotkeyId id);
    
    // 检测快捷键是否被其他应用占用
    bool isHotkeyAvailable(quint32 modifiers, quint32 virtualKey);
    
    // 尝试强制注册（先注销再注册）
    bool forceRegisterHotkey(HotkeyId id, quint32 modifiers, quint32 virtualKey);
    
    // 注销所有快捷键
    void unregisterAll();
    
    // QAbstractNativeEventFilter
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
#endif

signals:
    void hotkeyTriggered(int id);

private:
    explicit GlobalHotkey(QObject *parent = nullptr);
    ~GlobalHotkey();
    
    static GlobalHotkey *s_instance;
    
#ifdef Q_OS_WIN
    HWND m_hwnd;
#endif
};
