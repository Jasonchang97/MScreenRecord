#include "MainWindow.h"
#include "SettingsDialog.h"
#include "ThemePatternWidget.h"
#include "CustomMessageBox.h" 
#include "LogManager.h"
#include "HotkeyEdit.h"
#include "GlobalHotkey.h"
#include "ToastTip.h"
#include <QApplication>
#include <windows.h>
#include <windowsx.h> 
#include <QGroupBox>
#include <QTimer>
#include <QMessageBox>
#include <QInputDialog> 
#include <QStyle>
#include <QTime>
#include <QFileDialog>
#include <QAction>
#include <QAudioDeviceInfo>
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QPainter>
#include <QLinearGradient>
#include <QGraphicsDropShadowEffect>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QBitmap>
#include <QDir>
#include <QCoreApplication>
#include <QProcess>
#include <QResizeEvent>
#include <QtMath>
#include <QScreen>
#include <QScrollBar>
#include "AppVersion.h"

extern "C" {
#include <libavformat/avformat.h>
}

// Helper for video duration
static qint64 getVideoDuration(const QString &path) {
    AVFormatContext *fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, path.toStdString().c_str(), NULL, NULL) < 0) return 0;
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) { avformat_close_input(&fmt_ctx); return 0; }
    qint64 duration = fmt_ctx->duration / 1000; // Duration in milliseconds
    avformat_close_input(&fmt_ctx);
    return duration;
}

// Helper for coloring icons
static QIcon createIcon(const QIcon &source, const QColor &color) {
    if (source.isNull()) return QIcon();
    QPixmap pix = source.pixmap(32, 32);
    if (pix.isNull()) return QIcon();
    QImage img = pix.toImage();
    QPainter p(&img);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(img.rect(), color);
    p.end();
    return QIcon(QPixmap::fromImage(img));
}

// Helper to draw a gear icon
static QIcon drawGearIcon(const QColor &color) {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(color);

    int cx = 16, cy = 16;
    int outerR = 14;
    int innerR = 10;
    int holeR = 5;
    int teeth = 8;
    
    // Draw teeth
    for (int i = 0; i < teeth; ++i) {
        p.save();
        p.translate(cx, cy);
        p.rotate(i * (360.0 / teeth));
        p.drawRect(-3, -outerR, 6, 6);
        p.restore();
    }
    
    // Draw body
    p.drawEllipse(QPoint(cx, cy), innerR + 2, innerR + 2);
    
    // Draw hole
    p.setCompositionMode(QPainter::CompositionMode_Clear);
    p.setBrush(Qt::transparent);
    p.drawEllipse(QPoint(cx, cy), holeR, holeR);
    
    return QIcon(pix);
}

// History Item Widget
class HistoryItemWidget : public QWidget {
public:
    HistoryItemWidget(const QString &id, const QString &path, const QString &duration, const QString &time, QColor iconColor, QColor textColor, MainWindow *mainWin)
        : QWidget(mainWin), m_id(id), m_path(path)
    {
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(10, 5, 10, 5);
        layout->setSpacing(10);
        
        // Icon
        QLabel *iconLabel = new QLabel(this);
        QPixmap pix = mainWin->style()->standardIcon(QStyle::SP_FileIcon).pixmap(32, 32);
        QImage img = pix.toImage();
        QPainter p(&img);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(img.rect(), iconColor);
        p.end();
        iconLabel->setPixmap(QPixmap::fromImage(img));
        layout->addWidget(iconLabel);

        // Info
        QVBoxLayout *infoLayout = new QVBoxLayout();
        infoLayout->setSpacing(2);
        infoLayout->setContentsMargins(0, 0, 0, 0);
        
        QLabel *nameLabel = new QLabel(QFileInfo(path).fileName(), this);
        // Explicitly set text color from theme
        nameLabel->setStyleSheet(QString("font-weight: bold; font-size: 11px; color: %1;").arg(textColor.name()));
        
        QLabel *detailLabel = new QLabel(QString("%1 • %2").arg(time).arg(duration), this);
        // Use semi-transparent color for detail
        QColor detailColor = textColor;
        detailColor.setAlpha(150);
        detailLabel->setStyleSheet(QString("font-size: 9px; color: %1;").arg(detailColor.name(QColor::HexArgb))); 
        
        infoLayout->addWidget(nameLabel);
        infoLayout->addWidget(detailLabel);
        layout->addLayout(infoLayout, 1);

        // Play Button
        QPushButton *btnPlay = new QPushButton(this);
        QPixmap playPix = mainWin->style()->standardIcon(QStyle::SP_MediaPlay).pixmap(20, 20);
        QImage playImg = playPix.toImage();
        QPainter pp(&playImg);
        pp.setCompositionMode(QPainter::CompositionMode_SourceIn);
        pp.fillRect(playImg.rect(), iconColor);
        pp.end();
        
        btnPlay->setIcon(QIcon(QPixmap::fromImage(playImg)));
        btnPlay->setFixedSize(30, 30);
        btnPlay->setCursor(Qt::PointingHandCursor);
        btnPlay->setFlat(true);
        // Adapted hover style
        QString hoverColor = (textColor.value() > 128) ? "rgba(255,255,255,0.1)" : "rgba(0,0,0,0.05)";
        btnPlay->setStyleSheet(QString("QPushButton { background: transparent; border: 1px solid transparent; border-radius: 4px; } QPushButton:hover { background: %1; border-color: %2; }").arg(hoverColor).arg(iconColor.name()));
        
        mainWin->connect(btnPlay, &QPushButton::clicked, mainWin, [mainWin, path](){
            mainWin->loadVideo(path);
        });
        
        layout->addWidget(btnPlay);
        
        setAttribute(Qt::WA_TransparentForMouseEvents, false); 
        setAutoFillBackground(false); 
        setStyleSheet("background: transparent;");
    }
    
    QString id() const { return m_id; }

private:
    QString m_id;
    QString m_path;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_recorder(new RecorderController(this))
    , m_historyMgr(new HistoryManager(this))
    , m_videoUtils(new VideoUtils(this))
    , m_isDragging(false)
    , m_isSeeking(false)
    , m_pendingRecording(false)
    , m_currentDuration(0)
    , m_currentPosition(0)
    , m_totalDuration(0)
    , m_isPlaying(false)
    , m_settings(new QSettings("KSO", "MScreenRecord", this))
    , m_titleBar(nullptr)
    , m_btnStartStop(nullptr)
    , m_btnOpenFile(nullptr)
    , m_btnSettings(nullptr)
    , m_lblAreaStatus(nullptr)
    , m_chkSysAudio(nullptr)
    , m_sliderSysVol(nullptr)
    , m_lblSysVolValue(nullptr)
    , m_chkMicAudio(nullptr)
    , m_sliderMicVol(nullptr)
    , m_lblMicVolValue(nullptr)
    , m_videoContainer(nullptr)
    , m_videoLayout(nullptr)
    , m_player(nullptr)
    , m_previewLabel(nullptr)
    , m_rangeSlider(nullptr)
    , m_btnPlayPause(nullptr)
    , m_lblTimeDisplay(nullptr)
    , m_lblTrimInfo(nullptr)
    , m_editTrimStart(nullptr)
    , m_editTrimEnd(nullptr)
    , m_btnTrim(nullptr)
    , m_listHistory(nullptr)
    , m_btnDeleteHistory(nullptr)
    , m_btnMin(nullptr)
    , m_btnMax(nullptr)
    , m_btnClose(nullptr)
    , m_logConsole(nullptr)
    , m_themeOverlay(nullptr)
    , m_overlay(nullptr)
    , m_countdownOverlay(nullptr)
    , m_previewTimer(nullptr)
    , m_recTimer(nullptr)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
    , m_floatingBall(nullptr)
{
    setupUi();
    setupStyle();
    setupTrayIcon();
    setupFloatingBall();
    registerHotkeys();

    // Timers
    m_previewTimer = new QTimer(this);
    m_previewTimer->setInterval(100);

    m_recTimer = new QTimer(this);
    m_recTimer->setInterval(100); // Increased frequency to 100ms for smoother updates
    connect(m_recTimer, &QTimer::timeout, [this]() {
        if (m_recorder->state() == RecorderController::Recording) {
            m_currentDuration = m_recorder->getDuration();
            // Update overlay toolbar with duration
            if (m_overlay && m_overlay->isVisible()) {
                int seconds = (m_currentDuration / 1000) % 60;
                int minutes = (m_currentDuration / 60000) % 60;
                m_overlay->updateDuration(QString("%1:%2")
                    .arg(minutes, 2, 10, QChar('0'))
                    .arg(seconds, 2, 10, QChar('0')));
            }
        }
    });

    // Connections
    connect(m_recorder, &RecorderController::stateChanged, this, &MainWindow::onRecorderStateChanged);
    connect(m_recorder, &RecorderController::errorOccurred, this, &MainWindow::logMessage);
    
    connect(m_recorder, &RecorderController::recordingFinished, [this](const QString &path){
        saveAndAddToHistory(path);
    });

    connect(m_recorder, &RecorderController::audioLevelsCalculated, this, &MainWindow::updateAudioLevels);

    // Initial Load
    refreshHistoryList();
    loadSettings();
    
    // Startup Logs
    logMessage("Application started. Version: " + QString(APP_VERSION_STR));
    logMessage("Initializing environment...");
    logMessage("FFmpeg libraries loaded.");
    if (QAudioDeviceInfo::availableDevices(QAudio::AudioInput).isEmpty()) {
        logMessage("Warning: No audio input devices found.");
    } else {
        logMessage("Audio devices detected.");
    }

    QStringList args = QCoreApplication::arguments();
    if (args.contains("--minimized")) {
        showMinimized();
    }
}

MainWindow::~MainWindow() {
    if (m_recorder && m_recorder->state() == RecorderController::Recording) {
        m_recorder->stopRecording();
    }
    if (m_floatingBall) delete m_floatingBall;
}

void MainWindow::setupUi() {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    
    QWidget *shadowWrapper = new QWidget(this);
    setCentralWidget(shadowWrapper);
    QVBoxLayout *wrapperLayout = new QVBoxLayout(shadowWrapper);
    wrapperLayout->setContentsMargins(10, 10, 10, 10);
    
    QWidget *mainContainer = new QWidget(shadowWrapper);
    mainContainer->setObjectName("MainContainer"); 
    mainContainer->setAttribute(Qt::WA_StyledBackground, true);
    mainContainer->setAutoFillBackground(true); 
    mainContainer->setMinimumWidth(1100); 
    mainContainer->setMinimumHeight(780); 
    
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(0, 0, 0, 100));
    mainContainer->setGraphicsEffect(shadow);
    
    m_themeOverlay = new ThemePatternWidget(mainContainer);
    m_themeOverlay->lower(); 
    
    wrapperLayout->addWidget(mainContainer);

    QVBoxLayout *mainLayout = new QVBoxLayout(mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0); 
    mainLayout->setSpacing(0);

    // === 1. Title Bar ===
    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("TitleBar"); 
    m_titleBar->setFixedHeight(50);
    QHBoxLayout *titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(20, 0, 10, 0); 
    titleLayout->setSpacing(10); 
    
    QLabel *lblIcon = new QLabel(this);
    QString iconPath = QCoreApplication::applicationDirPath() + "/resources/app.svg";
    if (QFile::exists(iconPath)) {
        lblIcon->setPixmap(QIcon(iconPath).pixmap(24, 24));
    } else {
        lblIcon->setPixmap(style()->standardIcon(QStyle::SP_DesktopIcon).pixmap(24, 24));
    }
    lblIcon->setObjectName("TitleIcon");
    
    QVBoxLayout *titleTextLayout = new QVBoxLayout();
    titleTextLayout->setSpacing(0);
    titleTextLayout->setAlignment(Qt::AlignVCenter);
    QLabel *lblTitle = new QLabel("MScreenRecord", this);
    lblTitle->setObjectName("TitleLabel");
    lblTitle->setStyleSheet("font-weight: bold; font-size: 14px;");
    
    QLabel *lblSubTitle = new QLabel("Author: zhangjisheng", this);
    lblSubTitle->setObjectName("SubTitleLabel");
    lblSubTitle->setStyleSheet("font-size: 10px; color: #888;");
    
    titleTextLayout->addWidget(lblTitle);
    titleTextLayout->addWidget(lblSubTitle);
    
    titleLayout->addWidget(lblIcon);
    titleLayout->addLayout(titleTextLayout);
    titleLayout->addStretch();

    m_btnSettings = new QPushButton(this);
    m_btnSettings->setFixedSize(30, 30);
    m_btnSettings->setFlat(true);
    m_btnSettings->setCursor(Qt::PointingHandCursor);
    m_btnSettings->setObjectName("BtnWindow"); 
    connect(m_btnSettings, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    titleLayout->addWidget(m_btnSettings);

    m_btnMin = new QPushButton(this); m_btnMin->setObjectName("BtnWindow");
    m_btnMax = new QPushButton(this); m_btnMax->setObjectName("BtnWindow");
    m_btnClose = new QPushButton(this); m_btnClose->setObjectName("BtnWindow");
    m_btnMin->setFixedSize(30, 30); m_btnMax->setFixedSize(30, 30); m_btnClose->setFixedSize(30, 30);
    
    connect(m_btnMin, &QPushButton::clicked, this, &MainWindow::onMinimizeClicked);
    connect(m_btnMax, &QPushButton::clicked, this, &MainWindow::onMaximizeClicked);
    connect(m_btnClose, &QPushButton::clicked, this, &MainWindow::onCloseClicked);
    
    titleLayout->addWidget(m_btnMin);
    titleLayout->addWidget(m_btnMax);
    titleLayout->addWidget(m_btnClose);

    mainLayout->addWidget(m_titleBar);

    // === 2. Content Splitter ===
    QSplitter *splitter = new QSplitter(Qt::Horizontal, mainContainer);
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false);
    
    // === 2.1 Left Module (Video Preview) ===
    QWidget *leftPanel = new QWidget(splitter);
    leftPanel->setObjectName("LeftPanel");
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(20, 20, 20, 5); // Reduced bottom margin
    leftLayout->setSpacing(15);
    
    QGroupBox *grpPreview = new QGroupBox("视频预览", leftPanel);
    QVBoxLayout *previewLayout = new QVBoxLayout(grpPreview);
    previewLayout->setContentsMargins(15, 25, 15, 15);
    previewLayout->setSpacing(10);

    // Video Area
    m_videoContainer = new VideoContainer(grpPreview);
    m_videoContainer->setMinimumSize(720, 405); // 16:9 aspect ratio, roughly 720x405
    m_videoLayout = new QVBoxLayout(m_videoContainer);
    m_videoLayout->setContentsMargins(0,0,0,0);
    
    m_player = new NativePlayerWidget(m_videoContainer);
    m_player->hide();
    connect(m_player, &NativePlayerWidget::playbackFinished, this, &MainWindow::onPlayerFinished);
    connect(m_player, &NativePlayerWidget::positionChanged, this, &MainWindow::onPlayerPositionChanged);
    connect(m_player, &NativePlayerWidget::previewImageReady, [this](const QImage &img){
        if (m_previewLabel && !img.isNull()) {
             m_previewLabel->setPixmap(QPixmap::fromImage(img).scaled(m_previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    });
    
    m_previewLabel = new QLabel(m_videoContainer);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setStyleSheet("background-color: #000; color: #555; font-size: 14px;");
    m_previewLabel->setText("暂无视频\n请录制或打开视频文件");
    
    m_videoLayout->addWidget(m_player);
    m_videoLayout->addWidget(m_previewLabel);
    previewLayout->addWidget(m_videoContainer);
    
    // Control Bar
    QHBoxLayout *ctrlBarLayout = new QHBoxLayout();
    ctrlBarLayout->setSpacing(10);

    m_btnOpenFile = new QPushButton(" 打开视频预览", grpPreview);
    m_btnOpenFile->setObjectName("BtnStandard");
    // Icon will be set in setupStyle for theming
    m_btnOpenFile->setFixedHeight(32);
    m_btnOpenFile->setCursor(Qt::PointingHandCursor);
    connect(m_btnOpenFile, &QPushButton::clicked, this, &MainWindow::onOpenFileClicked);

    m_btnPlayPause = new QPushButton(grpPreview);
    m_btnPlayPause->setObjectName("BtnMedia");
    m_btnPlayPause->setFixedSize(32, 32);
    m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_btnPlayPause->setCursor(Qt::PointingHandCursor);
    connect(m_btnPlayPause, &QPushButton::clicked, this, &MainWindow::onPlayPauseClicked);

    m_lblTimeDisplay = new QLabel("00:00.000 / 00:00.000", grpPreview);
    m_lblTimeDisplay->setStyleSheet("font-family: 'Consolas', 'Monospace'; font-size: 12px; font-weight: bold;");
    
    // RangeSlider
    m_rangeSlider = new RangeSlider(grpPreview);
    m_rangeSlider->setRange(0, 1000);
    m_rangeSlider->setValues(0, 1000);
    connect(m_rangeSlider, &RangeSlider::valuesChanged, [this](int lower, int upper){
        updateTimeLabel(m_currentPosition, m_totalDuration);
    });
    connect(m_rangeSlider, &RangeSlider::valuePreview, [this](int val){
        if (m_totalDuration > 0 && !m_lastRecordedFile.isEmpty()) {
             qint64 targetMs = val * m_totalDuration / 1000;
             
             // If playing, stop first.
             if (m_isPlaying) {
                 m_player->stopPlay();
                 m_isPlaying = false;
                 m_btnPlayPause->setIcon(createIcon(style()->standardIcon(QStyle::SP_MediaPlay), getThemeIconColor()));
                 m_rangeSlider->setPlaybackValue(-1);
             }
             
             // 确保播放器可见
             if (!m_player->isVisible()) {
                 m_player->show();
                 m_previewLabel->hide();
             }
             
             // Seek to target frame
             m_player->seek(m_lastRecordedFile, targetMs);
             
             // 强制刷新
             m_player->repaint();
             QCoreApplication::processEvents();
             
             m_currentPosition = targetMs;
             updateTimeLabel(targetMs, m_totalDuration);
        }
    });

    ctrlBarLayout->addWidget(m_btnOpenFile);
    ctrlBarLayout->addWidget(m_btnPlayPause);
    ctrlBarLayout->addWidget(m_lblTimeDisplay);
    ctrlBarLayout->addWidget(m_rangeSlider, 1);
    
    previewLayout->addLayout(ctrlBarLayout);

    // Trim Bar
    QHBoxLayout *trimLayout = new QHBoxLayout();
    trimLayout->setSpacing(10);
    
    m_lblTrimInfo = new QLabel("剪切范围: 00:00.000 - 00:00.000", grpPreview);
    trimLayout->addWidget(m_lblTrimInfo);
    trimLayout->addStretch();
    
    m_btnTrim = new QPushButton("剪切保存", grpPreview);
    m_btnTrim->setObjectName("BtnStandard");
    m_btnTrim->setFixedHeight(28);
    m_btnTrim->setCursor(Qt::PointingHandCursor);
    connect(m_btnTrim, &QPushButton::clicked, this, &MainWindow::onTrimClicked);
    
    trimLayout->addWidget(m_btnTrim);
    
    previewLayout->addLayout(trimLayout);

    leftLayout->addWidget(grpPreview);
    splitter->addWidget(leftPanel);

    // === 2.2 Right Module (Recording & History) ===
    QWidget *rightPanel = new QWidget(splitter);
    rightPanel->setObjectName("RightPanel");
    rightPanel->setMinimumWidth(340); // Use minimum width to allow resizing
    // rightPanel->setFixedWidth(360); // Removed fixed width
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(15);
    rightLayout->setContentsMargins(15, 15, 15, 5); // Reduced bottom margin

    // Recording Controls
    QGroupBox *grpRecord = new QGroupBox("录制控制", rightPanel);
    QVBoxLayout *recLayout = new QVBoxLayout(grpRecord);
    recLayout->setSpacing(15);
    recLayout->setContentsMargins(15, 25, 15, 15);

    // Audio Settings Grid
    QGridLayout *audioGrid = new QGridLayout();
    audioGrid->setVerticalSpacing(12);
    audioGrid->setHorizontalSpacing(10);
    
    m_chkSysAudio = new QCheckBox("录制系统声音", grpRecord);
    // Connect click logic for registration check
    connect(m_chkSysAudio, &QCheckBox::clicked, [this](bool checked){
        if (checked) {
            logMessage("用户开启系统声音录制，正在检测环境...");
            // Force process events to show log
            QApplication::processEvents(); 
            
            if (!m_recorder->checkSystemAudioAvailable()) {
                m_chkSysAudio->setChecked(false);
                logMessage("系统声音组件注册失败或被取消，已取消勾选。");
            } else {
                logMessage("系统声音组件检测通过。");
            }
        } else {
            logMessage("用户关闭系统声音录制。");
        }
    });
    
    m_sliderSysVol = new QSlider(Qt::Horizontal, grpRecord);
    m_sliderSysVol->setRange(0, 100); m_sliderSysVol->setValue(100);
    m_lblSysVolValue = new QLabel("100%", grpRecord);
    m_lblSysVolValue->setFixedWidth(35);
    connect(m_sliderSysVol, &QSlider::valueChanged, [this](int val){ m_lblSysVolValue->setText(QString::number(val) + "%"); });

    audioGrid->addWidget(m_chkSysAudio, 0, 0);
    audioGrid->addWidget(m_sliderSysVol, 0, 1);
    audioGrid->addWidget(m_lblSysVolValue, 0, 2);

    m_chkMicAudio = new QCheckBox("录制麦克风", grpRecord);
    connect(m_chkMicAudio, &QCheckBox::clicked, [this](bool checked){
        if (checked) {
            logMessage("用户开启麦克风录制。");
        } else {
            logMessage("用户关闭麦克风录制。");
        }
    });
    
    m_sliderMicVol = new QSlider(Qt::Horizontal, grpRecord);
    m_sliderMicVol->setRange(0, 100); m_sliderMicVol->setValue(100);
    m_lblMicVolValue = new QLabel("100%", grpRecord);
    m_lblMicVolValue->setFixedWidth(35);
    connect(m_sliderMicVol, &QSlider::valueChanged, [this](int val){ m_lblMicVolValue->setText(QString::number(val) + "%"); });

    audioGrid->addWidget(m_chkMicAudio, 1, 0);
    audioGrid->addWidget(m_sliderMicVol, 1, 1);
    audioGrid->addWidget(m_lblMicVolValue, 1, 2);

    recLayout->addLayout(audioGrid);

    // Start Screen Record Button - 点击后进入框选模式
    m_btnStartStop = new QPushButton("开始录屏", grpRecord);
    m_btnStartStop->setFixedHeight(46);
    m_btnStartStop->setObjectName("BtnAction");
    m_btnStartStop->setCursor(Qt::PointingHandCursor);
    connect(m_btnStartStop, &QPushButton::clicked, this, &MainWindow::onSelectAreaClicked);
    
    recLayout->addWidget(m_btnStartStop);

    rightLayout->addWidget(grpRecord);

    // History
    QGroupBox *grpHistory = new QGroupBox("历史记录 (支持多选)", rightPanel);
    QVBoxLayout *histLayout = new QVBoxLayout(grpHistory);
    histLayout->setContentsMargins(15, 25, 15, 15);
    histLayout->setSpacing(10);
    
    m_listHistory = new QListWidget(grpHistory);
    m_listHistory->setFrameShape(QFrame::NoFrame); 
    m_listHistory->setSelectionMode(QAbstractItemView::ExtendedSelection); 
    m_listHistory->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    
    m_btnDeleteHistory = new QPushButton("删除选中记录", grpHistory);
    m_btnDeleteHistory->setObjectName("BtnStandard");
    m_btnDeleteHistory->setFixedHeight(36);
    m_btnDeleteHistory->setCursor(Qt::PointingHandCursor);
    connect(m_btnDeleteHistory, &QPushButton::clicked, this, &MainWindow::onDeleteHistoryClicked);

    histLayout->addWidget(m_listHistory);
    histLayout->addWidget(m_btnDeleteHistory);

    rightLayout->addWidget(grpHistory, 1); 

    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);

    mainLayout->addWidget(splitter, 1); // Add stretch factor to push bottom panel down
    
    // === 3. Log Module (Bottom) ===
    // Use a wrapper widget to align with top panels (Margins: L20, R15 roughly)
    QWidget *bottomPanel = new QWidget(mainContainer);
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomPanel);
    bottomLayout->setContentsMargins(20, 0, 15, 15); // Align Left 20, Right 15. Bottom 15.
    
    QGroupBox *grpLog = new QGroupBox("运行日志", bottomPanel);
    grpLog->setMaximumHeight(100); 
    QVBoxLayout *logLayout = new QVBoxLayout(grpLog);
    logLayout->setContentsMargins(10, 15, 10, 5); 
    
    m_logConsole = new QTextEdit(grpLog);
    m_logConsole->setReadOnly(true);
    m_logConsole->setObjectName("LogConsole");
    logLayout->addWidget(m_logConsole);
    
    bottomLayout->addWidget(grpLog);
    mainLayout->addWidget(bottomPanel);

    // Overlays
    m_overlay = new SelectionOverlay(nullptr); 
    connect(m_overlay, &SelectionOverlay::areaSelected, this, &MainWindow::onAreaSelected);
    connect(m_overlay, &SelectionOverlay::cancelled, this, &MainWindow::onSelectionCancelled);
    connect(m_overlay, &SelectionOverlay::requestStartRecording, this, &MainWindow::startRecordingInternal);
    connect(m_overlay, &SelectionOverlay::requestStopRecording, [this](){
        if (m_recorder->state() == RecorderController::Recording) {
            m_recorder->stopRecording();
        }
    });

    // CountdownOverlay is no longer used - countdown handled in SelectionOverlay toolbar
    m_countdownOverlay = nullptr;

    resize(1180, 830);
}

void MainWindow::setupStyle() {
    QString theme = m_settings->value("theme", "dark").toString();
    applyTheme(theme);
}

void MainWindow::applyTheme(QString themeName) {
    themeName = themeName.toLower().trimmed();
    
    if (m_themeOverlay) m_themeOverlay->update();
    
    // Define Color Palettes
    QString borderColor = "#333333";
    QString bgColor = "#1e1e1e";
    QString textColor = "#ffffff";
    QString groupBg = "#2d2d2d";
    QString btnBg = "#333333";
    QString highlight = "#007acc";
    QString highlightRgb = "0, 122, 204"; // RGB for rgba() usage

    // Extended color logic for better icon adaptation
    QColor iconColor = Qt::white;

    if (themeName == "light") {
        borderColor = "#cccccc"; bgColor = "#f5f5f5"; textColor = "#000000";
        groupBg = "rgba(255,255,255,0.8)"; btnBg = "#ffffff"; highlight = "#0078d7";
        highlightRgb = "0, 120, 215";
        iconColor = Qt::black;
    } else if (themeName == "pink") {
        borderColor = "#ffb6c1"; bgColor = "#fff0f5"; textColor = "#000000";
        groupBg = "rgba(255,255,255,0.6)"; btnBg = "#ffffff"; highlight = "#ff69b4";
        highlightRgb = "255, 105, 180";
        iconColor = QColor("#552233");
    } else if (themeName == "tech") {
        borderColor = "#2a3d5c"; bgColor = "#121a2e"; textColor = "#ffffff";
        groupBg = "rgba(20, 30, 50, 0.8)"; btnBg = "#1a2a40"; highlight = "#00e5ff";
        highlightRgb = "0, 229, 255";
        iconColor = QColor("#aaddff");
    } else if (themeName == "purple") {
        borderColor = "#7a5a9a"; bgColor = "#3e2e4e"; textColor = "#ffffff";
        groupBg = "rgba(80, 60, 100, 0.8)"; btnBg = "#504060"; highlight = "#e0b0ff";
        highlightRgb = "224, 176, 255";
        iconColor = QColor("#e0b0ff");
    } else if (themeName == "green") {
        borderColor = "#5a7a5a"; bgColor = "#2e3e2e"; textColor = "#ffffff";
        groupBg = "rgba(60, 80, 60, 0.8)"; btnBg = "#405040"; highlight = "#90ee90";
        highlightRgb = "144, 238, 144";
        iconColor = QColor("#a0e0a0");
    } else if (themeName == "zijunpink") {
        // 子君粉：底色 #E6C7C0，字体 #F8F4EE
        borderColor = "#d4b0a8"; bgColor = "#E6C7C0"; textColor = "#F8F4EE";
        groupBg = "rgba(248, 244, 238, 0.25)"; btnBg = "#dbb8b0"; highlight = "#c9a8a0";
        highlightRgb = "201, 168, 160";
        iconColor = QColor("#F8F4EE");
    } else if (themeName == "zijunwhite") {
        // 子君白：底色 #F8F4EE，字体 #E6C7C0
        borderColor = "#E6C7C0"; bgColor = "#F8F4EE"; textColor = "#c9a8a0";
        groupBg = "rgba(230, 199, 192, 0.2)"; btnBg = "#ffffff"; highlight = "#E6C7C0";
        highlightRgb = "230, 199, 192";
        iconColor = QColor("#c9a8a0");
    }

    // Construct QSS
    QString qss = QString(R"(
        QWidget { font-family: 'Microsoft YaHei'; font-size: 9pt; color: %3; }
        #MainContainer { background-color: %2; border: 1px solid %1; border-radius: 8px; }
        QGroupBox { font-weight: bold; border: 1px solid %1; border-radius: 6px; margin-top: 12px; background: %4; color: %3; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: %3; opacity: 0.8; }
        
        QPushButton { border: 1px solid %1; border-radius: 4px; background: %5; padding: 4px; color: %3; }
        QPushButton:hover { background: rgba(128,128,128,0.2); border-color: %6; }
        
        #BtnStandard { border: 1px solid %1; background: %5; color: %3; }
        #BtnStandard:hover { background: rgba(128,128,128,0.15); border: 1px solid %6; color: %6; }
        
        #BtnAction { border: 2px solid %6; color: %6; background: rgba(128,128,128,0.05); font-weight: bold; border-radius: 6px; }
        #BtnAction:hover { background: rgba(%7, 0.3); color: %6; border-color: %6; }
        
        #BtnMedia { border: 1px solid %1; background: transparent; border-radius: 4px; }
        #BtnMedia:hover { background: rgba(128,128,128,0.2); border-color: %6; }
        
        #BtnWindow { border: 1px solid %1; background: transparent; border-radius: 4px; }
        #BtnWindow:hover { background-color: %1; color: %3; border-color: %6; }

        QTextEdit { background: %2; color: %3; border: 1px solid %1; }
        
        QScrollBar:vertical {
            border: none;
            background: rgba(128, 128, 128, 0.1);
            width: 8px;
            margin: 0px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: rgba(128, 128, 128, 0.4);
            min-height: 20px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
            background: rgba(128, 128, 128, 0.6);
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        
        QSlider::groove:horizontal { height: 6px; background: rgba(128,128,128,0.3); border-radius: 3px; }
        QSlider::handle:horizontal { width: 14px; margin: -5px 0; border-radius: 7px; background: %6; border: 2px solid %2; }
        QSlider::sub-page:horizontal { background: %6; border-radius: 3px; }

        QListWidget { background: transparent; border: none; outline: none; color: %3; }
        QListWidget::item { border-radius: 4px; padding: 2px; border: 1px solid transparent; color: %3; }
        QListWidget::item:selected { background: rgba(%7, 0.25); color: %3; border: 1px solid rgba(%7, 0.5); }
        QListWidget::item:selected:!active { background: rgba(%7, 0.15); color: %3; }
        QListWidget::item:hover:!selected { background: rgba(128,128,128,0.08); border: 1px solid rgba(128,128,128,0.2); }
        QListWidget::item:selected:hover { background: rgba(%7, 0.35); border: 1px solid rgba(%7, 0.6); }
        
        QCheckBox { spacing: 5px; color: %3; }
        QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid %1; border-radius: 3px; background: transparent; }
        QCheckBox::indicator:checked { background-color: %6; border: 1px solid %6; } 
        QCheckBox::indicator:checked:hover { background-color: %6; border-color: %6; }
        QLabel { color: %3; }
    )")
    .arg(borderColor, bgColor, textColor, groupBg, btnBg, highlight, highlightRgb);

    setStyleSheet(qss);
    
    // Global menu style (for tray menu and combobox popups)
    QString globalMenuStyle = QString(
        "QMenu { background-color: %1; border: 1px solid %2; border-radius: 6px; padding: 5px; }"
        "QMenu::item { background-color: transparent; color: %3; padding: 8px 25px; border-radius: 4px; margin: 2px 4px; }"
        "QMenu::item:selected { background-color: %4; }"
        "QMenu::separator { height: 1px; background: %2; margin: 5px 10px; }"
        "QComboBox QAbstractItemView { background-color: %1; border: 1px solid %2; border-radius: 4px; selection-background-color: %4; outline: none; }"
        "QComboBox QAbstractItemView::item { color: %3; padding: 6px 10px; min-height: 24px; }"
        "QComboBox QAbstractItemView::item:selected { background-color: %4; color: %3; }"
        "QComboBox QAbstractItemView::item:hover { background-color: %2; }"
    ).arg(bgColor, borderColor, textColor, highlight);
    qApp->setStyleSheet(qApp->styleSheet() + globalMenuStyle);
    
    // Apply calculated iconColor
    m_btnMin->setIcon(createIcon(style()->standardIcon(QStyle::SP_TitleBarMinButton), iconColor));
    m_btnMax->setIcon(createIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton), iconColor));
    m_btnClose->setIcon(createIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton), iconColor));
    
    // Draw Custom Gear Icon for Settings
    m_btnSettings->setIcon(drawGearIcon(iconColor)); 
    
    // Update Open File Button Icon
    if (m_btnOpenFile) {
        m_btnOpenFile->setIcon(createIcon(style()->standardIcon(QStyle::SP_DirOpenIcon), iconColor));
    }

    // Update Play Button Icon to match theme
    if (m_btnPlayPause) {
        QIcon playIcon = m_isPlaying ? style()->standardIcon(QStyle::SP_MediaPause) : style()->standardIcon(QStyle::SP_MediaPlay);
        m_btnPlayPause->setIcon(createIcon(playIcon, iconColor));
    }

    // Refresh history list to apply new colors (check for nullptr)
    if (m_listHistory) {
        refreshHistoryList();
    }
}

void MainWindow::onStartStopClicked() {
    // 这个函数不再使用，改为直接进入框选模式
    onSelectAreaClicked();
}

void MainWindow::startRecordingInternal() {
    m_pendingRecording = false;
    
    if (m_chkSysAudio->isChecked() && !m_recorder->checkSystemAudioAvailable()) {
        ToastTip::warning(this, "系统声音设备不可用");
        m_chkSysAudio->setChecked(false);
    }

    // 录制区域往内缩小2px，避免把框线录进去
    QRect recordRegion = m_currentSelection.adjusted(2, 2, -2, -2);
    m_recorder->setRegion(recordRegion);
    m_recorder->setAudioConfig(m_chkSysAudio->isChecked(), m_sliderSysVol->value() / 100.0, 
                               m_chkMicAudio->isChecked(), m_sliderMicVol->value() / 100.0);
    
    m_recorder->setFps(m_settings->value("fps", 30).toInt());

    m_recorder->startRecording();
    
    // Save current audio config
    m_settings->setValue("recordSysAudio", m_chkSysAudio->isChecked());
    m_settings->setValue("recordMicAudio", m_chkMicAudio->isChecked());
    m_settings->setValue("sysVolume", m_sliderSysVol->value());
    m_settings->setValue("micVolume", m_sliderMicVol->value());
    
    logMessage("Recording started.");
}

void MainWindow::onRecorderStateChanged(RecorderController::State state) {
    if (state == RecorderController::Recording) {
        m_btnStartStop->setEnabled(false);
        m_btnSettings->setEnabled(false);
        
        // Timer should start here, when actual recording starts
        m_currentDuration = 0; // Reset duration
        m_recTimer->start();
        
        // Update overlay state
        if (m_overlay) {
            m_overlay->startRecording();
        }
    } else {
        // Re-enable UI
        m_btnStartStop->setEnabled(true);
        m_btnSettings->setEnabled(true);
        m_recTimer->stop();
        
        // Close overlay and show main window
        if (m_overlay) {
            m_overlay->stopRecording();
        }
    }
}

void MainWindow::saveAndAddToHistory(const QString &path) {
    m_lastRecordedFile = path;
    m_historyMgr->addRecord(path, m_currentDuration / 1000);
    refreshHistoryList();
    
    if (m_settings->value("minimizeToTray", true).toBool()) {
        showNormal();
        activateWindow();
    } else {
        // If not min to tray, we still want to show normal to preview
        showNormal();
        activateWindow();
    }
    
    // QMessageBox::information(this, "录制完成", "视频已保存至:\n" + path);
    logMessage("Recording saved: " + path);
    loadVideo(path);
}

void MainWindow::loadVideo(const QString &path, qint64 durationMs) {
    logMessage("Loading video: " + path);
    m_lastRecordedFile = path;
    m_player->show();
    m_previewLabel->hide();
    m_player->startPlay(path);
    
    // Update play button icon with theme color
    m_btnPlayPause->setIcon(createIcon(style()->standardIcon(QStyle::SP_MediaPause), getThemeIconColor()));
    
    m_isPlaying = true;
    m_totalDuration = getVideoDuration(path); 
    if (durationMs > 0) m_totalDuration = durationMs;
    
    m_rangeSlider->setRange(0, 1000);
    m_rangeSlider->setValues(0, 1000);
    
    updateTimeLabel(0, m_totalDuration);
}

void MainWindow::onPlayerFinished() {
    logMessage("Playback finished.");
    m_isPlaying = false;
    m_btnPlayPause->setIcon(createIcon(style()->standardIcon(QStyle::SP_MediaPlay), getThemeIconColor()));
    
    m_rangeSlider->setPlaybackValue(-1);

    if (m_totalDuration > 0 && !m_lastRecordedFile.isEmpty()) {
        int lower = m_rangeSlider->lowerValue();
        qint64 startMs = lower * m_totalDuration / 1000;
        
        m_currentPosition = startMs;
        m_player->seek(m_lastRecordedFile, startMs); // Show start frame
        updateTimeLabel(startMs, m_totalDuration);
    }
}

void MainWindow::onPlayerPositionChanged(qint64 ms) {
    if (!m_isPlaying) return; // Ignore signals if stopped

    m_currentPosition = ms;
    if (m_totalDuration > 0) {
        int sliderVal = static_cast<int>(ms * 1000 / m_totalDuration);
        
        int upper = m_rangeSlider->upperValue();
        if (sliderVal >= upper) {
             m_player->stopPlay();
             m_isPlaying = false;
             m_btnPlayPause->setIcon(createIcon(style()->standardIcon(QStyle::SP_MediaPlay), getThemeIconColor()));
             
             // Reset UI immediately
             m_rangeSlider->setPlaybackValue(-1);
             
             int lower = m_rangeSlider->lowerValue();
             qint64 startMs = lower * m_totalDuration / 1000;
             m_currentPosition = startMs;
             m_player->seek(m_lastRecordedFile, startMs);
             updateTimeLabel(startMs, m_totalDuration);
             return;
        }
        
        m_rangeSlider->setPlaybackValue(sliderVal);
    }
    updateTimeLabel(ms, m_totalDuration);
}

void MainWindow::updateTimeLabel(qint64 current, qint64 total) {
    qint64 endMs = total;
    if (m_rangeSlider && total > 0) {
        endMs = m_rangeSlider->upperValue() * total / 1000;
    }
    m_lblTimeDisplay->setText(QString("%1 / %2").arg(formatTime(current)).arg(formatTime(endMs)));
    
    if (m_lblTrimInfo && total > 0) {
        qint64 startMs = m_rangeSlider->lowerValue() * total / 1000;
        m_lblTrimInfo->setText(QString("剪切范围: %1 - %2").arg(formatTime(startMs)).arg(formatTime(endMs)));
    }
}

void MainWindow::onPlayPauseClicked() { 
    if (m_isPlaying) {
        logMessage("Stopping playback.");
        m_player->stopPlay();
        m_isPlaying = false;
        m_btnPlayPause->setIcon(createIcon(style()->standardIcon(QStyle::SP_MediaPlay), getThemeIconColor()));
        
        m_rangeSlider->setPlaybackValue(-1);
        
        if (m_totalDuration > 0 && !m_lastRecordedFile.isEmpty()) {
            int lower = m_rangeSlider->lowerValue();
            qint64 startMs = lower * m_totalDuration / 1000;
            m_currentPosition = startMs;
            m_player->seek(m_lastRecordedFile, startMs); // Show start frame
            updateTimeLabel(startMs, m_totalDuration);
        }
    } else {
        if (!m_lastRecordedFile.isEmpty()) {
            logMessage("Starting playback: " + m_lastRecordedFile);
            
            qint64 startMs = 0;
            qint64 endMs = m_totalDuration;
            if (m_totalDuration > 0) {
                startMs = m_rangeSlider->lowerValue() * m_totalDuration / 1000;
                endMs = m_rangeSlider->upperValue() * m_totalDuration / 1000;
            }
            
            // 设置播放范围（从开始滑块到结束滑块）
            m_player->setRange(startMs, endMs);
            m_player->startPlay(m_lastRecordedFile); 
            
            m_isPlaying = true;
            m_btnPlayPause->setIcon(createIcon(style()->standardIcon(QStyle::SP_MediaStop), getThemeIconColor()));
        }
    }
}

void MainWindow::onSelectAreaClicked() {
    // 隐藏主窗口，显示框选遮罩
    hide();
    QTimer::singleShot(200, this, [this](){
        // 设置倒计时配置
        bool countEnabled = m_settings->value("countdownEnabled", true).toBool();
        int countSecs = m_settings->value("countdownSecs", 3).toInt();
        m_overlay->setCountdownEnabled(countEnabled, countSecs);
        
        m_overlay->showFullScreen();
        m_overlay->update();
    });
}

void MainWindow::onFullScreenClicked() {
    // 不再需要，保留空实现以防止编译错误
}

void MainWindow::onAreaSelected(const QRect &rect) {
    m_currentSelection = rect;
    // 不再立即显示主窗口，等待用户点击工具栏上的开始按钮
    logMessage(QString("Area selected: %1x%2").arg(rect.width()).arg(rect.height()));
}

void MainWindow::onSelectionCancelled() {
    showNormal();
}

void MainWindow::onOpenFileClicked() {
    QString path = QFileDialog::getOpenFileName(this, "打开视频", m_settings->value("savePath").toString(), "Video Files (*.mp4 *.avi *.mkv)");
    if (!path.isEmpty()) {
        loadVideo(path);
    }
}

void MainWindow::onTrimClicked() {
    if (m_lastRecordedFile.isEmpty() || m_totalDuration <= 0) {
        ToastTip::warning(this, "请先加载视频文件");
        return;
    }
    
    qint64 startMs = m_rangeSlider->lowerValue() * m_totalDuration / 1000;
    qint64 endMs = m_rangeSlider->upperValue() * m_totalDuration / 1000;
    
    if (endMs <= startMs) {
        ToastTip::warning(this, "剪切范围无效");
        return;
    }
    
    // 禁用按钮防止重复点击
    m_btnTrim->setEnabled(false);
    m_btnTrim->setText("剪切中...");
    
    QString dir = QFileInfo(m_lastRecordedFile).absolutePath();
    QString name = QFileInfo(m_lastRecordedFile).baseName() + "_trim_" + QDateTime::currentDateTime().toString("HHmmss") + ".mp4";
    QString outPath = dir + "/" + name;
    
    // 断开之前的连接，避免重复
    disconnect(m_videoUtils, nullptr, this, nullptr);
    
    // 连接处理完成信号
    qint64 durationMs = endMs - startMs;
    connect(m_videoUtils, &VideoUtils::processingFinished, this, [this, durationMs](bool success, const QString &output) {
        m_btnTrim->setEnabled(true);
        m_btnTrim->setText("剪切视频");
        if (success) {
            m_historyMgr->addRecord(output, durationMs / 1000);
            refreshHistoryList();
            ToastTip::success(this, "视频剪切完成");
            logMessage("Video trimmed: " + output);
        }
        disconnect(m_videoUtils, nullptr, this, nullptr);
    });
    
    connect(m_videoUtils, &VideoUtils::processingError, this, [this](const QString &error) {
        m_btnTrim->setEnabled(true);
        m_btnTrim->setText("剪切视频");
        ToastTip::error(this, "剪切失败: " + error);
        logMessage("Trim error: " + error);
        disconnect(m_videoUtils, nullptr, this, nullptr);
    });
    
    // 执行剪切
    logMessage(QString("Trimming video: %1 -> %2 (range: %3-%4 ms)").arg(m_lastRecordedFile).arg(outPath).arg(startMs).arg(endMs));
    m_videoUtils->trimVideoMs(m_lastRecordedFile, outPath, startMs, endMs);
}

void MainWindow::onSettingsClicked() {
    SettingsDialog dlg(this);
    connect(&dlg, &SettingsDialog::hotkeyChanged, this, &MainWindow::registerHotkeys);
    if (dlg.exec() == QDialog::Accepted) {
        loadSettings();
        setupStyle();
        registerHotkeys(); // 重新注册快捷键
    }
}

void MainWindow::loadSettings() {
    if (m_settings) m_settings->sync(); // Ensure settings are up-to-date
    
    // Reload settings
    bool sysAudio = m_settings->value("recordSysAudio", false).toBool(); // Default false
    if (sysAudio) {
        logMessage("配置: 尝试自动开启系统声音录制...");
        if (m_recorder->checkSystemAudioAvailable()) {
            m_chkSysAudio->setChecked(true);
            logMessage("配置: 系统声音已自动开启。");
        } else {
            m_chkSysAudio->setChecked(false);
            logMessage("配置: 系统声音组件未就绪，保持关闭。");
        }
    } else {
        m_chkSysAudio->setChecked(false);
    }
    
    bool micAudio = m_settings->value("recordMicAudio", false).toBool(); // Default false
    m_chkMicAudio->setChecked(micAudio);
    
    m_sliderSysVol->setValue(m_settings->value("sysVolume", 100).toInt());
    m_sliderMicVol->setValue(m_settings->value("micVolume", 100).toInt());
}

// Helper for friendly duration format
static QString formatDurationFriendly(qint64 ms) {
    int totalSeconds = ms / 1000;
    int mins = totalSeconds / 60;
    int secs = totalSeconds % 60;
    if (mins > 0) {
        return QString("%1 mins %2 s").arg(mins).arg(secs);
    } else {
        return QString("%1 s").arg(secs);
    }
}

void MainWindow::refreshHistoryList() {
    m_listHistory->clear();
    auto records = m_historyMgr->getHistory(); 
    
    QString themeName = m_settings->value("theme", "dark").toString().toLower();
    QColor iconColor = (themeName == "light" || themeName == "pink") ? Qt::black : Qt::white;
    QColor textColor = (themeName == "light" || themeName == "pink") ? Qt::black : Qt::white;
    if (themeName == "pink") {
        iconColor = QColor("#552233");
        textColor = QColor("#552233");
    } else if (themeName == "zijunpink") {
        iconColor = QColor("#F8F4EE");
        textColor = QColor("#F8F4EE");
    } else if (themeName == "zijunwhite") {
        iconColor = QColor("#c9a8a0");
        textColor = QColor("#c9a8a0");
    }

    for (const auto &rec : records) {
        if (!rec.exists) continue;
        
        QListWidgetItem *item = new QListWidgetItem(m_listHistory);
        item->setData(Qt::UserRole, rec.id); 
        item->setSizeHint(QSize(0, 42)); // Reduced height from 60 to 42
        
        QString durationStr = formatDurationFriendly(rec.durationSec * 1000);
        QString timeStr = rec.createTime.toStringEx("MM-dd HH:mm");
        
        HistoryItemWidget *widget = new HistoryItemWidget(rec.id, rec.filePath, durationStr, timeStr, iconColor, textColor, this);        
        m_listHistory->setItemWidget(item, widget);
    }
}

// Helper to get current icon color based on theme
QColor MainWindow::getThemeIconColor() const {
    QString themeName = m_settings->value("theme", "dark").toString().toLower();
    if (themeName == "light") return Qt::black;
    if (themeName == "pink") return QColor("#552233");
    if (themeName == "tech") return QColor("#aaddff");
    if (themeName == "purple") return QColor("#e0b0ff");
    if (themeName == "green") return QColor("#a0e0a0");
    if (themeName == "zijunpink") return QColor("#F8F4EE");
    if (themeName == "zijunwhite") return QColor("#c9a8a0");
    return Qt::white;
}

void MainWindow::onHistoryItemDoubleClicked(QListWidgetItem *item) {
    QWidget *w = m_listHistory->itemWidget(item);
    if (w) {
        // ...
    }
}

void MainWindow::onDeleteHistoryClicked() {
    auto selectedItems = m_listHistory->selectedItems();
    if (selectedItems.isEmpty()) {
        ToastTip::info(this, "请先选择要删除的记录");
        return;
    }
    
    if (QMessageBox::question(this, "确认删除", QString("确定要删除选中的 %1 条记录吗？").arg(selectedItems.count())) != QMessageBox::Yes) {
        return;
    }

    for (QListWidgetItem *item : selectedItems) {
        QString id = item->data(Qt::UserRole).toString();
            m_historyMgr->deleteRecord(id);
        delete m_listHistory->takeItem(m_listHistory->row(item));
    }
}

void MainWindow::updateAudioLevels(double sys, double mic) {
    // ...
}

void MainWindow::logMessage(const QString &msg) {
    QString timeStr = QDateTime::currentDateTime().toStringEx("HH:mm:ss ");
    if (m_logConsole) {
        m_logConsole->append(timeStr + msg);
        // Auto scroll
        m_logConsole->verticalScrollBar()->setValue(m_logConsole->verticalScrollBar()->maximum());
    }
    LogManager::instance().write(msg, LogManager::App);
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_titleBar->geometry().contains(event->pos())) {
        m_isDragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}
void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}
void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    m_isDragging = false;
}

void MainWindow::onMinimizeClicked() { 
    if (m_settings->value("minimizeToTray", true).toBool()) {
        hide(); // Hide triggers hideEvent -> shows FloatingBall
    } else {
        showMinimized(); // Regular minimize, might still show FloatingBall depending on changeEvent, but user said 'minimized to tray'
    }
}
void MainWindow::onMaximizeClicked() {
    if (isMaximized()) showNormal();
    else showMaximized();
    updateMaximizeButton();
}
void MainWindow::onCloseClicked() { close(); }
void MainWindow::updateMaximizeButton() {
    // Simple logic
}

void MainWindow::closeEvent(QCloseEvent *event) {
            if (m_recorder->state() == RecorderController::Recording) {
        QMessageBox::StandardButton res = QMessageBox::question(this, "正在录制", "录制正在进行中，是否停止并退出？");
        if (res != QMessageBox::Yes) {
            event->ignore();
            return;
        }
        m_recorder->stopRecording();
    }
    
    if (m_settings->value("minimizeToTray", true).toBool() && isVisible()) {
        hide();
        event->ignore();
        if (m_trayIcon) m_trayIcon->showMessage("MScreenRecord", "程序已最小化到托盘", QSystemTrayIcon::Information, 2000);
    } else {
        event->accept();
        QApplication::quit();
    }
}

void MainWindow::setupTrayIcon() {
    m_trayIcon = new QSystemTrayIcon(this);
    
    QString iconPath = QCoreApplication::applicationDirPath() + "/resources/app.ico";
    if (!QFile::exists(iconPath)) {
        iconPath = QCoreApplication::applicationDirPath() + "/resources/app.svg";
    }
    
    if (QFile::exists(iconPath)) {
        m_trayIcon->setIcon(QIcon(iconPath));
    } else {
        m_trayIcon->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    }
    
    m_trayMenu = new QMenu(this);
    QAction *actShow = m_trayMenu->addAction("显示主窗口");
    connect(actShow, &QAction::triggered, this, &MainWindow::showNormal);
    QAction *actQuit = m_trayMenu->addAction("退出");
    connect(actQuit, &QAction::triggered, qApp, &QCoreApplication::quit);
    
    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();
    
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        showNormal();
        activateWindow();
    }
}

void MainWindow::setupFloatingBall() {
    // FloatingBall 不再用于录制控制，保留作为托盘最小化时的可选UI
    m_floatingBall = nullptr;
}

void MainWindow::registerHotkeys() {
    GlobalHotkey *hotkey = GlobalHotkey::instance();
    
    // 连接信号
    connect(hotkey, &GlobalHotkey::hotkeyTriggered, this, &MainWindow::onHotkeyTriggered, Qt::UniqueConnection);
    
    // 获取快捷键设置
    QSettings settings("KSO", "MScreenRecord");
    QString showWindowKey = settings.value("hotkeyShowWindow", "Ctrl+Alt+S").toString();
    QString startRecordKey = settings.value("hotkeyStartRecord", "Ctrl+Alt+O").toString();
    
    // 注册显示主界面快捷键
    if (!showWindowKey.isEmpty()) {
        QKeySequence seq(showWindowKey);
        if (!seq.isEmpty()) {
            HotkeyEdit tempEdit;
            tempEdit.setKeySequence(seq);
            hotkey->forceRegisterHotkey(GlobalHotkey::ShowMainWindow, 
                                        tempEdit.getModifiers(), tempEdit.getVirtualKey());
        }
    }
    
    // 注册开始/停止录制快捷键
    if (!startRecordKey.isEmpty()) {
        QKeySequence seq(startRecordKey);
        if (!seq.isEmpty()) {
            HotkeyEdit tempEdit;
            tempEdit.setKeySequence(seq);
            hotkey->forceRegisterHotkey(GlobalHotkey::StartStopRecording,
                                        tempEdit.getModifiers(), tempEdit.getVirtualKey());
        }
    }
}

void MainWindow::onHotkeyTriggered(int id) {
    switch (id) {
        case GlobalHotkey::ShowMainWindow:
            // 显示主界面
            if (isHidden() || isMinimized()) {
                show();
                raise();
                activateWindow();
                setWindowState(windowState() & ~Qt::WindowMinimized);
            } else {
                // 如果已显示，则隐藏到托盘
                if (m_settings->value("minimizeToTray", true).toBool()) {
                    hide();
                } else {
                    showMinimized();
                }
            }
            break;
            
        case GlobalHotkey::StartStopRecording:
            // 开始/停止录制
            if (m_recorder->state() == RecorderController::Recording) {
                // 正在录制，停止
                if (m_overlay && m_overlay->isVisible()) {
                    m_overlay->stopRecording();
                    emit m_overlay->requestStopRecording();
                }
            } else if (m_recorder->state() == RecorderController::Stopped) {
                // 空闲状态，开始录制
                onSelectAreaClicked();
            }
            break;
    }
}

QString MainWindow::formatTime(qint64 ms) const {
    int seconds = (ms / 1000) % 60;
    int minutes = (ms / 60000); // Show total minutes, no hours
    int msec = ms % 1000;
    return QString("%1:%2.%3")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
        .arg(msec, 3, 10, QChar('0'));
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized()) {
             // If minimized (not hidden), we can also show floating ball if desired
             // But 'minimizeToTray' usually means hiding.
             if (m_floatingBall && !m_floatingBall->isVisible() && m_settings->value("minimizeToTray", true).toBool()) {
                 // If regular minimize but user wants tray mode, maybe we should've hidden.
             }
        }
        updateMaximizeButton();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    setAttribute(Qt::WA_Mapped);
}

void MainWindow::hideEvent(QHideEvent *event) {
    QMainWindow::hideEvent(event);
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result) {
    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    if (m_themeOverlay) {
        QWidget *c = findChild<QWidget*>("MainContainer");
        if (c) m_themeOverlay->resize(c->size());
    }
    QMainWindow::resizeEvent(event);
}

void MainWindow::onSeekSliderPressed() {
    m_isSeeking = true;
}

void MainWindow::onSeekSliderReleased() {
    m_isSeeking = false;
}

void MainWindow::onSeekSliderMoved(int val) {
    if (m_isSeeking && m_totalDuration > 0) {
        // ...
    }
}
