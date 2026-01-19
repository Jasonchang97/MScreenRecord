#include "SettingsDialog.h"
#include "AppVersion.h"
#include "ThemePatternWidget.h"
#include "HotkeyEdit.h"
#include "GlobalHotkey.h"
#include "CustomMessageBox.h"
#include "ToastTip.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>
#include <QMouseEvent>
#include <QPainter>
#include <QComboBox>
#include <QAbstractItemView>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QGraphicsDropShadowEffect>
#include <QGroupBox>

// 辅助：图标颜色函数（复用）
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

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent), m_isDragging(false),
    m_hotkeyShowWindow(nullptr), m_hotkeyStartRecord(nullptr) {
    setObjectName("SettingsDialog");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(470, 500); // 增加高度以容纳快捷键设置
    
    QSettings s("KSO", "MScreenRecord");
    QString theme = s.value("theme", "dark").toString().toLower().trimmed();
    
    QString borderColor = "#333333";
    QString bgColor = "#1e1e1e";
    QString textColor = "#ffffff"; // Default Dark -> White
    
    if (theme == "light") {
        borderColor = "#cccccc"; bgColor = "#f5f5f5"; textColor = "#000000"; // Light -> Black
    } else if (theme == "tech") {
        borderColor = "#2a3d5c"; bgColor = "#121a2e"; textColor = "#ffffff"; // Dark -> White
    } else if (theme == "pink") {
        borderColor = "#ffb6c1"; bgColor = "#fff0f5"; textColor = "#000000"; // Light -> Black
    } else if (theme == "purple") {
        borderColor = "#7a5a9a"; bgColor = "#3e2e4e"; textColor = "#ffffff"; // Dark -> White (Lighter Purple)
    } else if (theme == "green") {
        borderColor = "#5a7a5a"; bgColor = "#2e3e2e"; textColor = "#ffffff"; // Dark -> White (Lighter Green)
    } else if (theme == "zijunpink") {
        borderColor = "#C7BBA8"; bgColor = "#F0D5CF"; textColor = "#5D4A42"; // 子君粉（浅粉）
    } else if (theme == "zijunwhite") {
        borderColor = "#E6C7C0"; bgColor = "#F8F4EE"; textColor = "#6B5D53"; // 子君白
    }

    if (parent) setStyleSheet(parent->styleSheet());

    // 1. Shadow Wrapper
    QVBoxLayout *shadowLayout = new QVBoxLayout(this);
    shadowLayout->setContentsMargins(10, 10, 10, 10);
    
    QWidget *container = new QWidget(this);
    container->setObjectName("SettingsContainer");
    container->setAttribute(Qt::WA_StyledBackground, true);
    container->setAutoFillBackground(true); // Force fill
    QPalette pal = container->palette();
    pal.setColor(QPalette::Window, QColor(bgColor));
    pal.setColor(QPalette::WindowText, QColor(textColor));
    pal.setColor(QPalette::Text, QColor(textColor));
    pal.setColor(QPalette::ButtonText, QColor(textColor));
    container->setPalette(pal);
    
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(0, 0, 0, 100));
    container->setGraphicsEffect(shadow);
    
    // Theme Pattern Overlay (Child of container, behind content)
    m_themeOverlay = new ThemePatternWidget(container);
    m_themeOverlay->lower(); // Stack at bottom
    
    shadowLayout->addWidget(container);

    QVBoxLayout *rootLayout = new QVBoxLayout(container);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // === 自定义标题栏 ===
    m_titleBar = new QWidget(container);
    m_titleBar->setObjectName("TitleBar"); 
    m_titleBar->setFixedHeight(40);
    
    QHBoxLayout *titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(15, 0, 10, 0);
    
    QLabel *lblTitle = new QLabel("设置", container);
    lblTitle->setObjectName("TitleLabel");
    lblTitle->setStyleSheet("font-weight: bold; font-size: 14px;");
    
    QPushButton *btnClose = new QPushButton(container);
    btnClose->setFixedSize(30, 30);
    btnClose->setFlat(true);
    btnClose->setObjectName("CloseButton");
    
    QColor iconColor = (theme == "light") ? Qt::black : Qt::white;
    if (theme == "pink") iconColor = QColor("#552233");
    if (theme == "tech") iconColor = QColor("#aaddff");
    if (theme == "purple") iconColor = QColor("#e0b0ff");
    if (theme == "green") iconColor = QColor("#a0e0a0");
    if (theme == "zijunpink") iconColor = QColor("#5D4A42");
    if (theme == "zijunwhite") iconColor = QColor("#6B5D53");

    btnClose->setIcon(createIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton), iconColor));
    
    connect(btnClose, &QPushButton::clicked, this, &SettingsDialog::reject);
    
    titleLayout->addWidget(lblTitle);
    titleLayout->addStretch();
    titleLayout->addWidget(btnClose);
    
    rootLayout->addWidget(m_titleBar);

    // === 内容区域 ===
    QWidget *contentWidget = new QWidget(container);
    contentWidget->setObjectName("SettingsContent"); 
    QVBoxLayout *mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 保存路径
    QHBoxLayout *pathLayout = new QHBoxLayout();
    m_editPath = new QLineEdit(container);
    QPushButton *btnBrowse = new QPushButton(container);
    btnBrowse->setIcon(createIcon(style()->standardIcon(QStyle::SP_DirIcon), iconColor));
    connect(btnBrowse, &QPushButton::clicked, this, &SettingsDialog::onBrowseClicked);
    pathLayout->addWidget(new QLabel("保存路径:", container));
    pathLayout->addWidget(m_editPath, 1);
    pathLayout->addWidget(btnBrowse);
    mainLayout->addLayout(pathLayout);

    // 帧率
    QHBoxLayout *fpsLayout = new QHBoxLayout();
    m_spinFps = new QSpinBox(container);
    m_spinFps->setRange(10, 60);
    fpsLayout->addWidget(new QLabel("录制帧率:", container));
    fpsLayout->addWidget(m_spinFps);
    fpsLayout->addStretch();
    mainLayout->addLayout(fpsLayout);

    // 码率
    QHBoxLayout *bitrateLayout = new QHBoxLayout();
    m_comboBitrate = new QComboBox(container);
    m_comboBitrate->addItems({"高 (原画)", "中 (均衡)", "低 (流畅)"});
    bitrateLayout->addWidget(new QLabel("视频质量:", container));
    bitrateLayout->addWidget(m_comboBitrate);
    bitrateLayout->addStretch();
    mainLayout->addLayout(bitrateLayout);

    // 主题
    QHBoxLayout *themeLayout = new QHBoxLayout();
    m_comboTheme = new QComboBox(container);
    m_comboTheme->addItem("暗夜黑 (默认)", "dark");
    m_comboTheme->addItem("明亮白", "light");
    m_comboTheme->addItem("赛博蓝", "tech");
    m_comboTheme->addItem("樱花粉", "pink");
    m_comboTheme->addItem("深邃紫", "purple");
    m_comboTheme->addItem("森林绿", "green");
    m_comboTheme->addItem("子君粉", "zijunpink");
    m_comboTheme->addItem("子君白", "zijunwhite");
    themeLayout->addWidget(new QLabel("界面主题:", container));
    themeLayout->addWidget(m_comboTheme);
    themeLayout->addStretch();
    mainLayout->addLayout(themeLayout);

    // 快捷键设置
    QLabel *lblHotkey = new QLabel("快捷键设置:", container);
    lblHotkey->setStyleSheet("font-weight: bold; margin-top: 5px;");
    mainLayout->addWidget(lblHotkey);
    
    QHBoxLayout *hotkeyLayout1 = new QHBoxLayout();
    m_hotkeyShowWindow = new HotkeyEdit(container);
    m_hotkeyShowWindow->setFixedWidth(180);
    m_hotkeyShowWindow->setThemeColors(borderColor, textColor, bgColor);
    hotkeyLayout1->addWidget(new QLabel("显示主界面:", container));
    hotkeyLayout1->addWidget(m_hotkeyShowWindow);
    hotkeyLayout1->addStretch();
    mainLayout->addLayout(hotkeyLayout1);
    
    QHBoxLayout *hotkeyLayout2 = new QHBoxLayout();
    m_hotkeyStartRecord = new HotkeyEdit(container);
    m_hotkeyStartRecord->setFixedWidth(180);
    m_hotkeyStartRecord->setThemeColors(borderColor, textColor, bgColor);
    hotkeyLayout2->addWidget(new QLabel("开始/停止录制:", container));
    hotkeyLayout2->addWidget(m_hotkeyStartRecord);
    hotkeyLayout2->addStretch();
    mainLayout->addLayout(hotkeyLayout2);
    
    connect(m_hotkeyShowWindow, &HotkeyEdit::keySequenceChanged, 
            this, &SettingsDialog::onShowWindowHotkeyChanged);
    connect(m_hotkeyStartRecord, &HotkeyEdit::keySequenceChanged, 
            this, &SettingsDialog::onStartRecordHotkeyChanged);

    // 倒计时设置
    QHBoxLayout *countLayout = new QHBoxLayout();
    m_chkCountdown = new QCheckBox("录制前倒计时", container);
    m_spinCountdownSecs = new QSpinBox(container);
    m_spinCountdownSecs->setRange(1, 10);
    m_spinCountdownSecs->setSuffix(" 秒");
    countLayout->addWidget(m_chkCountdown);
    countLayout->addWidget(m_spinCountdownSecs);
    countLayout->addStretch();
    mainLayout->addLayout(countLayout);

    // 最小化托盘
    m_chkMinimizeToTray = new QCheckBox("关闭主窗口时最小化到托盘", container);
    mainLayout->addWidget(m_chkMinimizeToTray);

    mainLayout->addStretch();

    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    
    // 版本号
    QLabel *lblVer = new QLabel(QString("Version: %1").arg(APP_VERSION_STR), container);
    lblVer->setStyleSheet(QString("color: %1; font-size: 10px; font-family: 'Segoe UI', 'Microsoft YaHei'; opacity: 0.5;").arg(textColor));
    // Note: opacity prop in QSS might not work for QLabel text color directly, use rgba if needed.
    // Using color directly is safer. We use textColor which adapts to theme.
    // To make it fainter, we can use an alpha blended color if we parsed hex. 
    // But font-size 10px is subtle enough.
    
    btnLayout->addWidget(lblVer);

    QPushButton *btnSave = new QPushButton("保存", container);
    QPushButton *btnCancel = new QPushButton("取消", container);
    btnSave->setCursor(Qt::PointingHandCursor);
    btnCancel->setCursor(Qt::PointingHandCursor);
    
    btnSave->setObjectName("BtnAction"); 
    btnCancel->setObjectName("BtnCancel");

    connect(btnSave, &QPushButton::clicked, this, &SettingsDialog::onSaveClicked);
    connect(btnCancel, &QPushButton::clicked, this, &SettingsDialog::reject);

    btnLayout->addStretch();
    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnCancel);
    mainLayout->addLayout(btnLayout);
    
    rootLayout->addWidget(contentWidget);

    loadSettings();
    
    // Style applied to container
    QString extraStyle = QString(
        "\n#SettingsContainer { border: 1px solid %1; border-radius: 4px; background-color: %2; }"
        "\n#SettingsContent { background-color: transparent; }"
        "\n#SettingsContainer QWidget { color: %3; }"
        "\n#SettingsContainer QLabel, #SettingsContainer QCheckBox { color: %3; }"
        "\n#SettingsContainer QLineEdit, #SettingsContainer QSpinBox { "
        "  border: 1px solid %1; border-radius: 4px; background-color: transparent; color: %3; padding: 2px; }"
        "\n#SettingsContainer QComboBox { "
        "  border: 1px solid %1; border-radius: 4px; background-color: rgba(128, 128, 128, 0.1); color: %3; padding: 4px; padding-right: 20px; }"
        "\n#SettingsContainer QComboBox:hover { border-color: %3; }"
        "\n#SettingsContainer QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 20px; border-left-width: 0px; border-top-right-radius: 3px; border-bottom-right-radius: 3px; }"
        "\n#SettingsContainer QComboBox::down-arrow { width: 0; height: 0; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 6px solid %3; margin-right: 5px; }"
        "\n#SettingsContainer QPushButton { "
        "  border: 1px solid %1; border-radius: 4px; padding: 4px 12px; color: %3; background-color: transparent; }"
        "\n#SettingsContainer QPushButton:hover { background-color: %1; color: %3; }"
        "\nQCheckBox { spacing: 5px; }"
        "\nQCheckBox::indicator { width: 16px; height: 16px; border: 1px solid %1; border-radius: 3px; background: transparent; }"
        "\nQCheckBox::indicator:checked { background-color: %1; border: 1px solid %1; }" 
        "\n#TitleLabel { color: %3; font-weight: bold; font-size: 14px; }"
    ).arg(borderColor, bgColor, textColor);
    
    container->setStyleSheet(extraStyle);
    
    // Apply ComboBox popup style directly to each combobox's view
    QString viewStyle = QString(
        "QAbstractItemView { background-color: %1; border: 1px solid %2; border-radius: 4px; selection-background-color: %2; outline: none; }"
        "QAbstractItemView::item { color: %3; padding: 6px 10px; min-height: 24px; }"
        "QAbstractItemView::item:selected, QAbstractItemView::item:hover { background-color: %2; color: %3; }"
    ).arg(bgColor, borderColor, textColor);
    
    if (m_comboBitrate && m_comboBitrate->view()) {
        m_comboBitrate->view()->setStyleSheet(viewStyle);
        m_comboBitrate->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
        m_comboBitrate->view()->window()->setAttribute(Qt::WA_TranslucentBackground, false);
    }
    if (m_comboTheme && m_comboTheme->view()) {
        m_comboTheme->view()->setStyleSheet(viewStyle);
        m_comboTheme->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
        m_comboTheme->view()->window()->setAttribute(Qt::WA_TranslucentBackground, false);
    }
    
    // Ensure overlay is sized correctly initially
    if (m_themeOverlay) m_themeOverlay->resize(450, 480); // Approximate inner size
}

void SettingsDialog::resizeEvent(QResizeEvent *event) {
    if (m_themeOverlay) {
        // Find container
        QWidget *container = findChild<QWidget*>("SettingsContainer");
        if (container) m_themeOverlay->resize(container->size());
    }
    QDialog::resizeEvent(event);
}

void SettingsDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_titleBar->geometry().contains(event->pos())) {
        m_isDragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}
void SettingsDialog::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}
void SettingsDialog::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) m_isDragging = false;
}

void SettingsDialog::onBrowseClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "选择保存路径", m_editPath->text());
    if (!dir.isEmpty()) {
        m_editPath->setText(dir);
    }
}

void SettingsDialog::onSaveClicked() {
    saveSettings();
    accept();
}

void SettingsDialog::loadSettings() {
    QSettings settings("KSO", "MScreenRecord");
    m_editPath->setText(settings.value("savePath", QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)).toString());
    m_spinFps->setValue(settings.value("fps", 30).toInt());
    m_comboBitrate->setCurrentIndex(settings.value("bitrateLevel", 1).toInt());
    m_chkMinimizeToTray->setChecked(settings.value("minimizeToTray", true).toBool());
    
    m_chkCountdown->setChecked(settings.value("countdownEnabled", true).toBool());
    m_spinCountdownSecs->setValue(settings.value("countdownSecs", 3).toInt());

    QString theme = settings.value("theme", "dark").toString();
    int themeIdx = m_comboTheme->findData(theme);
    if (themeIdx >= 0) m_comboTheme->setCurrentIndex(themeIdx);
    
    // 加载快捷键设置
    QString showWindowKey = settings.value("hotkeyShowWindow", "Ctrl+Alt+S").toString();
    QString startRecordKey = settings.value("hotkeyStartRecord", "Ctrl+Alt+O").toString();
    
    if (!showWindowKey.isEmpty()) {
        m_hotkeyShowWindow->setKeySequence(QKeySequence(showWindowKey));
    }
    if (!startRecordKey.isEmpty()) {
        m_hotkeyStartRecord->setKeySequence(QKeySequence(startRecordKey));
    }
}

void SettingsDialog::saveSettings() {
    QSettings settings("KSO", "MScreenRecord");
    settings.setValue("savePath", m_editPath->text());
    settings.setValue("fps", m_spinFps->value());
    settings.setValue("bitrateLevel", m_comboBitrate->currentIndex());
    settings.setValue("minimizeToTray", m_chkMinimizeToTray->isChecked());
    settings.setValue("theme", m_comboTheme->currentData().toString());
    
    settings.setValue("countdownEnabled", m_chkCountdown->isChecked());
    settings.setValue("countdownSecs", m_spinCountdownSecs->value());
    
    // 保存快捷键设置
    settings.setValue("hotkeyShowWindow", m_hotkeyShowWindow->keySequence().toString());
    settings.setValue("hotkeyStartRecord", m_hotkeyStartRecord->keySequence().toString());
    
    emit hotkeyChanged();
}

QString SettingsDialog::getSavePath() {
    QSettings settings("KSO", "MScreenRecord");
    return settings.value("savePath", QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)).toString();
}

bool SettingsDialog::getMinimizeToTray() {
    QSettings settings("KSO", "MScreenRecord");
    return settings.value("minimizeToTray", true).toBool();
}

QString SettingsDialog::getTheme() {
    QSettings settings("KSO", "MScreenRecord");
    return settings.value("theme", "dark").toString();
}

QKeySequence SettingsDialog::getShowWindowHotkey() {
    QSettings settings("KSO", "MScreenRecord");
    return QKeySequence(settings.value("hotkeyShowWindow", "Ctrl+Alt+S").toString());
}

QKeySequence SettingsDialog::getStartRecordHotkey() {
    QSettings settings("KSO", "MScreenRecord");
    return QKeySequence(settings.value("hotkeyStartRecord", "Ctrl+Alt+R").toString());
}

void SettingsDialog::onShowWindowHotkeyChanged(const QKeySequence &seq) {
    if (!seq.isEmpty()) {
        checkHotkeyConflict(seq, m_hotkeyShowWindow);
    }
}

void SettingsDialog::onStartRecordHotkeyChanged(const QKeySequence &seq) {
    if (!seq.isEmpty()) {
        checkHotkeyConflict(seq, m_hotkeyStartRecord);
    }
}

bool SettingsDialog::checkHotkeyConflict(const QKeySequence &seq, HotkeyEdit *sourceEdit) {
    if (seq.isEmpty()) return true;
    
    // 检查是否与另一个快捷键冲突
    HotkeyEdit *otherEdit = (sourceEdit == m_hotkeyShowWindow) ? m_hotkeyStartRecord : m_hotkeyShowWindow;
    if (otherEdit && otherEdit->keySequence() == seq) {
        ToastTip::warning(this, QString("快捷键 \"%1\" 已被其他功能使用").arg(seq.toString(QKeySequence::NativeText)));
        sourceEdit->clear();
        return false;
    }
    
    // 检查系统级别冲突
    GlobalHotkey *hotkey = GlobalHotkey::instance();
    quint32 mods = sourceEdit->getModifiers();
    quint32 vk = sourceEdit->getVirtualKey();
    
    if (!hotkey->isHotkeyAvailable(mods, vk)) {
        // 快捷键被其他应用占用
        int ret = CustomMessageBox::question(this, "快捷键冲突",
            QString("快捷键 \"%1\" 已被其他应用程序占用。\n是否强制使用此快捷键？").arg(seq.toString(QKeySequence::NativeText)));
        
        if (ret != 0) {
            // 用户取消
            sourceEdit->clear();
            return false;
        }
        // 用户选择强制使用，将在保存时注册
    }
    
    return true;
}
