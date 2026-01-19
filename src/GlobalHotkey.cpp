#include "GlobalHotkey.h"
#include <QCoreApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

GlobalHotkey* GlobalHotkey::s_instance = nullptr;

GlobalHotkey* GlobalHotkey::instance() {
    if (!s_instance) {
        s_instance = new GlobalHotkey(qApp);
    }
    return s_instance;
}

GlobalHotkey::GlobalHotkey(QObject *parent) : QObject(parent) {
#ifdef Q_OS_WIN
    m_hwnd = nullptr;
#endif
    qApp->installNativeEventFilter(this);
}

GlobalHotkey::~GlobalHotkey() {
    unregisterAll();
    qApp->removeNativeEventFilter(this);
}

bool GlobalHotkey::registerHotkey(HotkeyId id, quint32 modifiers, quint32 virtualKey) {
    if (modifiers == 0 && virtualKey == 0) return false;
    
#ifdef Q_OS_WIN
    // 先注销已存在的同ID快捷键
    UnregisterHotKey(nullptr, id);
    
    // 注册新快捷键
    if (RegisterHotKey(nullptr, id, modifiers, virtualKey)) {
        return true;
    }
    return false;
#else
    return false;
#endif
}

void GlobalHotkey::unregisterHotkey(HotkeyId id) {
#ifdef Q_OS_WIN
    UnregisterHotKey(nullptr, id);
#endif
}

bool GlobalHotkey::isHotkeyAvailable(quint32 modifiers, quint32 virtualKey) {
    if (modifiers == 0 && virtualKey == 0) return true;
    
#ifdef Q_OS_WIN
    // 尝试注册一个临时快捷键来检测是否可用
    int tempId = 9999;
    if (RegisterHotKey(nullptr, tempId, modifiers, virtualKey)) {
        UnregisterHotKey(nullptr, tempId);
        return true;
    }
    return false;
#else
    return true;
#endif
}

bool GlobalHotkey::forceRegisterHotkey(HotkeyId id, quint32 modifiers, quint32 virtualKey) {
    unregisterHotkey(id);
    return registerHotkey(id, modifiers, virtualKey);
}

void GlobalHotkey::unregisterAll() {
#ifdef Q_OS_WIN
    UnregisterHotKey(nullptr, ShowMainWindow);
    UnregisterHotKey(nullptr, StartStopRecording);
#endif
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool GlobalHotkey::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
#else
bool GlobalHotkey::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
#endif
{
    Q_UNUSED(result);
    
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        MSG *msg = static_cast<MSG*>(message);
        if (msg->message == WM_HOTKEY) {
            int id = static_cast<int>(msg->wParam);
            emit hotkeyTriggered(id);
            return true;
        }
    }
#endif
    
    return false;
}
