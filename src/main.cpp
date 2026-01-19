#include "MainWindow.h"
#include "AppVersion.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMessageBox>
#include <QStandardPaths>
#include <QLocalServer>
#include <QLocalSocket>
#include "LogManager.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// 自定义消息处理函数
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString typeStr;
    switch (type) {
    case QtDebugMsg:    typeStr = "Debug"; break;
    case QtWarningMsg:  typeStr = "Warning"; break;
    case QtCriticalMsg: typeStr = "Critical"; break;
    case QtFatalMsg:    typeStr = "Fatal"; break;
    case QtInfoMsg:     typeStr = "Info"; break;
    }

    QString formattedMsg = QString("[%1] %2: %3").arg(QDateTime::currentDateTime().toStringEx("yyyy-MM-dd hh:mm:ss.zzz")).arg(typeStr).arg(msg);

    LogManager::instance().write(formattedMsg, LogManager::App);
    
    // 关键：如果是致命错误，弹窗提示！
    if (type == QtFatalMsg || msg.contains("Failed to load platform plugin")) {
        #ifdef Q_OS_WIN
        MessageBoxA(0, msg.toLocal8Bit().constData(), "MScreenRecord Critical Error", 0x10 | 0x10000); // MB_ICONHAND | MB_SETFOREGROUND
        #endif
    }
}

int main(int argc, char *argv[])
{
    // 1. 设置应用信息
    QCoreApplication::setOrganizationName("KSO");
    QCoreApplication::setApplicationName("MScreenRecord");

    // 2. 设置 High DPI + 软件 OpenGL（避免缺少显卡驱动时崩溃）
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);

    try {
        // 3. 先构造 QApplication，再安装日志处理器，避免 applicationDirPath 警告
        QApplication a(argc, argv);
        
        // 设置全局字体为微软雅黑，指定大小防止过大
        QFont font("Microsoft YaHei");
        font.setPointSize(9); // 显式设置为 9pt
        font.setStyleStrategy(QFont::PreferAntialias);
        a.setFont(font);

        // Initialize LogManager (default location: AppData/Roaming/KSO/MScreenRecord/logs)
        LogManager::instance().init();
        
        qInstallMessageHandler(myMessageOutput);
        qInfo() << "=== Application Starting ===";
        qInfo() << "Version: " << APP_VERSION_STR;
        qInfo() << "App Path:" << QCoreApplication::applicationDirPath();
        qInfo() << "Log Path:" << LogManager::instance().getLogDir();

        // 4. 添加插件路径
        QDir dir(QCoreApplication::applicationDirPath());
        if (dir.cd("plugins")) {
            QCoreApplication::addLibraryPath(dir.absolutePath());
            qInfo() << "Added library path:" << dir.absolutePath();
        } else {
            qInfo() << "Plugins dir not found in app dir.";
        }
        // QT_DEBUG_PLUGINS 太冗余，先关闭。若需调试，再手动改为"1"
        qputenv("QT_DEBUG_PLUGINS", "0");
        
        // --- Single Instance Logic (QLocalServer) ---
        const QString serverName = "MScreenRecord_Server";
        QLocalSocket socket;
        socket.connectToServer(serverName);
        if (socket.waitForConnected(500)) {
            // Client Mode: Wait for Server to send HWND
            if (socket.waitForReadyRead(1000)) {
                QByteArray data = socket.readAll();
                qulonglong winId = data.toULongLong();
                if (winId) {
                    #ifdef Q_OS_WIN
                    HWND hwnd = (HWND)winId;
                    if (IsIconic(hwnd)) ShowWindow(hwnd, SW_RESTORE);
                    SetForegroundWindow(hwnd);
                    #endif
                }
            }
            return 0;
        }
        
        // We are the server
        QLocalServer::removeServer(serverName);
        QLocalServer server;
        server.listen(serverName);
        // --------------------------------------------
        
        qInfo() << "Initializing MainWindow...";
        
        MainWindow w;
        qInfo() << "MainWindow constructed.";

        // Connect activation signal
        QObject::connect(&server, &QLocalServer::newConnection, [&](){
            QLocalSocket *client = server.nextPendingConnection();
            if (!client) return;
            
            w.showNormal();
            w.activateWindow();
            w.raise();
            
            // Send HWND to client so it can force us to foreground
            QTextStream stream(client);
            stream << (qulonglong)w.winId();
            stream.flush();
            client->waitForBytesWritten(500);
            
            // Also try locally (best effort)
            #ifdef Q_OS_WIN
            HWND hwnd = (HWND)w.winId();
            if (IsIconic(hwnd)) ShowWindow(hwnd, SW_RESTORE);
            SetForegroundWindow(hwnd);
            #endif
            
            client->close();
            client->deleteLater();
        });
        
        w.show();
        
        qInfo() << "MainWindow shown.";
        return a.exec();
    } catch (const std::exception &e) {
        qCritical() << "Standard Exception:" << e.what();
        #ifdef Q_OS_WIN
        MessageBoxA(0, e.what(), "Startup Exception", 0x10);
        #endif
        return -1;
    } catch (...) {
        qCritical() << "Unknown Exception occurred.";
        #ifdef Q_OS_WIN
        MessageBoxA(0, "Unknown Error occurred during startup.", "Startup Error", 0x10);
        #endif
        return -1;
    }
}
