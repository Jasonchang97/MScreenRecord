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

SelectionOverlay::SelectionOverlay(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool),
      m_showToolbar(false),
      m_isRecording(false),
      m_isDraggingToolbar(false),
      m_durationText("00:00"),
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
    
    // 节流定时器，用于优化窗口检测的响应
    m_updateThrottle = new QTimer(this);
    m_updateThrottle->setInterval(16); // ~60fps
    m_updateThrottle->setSingleShot(true);
    connect(m_updateThrottle, &QTimer::timeout, this, [this]() {
        if (m_pendingUpdate && !m_showToolbar && !m_isRecording) {
            QRect newHovered = getWindowAtPoint(m_lastMousePos);
            if (newHovered != m_hoveredWindow) {
                m_hoveredWindow = newHovered;
                repaint(); // 使用 repaint 立即刷新
            }
            m_pendingUpdate = false;
        }
    });
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
    
    // 录制时设置鼠标穿透，允许用户操作录制范围内的内容
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    
    update();
}

void SelectionOverlay::stopRecording() {
    m_isRecording = false;
    m_isCountingDown = false;
    m_showToolbar = false;
    m_selection = QRect();
    m_hoveredWindow = QRect();
    m_countdownTimer->stop();
    
    // 恢复鼠标事件处理
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    
    hide();
}

void SelectionOverlay::updateDuration(const QString &duration) {
    m_durationText = duration;
    if (isVisible()) update();
}

void SelectionOverlay::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    detectWindows();
    m_selection = QRect();
    m_showToolbar = false;
    m_isRecording = false;
    m_isCountingDown = false;
    
    // 立即检测鼠标下的窗口进行预框选
    QPoint mousePos = QCursor::pos();
    m_hoveredWindow = getWindowAtPoint(mousePos);
    update();
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
    
    // 没有窗口，返回全屏
    QScreen *screen = QGuiApplication::screenAt(pos);
    if (screen) {
        return screen->geometry();
    }
    return QGuiApplication::primaryScreen()->geometry();
}

void SelectionOverlay::updateToolbarPosition() {
    int toolbarW = 160;
    int toolbarH = 50;
    int margin = 10;
    
    QRect screenGeom = QGuiApplication::primaryScreen()->geometry();
    
    int x = m_selection.right() - toolbarW;
    int y = m_selection.bottom() + margin;
    
    if (y + toolbarH > screenGeom.bottom()) {
        y = m_selection.top() - toolbarH - margin;
        if (y < screenGeom.top()) {
            x = m_selection.right() - toolbarW - margin;
            y = m_selection.bottom() - toolbarH - margin;
        }
    }
    
    if (x + toolbarW > screenGeom.right()) x = screenGeom.right() - toolbarW - margin;
    if (x < screenGeom.left()) x = screenGeom.left() + margin;
    if (y + toolbarH > screenGeom.bottom()) y = screenGeom.bottom() - toolbarH - margin;
    if (y < screenGeom.top()) y = screenGeom.top() + margin;
    
    m_toolbarRect = QRect(x, y, toolbarW, toolbarH);
    
    int btnSize = 36;
    m_buttonRect = QRect(
        m_toolbarRect.right() - btnSize - 10,
        m_toolbarRect.top() + (toolbarH - btnSize) / 2,
        btnSize, btnSize
    );
}

bool SelectionOverlay::isOverToolbar(const QPoint &pos) const {
    return m_showToolbar && m_toolbarRect.contains(pos);
}

bool SelectionOverlay::isOverButton(const QPoint &pos) const {
    return m_showToolbar && m_buttonRect.contains(pos);
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

    // 确定要高亮的区域
    QRect highlightRect = m_selection.isNull() ? m_hoveredWindow : m_selection;
    
    // 录制时只绘制边框，不绘制遮罩（鼠标穿透状态）
    if (m_isRecording) {
        // 只绘制录制区域的红色边框
        if (!highlightRect.isNull()) {
            painter.setPen(QPen(QColor(220, 50, 50), 3, Qt::SolidLine));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(highlightRect);
        }
        return; // 录制时不绘制其他内容
    }
    
    // 非录制状态：绘制整个屏幕的半透明遮罩
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 100));
    painter.drawRect(rect());
    
    // 高亮区域：用更透明的颜色覆盖（不完全透明以确保能接收鼠标事件）
    if (!highlightRect.isNull()) {
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.setBrush(Qt::transparent);
        painter.drawRect(highlightRect);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        // 绘制一个几乎透明的背景，确保鼠标事件不穿透
        painter.setBrush(QColor(0, 0, 0, 1));
        painter.drawRect(highlightRect);
    }

    // 绘制高亮边框
    if (!highlightRect.isNull()) {
        QColor borderColor;
        int penWidth;
        
        if (m_isRecording) {
            borderColor = QColor(220, 50, 50);
            penWidth = 3;
        } else if (!m_selection.isNull()) {
            borderColor = QColor(0, 180, 80);
            penWidth = 3;
        } else {
            borderColor = QColor(0, 150, 255);
            penWidth = 2;
        }
        
        painter.setPen(QPen(borderColor, penWidth, Qt::SolidLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(highlightRect);
        
        // 尺寸标签（录制时不显示）
        if (!m_isCountingDown && !m_isRecording) {
            QString sizeText = QString("%1 × %2").arg(highlightRect.width()).arg(highlightRect.height());
            
            QFont font = painter.font();
            font.setPixelSize(12);
            font.setBold(true);
            painter.setFont(font);
            
            int labelW = painter.fontMetrics().horizontalAdvance(sizeText) + 16;
            int labelH = 22;
            
            // 判断标签位置：优先左上角外侧，空间不够则放内侧
            QRect textBg;
            if (highlightRect.top() - labelH - 4 >= 0) {
                // 外侧（上方）
                textBg = QRect(highlightRect.left(), highlightRect.top() - labelH - 4, labelW, labelH);
            } else {
                // 内侧（框内左上角）
                textBg = QRect(highlightRect.left() + 4, highlightRect.top() + 4, labelW, labelH);
            }
            
            painter.setPen(Qt::NoPen);
            painter.setBrush(m_selection.isNull() ? QColor(0, 120, 215, 220) : QColor(0, 150, 80, 220));
            painter.drawRoundedRect(textBg, 4, 4);
            
            painter.setPen(Qt::white);
            painter.drawText(textBg, Qt::AlignCenter, sizeText);
        }
    }
    
    // 提示文字（预框选状态）
    if (m_selection.isNull() && !m_isCountingDown && !m_isRecording) {
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
    
    // 倒计时
    if (m_isCountingDown && !m_selection.isNull()) {
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
    
    // 悬浮工具栏（确认选择后显示）
    if (m_showToolbar && !m_selection.isNull() && !m_isCountingDown) {
        painter.setBrush(QColor(30, 30, 30, 240));
        painter.setPen(QPen(QColor(80, 80, 80), 1));
        painter.drawRoundedRect(m_toolbarRect, 8, 8);
        
        QFont font = painter.font();
        font.setPixelSize(14);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(Qt::white);
        
        QRect textRect = m_toolbarRect.adjusted(15, 0, -50, 0);
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, m_durationText);
        
        QPoint mousePos = mapFromGlobal(QCursor::pos());
        bool hoverBtn = m_buttonRect.contains(mousePos);
        
        if (m_isRecording) {
            painter.setBrush(hoverBtn ? QColor(255, 80, 80) : QColor(220, 50, 50));
        } else {
            painter.setBrush(hoverBtn ? QColor(80, 220, 80) : QColor(50, 180, 50));
        }
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(m_buttonRect);
        
        painter.setBrush(Qt::white);
        if (m_isRecording) {
            QRect stopIcon = m_buttonRect.adjusted(12, 12, -12, -12);
            painter.drawRoundedRect(stopIcon, 2, 2);
        } else {
            QPainterPath playPath;
            int cx = m_buttonRect.center().x();
            int cy = m_buttonRect.center().y();
            playPath.moveTo(cx - 4, cy - 7);
            playPath.lineTo(cx + 7, cy);
            playPath.lineTo(cx - 4, cy + 7);
            playPath.closeSubpath();
            painter.drawPath(playPath);
        }
        
        font.setPixelSize(10);
        font.setBold(false);
        painter.setFont(font);
        painter.setPen(QColor(180, 180, 180));
        QString hint = m_isRecording ? "点击结束" : "点击开始";
        
        if (m_toolbarRect.top() > m_selection.bottom()) {
            painter.drawText(m_toolbarRect.adjusted(0, m_toolbarRect.height() + 3, 0, 15), Qt::AlignCenter, hint);
        } else {
            painter.drawText(QRect(m_toolbarRect.left(), m_toolbarRect.top() - 18, m_toolbarRect.width(), 15), Qt::AlignCenter, hint);
        }
    }
}

void SelectionOverlay::mousePressEvent(QMouseEvent *event) {
    QPoint pos = event->pos();
    
    if (m_isCountingDown) return;
    
    // 左键
    if (event->button() == Qt::LeftButton) {
        // 工具栏拖动
        if (m_showToolbar && m_toolbarRect.contains(pos) && !m_buttonRect.contains(pos)) {
            m_isDraggingToolbar = true;
            m_toolbarDragPos = pos - m_toolbarRect.topLeft();
            setCursor(Qt::SizeAllCursor);
            return;
        }
        
        // 点击工具栏按钮
        if (m_showToolbar && m_buttonRect.contains(pos)) {
            if (m_isRecording) {
                emit requestStopRecording();
            } else {
                startCountdown();
            }
            return;
        }
        
        // 预框选状态下左键确认
        if (!m_showToolbar && !m_isRecording) {
            m_selection = m_hoveredWindow;
            if (!m_selection.isNull()) {
                m_showToolbar = true;
                updateToolbarPosition();
                emit areaSelected(m_selection);
            }
            update();
        }
    }
    // 右键取消
    else if (event->button() == Qt::RightButton) {
        if (!m_isRecording) {
            emit cancelled();
            hide();
        }
    }
}

void SelectionOverlay::mouseMoveEvent(QMouseEvent *event) {
    QPoint pos = event->pos();
    
    if (m_isCountingDown) return;
    
    // 拖动工具栏
    if (m_isDraggingToolbar) {
        m_toolbarRect.moveTopLeft(pos - m_toolbarDragPos);
        int btnSize = 36;
        m_buttonRect = QRect(
            m_toolbarRect.right() - btnSize - 10,
            m_toolbarRect.top() + (m_toolbarRect.height() - btnSize) / 2,
            btnSize, btnSize
        );
        repaint();
        return;
    }
    
    // 预框选状态：实时更新悬停窗口
    if (!m_showToolbar && !m_isRecording) {
        m_lastMousePos = pos;
        // 直接检测，如果变化了立即刷新
        QRect newHovered = getWindowAtPoint(pos);
        if (newHovered != m_hoveredWindow) {
            m_hoveredWindow = newHovered;
            repaint();
        }
    }
    
    // 鼠标样式
    if (m_showToolbar && m_toolbarRect.contains(pos)) {
        setCursor(m_buttonRect.contains(pos) ? Qt::PointingHandCursor : Qt::SizeAllCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
    
    if (m_showToolbar) {
        repaint();
    }
}

void SelectionOverlay::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_isDraggingToolbar) {
        m_isDraggingToolbar = false;
        setCursor(Qt::ArrowCursor);
    }
}

void SelectionOverlay::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        if (m_isCountingDown) {
            m_countdownTimer->stop();
            m_isCountingDown = false;
            update();
        } else if (m_isRecording) {
            emit requestStopRecording();
        } else {
            emit cancelled();
            hide();
        }
    }
}
