#include "SelectionOverlay.h"
#include <QGuiApplication>
#include <QPainterPath>
#include <QWindow>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

// ================== RecordingToolbar Implementation ==================

RecordingToolbar::RecordingToolbar(QWidget *parent)
    : QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
    , m_isRecording(false)
    , m_durationText("00:00")
    , m_isDragging(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFixedSize(160, 50);
    setMouseTracking(true);
    
    m_buttonRect = QRect(width() - 46, 7, 36, 36);
}

void RecordingToolbar::setRecording(bool recording) {
    m_isRecording = recording;
    update();
}

void RecordingToolbar::setDuration(const QString &duration) {
    m_durationText = duration;
    update();
}

void RecordingToolbar::updatePosition(const QRect &selection) {
    if (selection.isNull()) return;
    
    int margin = 10;
    QRect screenGeom = QGuiApplication::primaryScreen()->geometry();
    
    int x = selection.right() - width();
    int y = selection.bottom() + margin;
    
    // 下方没空间就放上方
    if (y + height() > screenGeom.bottom()) {
        y = selection.top() - height() - margin;
        // 上方也没空间就放内部右下角
        if (y < screenGeom.top()) {
            x = selection.right() - width() - margin;
            y = selection.bottom() - height() - margin;
        }
    }
    
    // 边界检查
    if (x + width() > screenGeom.right()) x = screenGeom.right() - width() - margin;
    if (x < screenGeom.left()) x = screenGeom.left() + margin;
    if (y + height() > screenGeom.bottom()) y = screenGeom.bottom() - height() - margin;
    if (y < screenGeom.top()) y = screenGeom.top() + margin;
    
    move(x, y);
}

void RecordingToolbar::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 背景
    painter.setBrush(QColor(30, 30, 30, 240));
    painter.setPen(QPen(QColor(80, 80, 80), 1));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
    
    // 时长文字
    QFont font = painter.font();
    font.setPixelSize(14);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    
    QRect textRect = rect().adjusted(15, 0, -50, 0);
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, m_durationText);
    
    // 按钮
    QPoint mousePos = mapFromGlobal(QCursor::pos());
    bool hoverBtn = m_buttonRect.contains(mousePos);
    
    if (m_isRecording) {
        painter.setBrush(hoverBtn ? QColor(255, 80, 80) : QColor(220, 50, 50));
    } else {
        painter.setBrush(hoverBtn ? QColor(80, 220, 80) : QColor(50, 180, 50));
    }
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(m_buttonRect);
    
    // 按钮图标
    painter.setBrush(Qt::white);
    if (m_isRecording) {
        // 停止图标（方块）
        QRect stopIcon = m_buttonRect.adjusted(12, 12, -12, -12);
        painter.drawRoundedRect(stopIcon, 2, 2);
    } else {
        // 开始图标（三角形）
        QPainterPath playPath;
        int cx = m_buttonRect.center().x();
        int cy = m_buttonRect.center().y();
        playPath.moveTo(cx - 4, cy - 7);
        playPath.lineTo(cx + 7, cy);
        playPath.lineTo(cx - 4, cy + 7);
        playPath.closeSubpath();
        painter.drawPath(playPath);
    }
    
    // 提示文字
    font.setPixelSize(10);
    font.setBold(false);
    painter.setFont(font);
    painter.setPen(QColor(180, 180, 180));
    QString hint = m_isRecording ? "点击结束 | ESC" : "点击开始";
    painter.drawText(QRect(0, height() + 3, width(), 15), Qt::AlignCenter, hint);
}

void RecordingToolbar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (m_buttonRect.contains(event->pos())) {
            if (m_isRecording) {
                emit stopClicked();
            } else {
                emit startClicked();
            }
        } else {
            m_isDragging = true;
            m_dragPos = event->pos();
            setCursor(Qt::SizeAllCursor);
        }
    }
}

void RecordingToolbar::mouseMoveEvent(QMouseEvent *event) {
    if (m_isDragging) {
        move(event->globalPos() - m_dragPos);
    } else {
        setCursor(m_buttonRect.contains(event->pos()) ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update(); // 更新 hover 状态
    }
}

void RecordingToolbar::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_isDragging) {
        m_isDragging = false;
        setCursor(Qt::ArrowCursor);
    }
}

// ================== SelectionOverlay Implementation ==================

SelectionOverlay::SelectionOverlay(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool),
      m_showToolbar(false),
      m_isRecording(false),
      m_durationText("00:00"),
      m_toolbar(nullptr),
      m_countdownEnabled(true),
      m_countdownSecs(3),
      m_currentCountdown(0),
      m_isCountingDown(false),
      m_countdownScale(1.0),
      m_countdownOpacity(1.0),
      m_scaleAnim(nullptr),
      m_opacityAnim(nullptr),
      m_updateThrottle(nullptr),
      m_pendingUpdate(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::ArrowCursor);
    setMouseTracking(true);
    
    QRect totalGeometry;
    for (QScreen *screen : QGuiApplication::screens()) {
        totalGeometry = totalGeometry.united(screen->geometry());
    }
    setGeometry(totalGeometry);
    
    m_countdownTimer = new QTimer(this);
    m_countdownTimer->setInterval(1000);
    connect(m_countdownTimer, &QTimer::timeout, this, &SelectionOverlay::onCountdownTick);
    
    m_scaleAnim = new QPropertyAnimation(this, "countdownScale", this);
    m_opacityAnim = new QPropertyAnimation(this, "countdownOpacity", this);
    
    // 节流定时器
    m_updateThrottle = new QTimer(this);
    m_updateThrottle->setInterval(16);
    m_updateThrottle->setSingleShot(true);
    connect(m_updateThrottle, &QTimer::timeout, this, [this]() {
        if (m_pendingUpdate && !m_showToolbar && !m_isRecording && !m_isCountingDown) {
            QRect newHovered = getWindowAtPoint(m_lastMousePos);
            if (newHovered != m_hoveredWindow) {
                m_hoveredWindow = newHovered;
                repaint();
            }
            m_pendingUpdate = false;
        }
    });
    
    // 创建独立工具条
    m_toolbar = new RecordingToolbar(this);
    connect(m_toolbar, &RecordingToolbar::startClicked, this, &SelectionOverlay::startCountdown);
    connect(m_toolbar, &RecordingToolbar::stopClicked, this, &SelectionOverlay::requestStopRecording);
}

SelectionOverlay::~SelectionOverlay() {
    if (m_toolbar) {
        m_toolbar->hide();
        delete m_toolbar;
        m_toolbar = nullptr;
    }
}

QRect SelectionOverlay::getSelection() const {
    return m_selection;
}

void SelectionOverlay::setCountdownEnabled(bool enabled, int seconds) {
    m_countdownEnabled = enabled;
    m_countdownSecs = seconds;
}

void SelectionOverlay::startRecording() {
    m_isRecording = true;
    m_isCountingDown = false;
    m_durationText = "00:00";
    
    // 设置鼠标穿透
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    
    // 显示并更新工具条状态
    if (m_toolbar) {
        m_toolbar->setRecording(true);
        m_toolbar->setDuration(m_durationText);
        m_toolbar->updatePosition(m_selection);
        m_toolbar->show();
        m_toolbar->raise();
    }
    
    update();
}

void SelectionOverlay::stopRecording() {
    m_isRecording = false;
    m_isCountingDown = false;
    m_showToolbar = false;
    m_selection = QRect();
    m_hoveredWindow = QRect();
    m_countdownTimer->stop();
    
    // 恢复鼠标事件
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    
    // 隐藏工具条
    if (m_toolbar) {
        m_toolbar->hide();
    }
    
    hide();
}

void SelectionOverlay::updateDuration(const QString &duration) {
    m_durationText = duration;
    if (m_toolbar && m_toolbar->isVisible()) {
        m_toolbar->setDuration(duration);
    }
    if (isVisible()) update();
}

void SelectionOverlay::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    detectWindows();
    m_selection = QRect();
    m_showToolbar = false;
    m_isRecording = false;
    m_isCountingDown = false;
    
    // 恢复鼠标事件
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    
    // 隐藏工具条
    if (m_toolbar) {
        m_toolbar->hide();
    }
    
    // 立即检测鼠标下的窗口
    QPoint mousePos = QCursor::pos();
    m_hoveredWindow = getWindowAtPoint(mousePos);
    update();
}

void SelectionOverlay::hideEvent(QHideEvent *event) {
    QWidget::hideEvent(event);
    if (m_toolbar) {
        m_toolbar->hide();
    }
}

void SelectionOverlay::detectWindows() {
    m_windows.clear();
    
    HWND selfHwnd = nullptr;
    if (QWindow *w = windowHandle()) {
        selfHwnd = (HWND)w->winId();
    }
    
#ifdef Q_OS_WIN
    struct EnumData {
        QList<SelectionOverlay::WindowInfo> *windows;
        HWND selfHwnd;
    };
    EnumData data = { &m_windows, selfHwnd };
    
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        EnumData *pData = reinterpret_cast<EnumData*>(lParam);
        
        if (!IsWindowVisible(hwnd)) return TRUE;
        if (IsIconic(hwnd)) return TRUE;
        if (pData->selfHwnd && hwnd == pData->selfHwnd) return TRUE;
        
        wchar_t title[256] = {0};
        GetWindowTextW(hwnd, title, 256);
        if (wcslen(title) == 0) return TRUE;
        
        wchar_t className[256] = {0};
        GetClassNameW(hwnd, className, 256);
        if (wcscmp(className, L"Progman") == 0 || 
            wcscmp(className, L"WorkerW") == 0 ||
            wcscmp(className, L"Shell_TrayWnd") == 0) {
            return TRUE;
        }
        
        RECT rect;
        if (DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect)) == S_OK) {
            int w = rect.right - rect.left;
            int h = rect.bottom - rect.top;
            if (w > 50 && h > 50) {
                SelectionOverlay::WindowInfo info;
                info.hwnd = (quintptr)hwnd;
                info.rect = QRect(rect.left, rect.top, w, h);
                info.title = QString::fromWCharArray(title);
                pData->windows->append(info);
            }
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&data));
#endif
}

QRect SelectionOverlay::getWindowAtPoint(const QPoint &pos) {
    for (const WindowInfo &info : m_windows) {
        if (info.rect.contains(pos)) {
            return info.rect;
        }
    }
    
    QScreen *screen = QGuiApplication::screenAt(pos);
    if (screen) {
        return screen->geometry();
    }
    return QGuiApplication::primaryScreen()->geometry();
}

void SelectionOverlay::animateCountdownNumber() {
    m_scaleAnim->stop();
    m_scaleAnim->setDuration(300);
    m_scaleAnim->setStartValue(1.5);
    m_scaleAnim->setEndValue(1.0);
    m_scaleAnim->setEasingCurve(QEasingCurve::OutBack);
    m_scaleAnim->start();
    
    m_opacityAnim->stop();
    m_opacityAnim->setDuration(200);
    m_opacityAnim->setStartValue(0.3);
    m_opacityAnim->setEndValue(1.0);
    m_opacityAnim->setEasingCurve(QEasingCurve::OutQuad);
    m_opacityAnim->start();
}

void SelectionOverlay::startCountdown() {
    if (m_countdownEnabled && m_countdownSecs > 0) {
        m_isCountingDown = true;
        m_currentCountdown = m_countdownSecs;
        m_countdownScale = 1.5;
        m_countdownOpacity = 0.3;
        
        // 倒计时时也设置鼠标穿透
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        
        // 隐藏工具条（倒计时期间）
        if (m_toolbar) {
            m_toolbar->hide();
        }
        
        animateCountdownNumber();
        m_countdownTimer->start();
        update();
    } else {
        emit requestStartRecording();
    }
}

void SelectionOverlay::onCountdownTick() {
    m_currentCountdown--;
    if (m_currentCountdown <= 0) {
        m_countdownTimer->stop();
        m_isCountingDown = false;
        emit requestStartRecording();
    } else {
        animateCountdownNumber();
    }
    update();
}

void SelectionOverlay::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect highlightRect = m_selection.isNull() ? m_hoveredWindow : m_selection;
    
    // 录制时只绘制边框
    if (m_isRecording) {
        if (!highlightRect.isNull()) {
            painter.setPen(QPen(QColor(220, 50, 50), 3, Qt::SolidLine));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(highlightRect);
        }
        return;
    }
    
    // 倒计时状态：绘制边框和倒计时
    if (m_isCountingDown && !m_selection.isNull()) {
        // 只绘制边框
        painter.setPen(QPen(QColor(0, 180, 80), 3, Qt::SolidLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(m_selection);
        
        // 绘制倒计时
        painter.save();
        
        QPoint center = m_selection.center();
        painter.translate(center);
        painter.scale(m_countdownScale, m_countdownScale);
        painter.setOpacity(m_countdownOpacity);
        
        int circleRadius = 80;
        QRadialGradient gradient(0, 0, circleRadius);
        gradient.setColorAt(0, QColor(0, 0, 0, 220));
        gradient.setColorAt(0.7, QColor(0, 0, 0, 200));
        gradient.setColorAt(1, QColor(0, 0, 0, 0));
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(gradient);
        painter.drawEllipse(QPoint(0, 0), circleRadius, circleRadius);
        
        painter.setPen(QPen(QColor(0, 180, 80), 6));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QPoint(0, 0), circleRadius - 10, circleRadius - 10);
        
        QFont countFont;
        countFont.setPixelSize(72);
        countFont.setBold(true);
        countFont.setFamily("Arial");
        painter.setFont(countFont);
        
        QString countText = QString::number(m_currentCountdown);
        QRect textRect(-50, -40, 100, 80);
        
        painter.setPen(QColor(0, 220, 100));
        painter.drawText(textRect, Qt::AlignCenter, countText);
        
        painter.restore();
        return;
    }
    
    // 非录制/倒计时状态：绘制遮罩
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 100));
    painter.drawRect(rect());
    
    // 高亮区域
    if (!highlightRect.isNull()) {
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.setBrush(Qt::transparent);
        painter.drawRect(highlightRect);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setBrush(QColor(0, 0, 0, 1));
        painter.drawRect(highlightRect);
    }

    // 绘制高亮边框
    if (!highlightRect.isNull()) {
        QColor borderColor;
        int penWidth;
        
        if (!m_selection.isNull()) {
            borderColor = QColor(0, 180, 80);
            penWidth = 3;
        } else {
            borderColor = QColor(0, 150, 255);
            penWidth = 2;
        }
        
        painter.setPen(QPen(borderColor, penWidth, Qt::SolidLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(highlightRect);
        
        // 尺寸标签
        QString sizeText = QString("%1 × %2").arg(highlightRect.width()).arg(highlightRect.height());
        
        QFont font = painter.font();
        font.setPixelSize(12);
        font.setBold(true);
        painter.setFont(font);
        
        int labelW = painter.fontMetrics().horizontalAdvance(sizeText) + 16;
        int labelH = 22;
        
        QRect textBg;
        if (highlightRect.top() - labelH - 4 >= 0) {
            textBg = QRect(highlightRect.left(), highlightRect.top() - labelH - 4, labelW, labelH);
        } else {
            textBg = QRect(highlightRect.left() + 4, highlightRect.top() + 4, labelW, labelH);
        }
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_selection.isNull() ? QColor(0, 120, 215, 220) : QColor(0, 150, 80, 220));
        painter.drawRoundedRect(textBg, 4, 4);
        
        painter.setPen(Qt::white);
        painter.drawText(textBg, Qt::AlignCenter, sizeText);
    }
    
    // 提示文字（预框选状态）
    if (m_selection.isNull()) {
        QFont font = painter.font();
        font.setPixelSize(13);
        painter.setFont(font);
        
        QString hint = "左键确认  |  右键取消  |  ESC 退出";
        
        QRect bgRect = painter.fontMetrics().boundingRect(hint);
        bgRect.setWidth(bgRect.width() + 40);
        bgRect.setHeight(28);
        bgRect.moveCenter(QPoint(width() / 2, 55));
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 200));
        painter.drawRoundedRect(bgRect, 14, 14);
        
        painter.setPen(Qt::white);
        painter.drawText(bgRect, Qt::AlignCenter, hint);
    }
}

void SelectionOverlay::mousePressEvent(QMouseEvent *event) {
    QPoint pos = event->pos();
    
    if (m_isCountingDown || m_isRecording) return;
    
    if (event->button() == Qt::LeftButton) {
        // 预框选状态下左键确认
        if (!m_showToolbar) {
            m_selection = m_hoveredWindow;
            if (!m_selection.isNull()) {
                m_showToolbar = true;
                
                // 显示独立工具条
                if (m_toolbar) {
                    m_toolbar->setRecording(false);
                    m_toolbar->setDuration("00:00");
                    m_toolbar->updatePosition(m_selection);
                    m_toolbar->show();
                    m_toolbar->raise();
                }
                
                emit areaSelected(m_selection);
            }
            update();
        }
    } else if (event->button() == Qt::RightButton) {
        emit cancelled();
        hide();
    }
}

void SelectionOverlay::mouseMoveEvent(QMouseEvent *event) {
    QPoint pos = event->pos();
    
    if (m_isCountingDown || m_isRecording) return;
    
    // 预框选状态
    if (!m_showToolbar) {
        m_lastMousePos = pos;
        QRect newHovered = getWindowAtPoint(pos);
        if (newHovered != m_hoveredWindow) {
            m_hoveredWindow = newHovered;
            repaint();
        }
    }
    
    setCursor(Qt::ArrowCursor);
}

void SelectionOverlay::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
}

void SelectionOverlay::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        if (m_isCountingDown) {
            m_countdownTimer->stop();
            m_isCountingDown = false;
            
            // 恢复鼠标事件
            setAttribute(Qt::WA_TransparentForMouseEvents, false);
            
            // 恢复工具条
            if (m_toolbar && !m_selection.isNull()) {
                m_toolbar->show();
                m_toolbar->raise();
            }
            
            update();
        } else if (m_isRecording) {
            emit requestStopRecording();
        } else {
            emit cancelled();
            hide();
        }
    }
}
