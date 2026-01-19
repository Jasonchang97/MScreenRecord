#include "CountdownOverlay.h"
#include <QApplication>
#include <QScreen>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QRadialGradient>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

CountdownOverlay::CountdownOverlay(QWidget *parent)
    : QWidget(parent), m_countdown(0), m_isActive(false)
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating, false);
    setAttribute(Qt::WA_TransparentForMouseEvents, true); // Enable mouse passthrough
    setAttribute(Qt::WA_NoSystemBackground, true); // Improve transparency performance
    
    // Cover all screens
    QRect totalGeometry;
    QList<QScreen*> screens = QApplication::screens();
    for (QScreen *screen : screens) {
        totalGeometry = totalGeometry.united(screen->geometry());
    }
    setGeometry(totalGeometry);
    
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    m_timer->setInterval(1000); // 1 second
    connect(m_timer, &QTimer::timeout, this, &CountdownOverlay::onTimerTick);
    
    m_scaleAnimation = new QPropertyAnimation(this, "countdown", this);
    m_fadeAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    
    hide();
}

CountdownOverlay::~CountdownOverlay() {
    stopCountdown();
}

void CountdownOverlay::startCountdown(int seconds) {
    if (m_isActive) return;
    
    m_countdown = seconds;
    m_isActive = true;
    
    // Show overlay (mouse passthrough will be set in showEvent)
    show();
    raise();
    activateWindow();
    
    // Start timer
    m_timer->start();
    
    // Initial update
    update();
}

void CountdownOverlay::stopCountdown() {
    if (!m_isActive) return;
    
    m_timer->stop();
    m_isActive = false;
    m_countdown = 0;
    
#ifdef Q_OS_WIN
    // Remove WS_EX_TRANSPARENT extended style when hiding
    HWND hwnd = (HWND)winId();
    if (hwnd) {
        LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
    }
#endif
    
    hide();
}

void CountdownOverlay::onTimerTick() {
    m_countdown--;
    update();
    
    if (m_countdown <= 0) {
        m_timer->stop();
        m_isActive = false;
        emit countdownFinished();
        hide();
    }
}

void CountdownOverlay::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Very transparent dark background (just for subtle hint)
    painter.fillRect(rect(), QColor(0, 0, 0, 60));
    
    if (m_countdown > 0) {
        // Draw countdown number (smaller size for hint only)
        QFont font;
        font.setPointSize(120);
        font.setBold(true);
        painter.setFont(font);
        
        QString text = QString::number(m_countdown);
        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(text);
        
        // Calculate circle size - smaller for hint only
        int circleSize = qMax(textRect.width(), textRect.height()) + 50; // Smaller padding
        QRect circleRect(0, 0, circleSize, circleSize);
        circleRect.moveCenter(rect().center());
        
        // Draw shadow (offset circle)
        QRect shadowRect = circleRect;
        shadowRect.translate(6, 6);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 80));
        painter.drawEllipse(shadowRect);
        
        // Draw main circle with gradient (more transparent)
        QRadialGradient gradient(circleRect.center(), circleRect.width() / 2);
        gradient.setColorAt(0, QColor(255, 80, 80, 200));
        gradient.setColorAt(1, QColor(200, 40, 40, 200));
        painter.setPen(QPen(QColor(255, 255, 255, 230), 3));
        painter.setBrush(gradient);
        painter.drawEllipse(circleRect);
        
        // Draw number text
        textRect.moveCenter(circleRect.center());
        painter.setPen(QColor(255, 255, 255, 255));
        painter.drawText(textRect, Qt::AlignCenter, text);
    }
}

void CountdownOverlay::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    
#ifdef Q_OS_WIN
    // Set WS_EX_TRANSPARENT extended style for true mouse passthrough
    // This must be done after the window is shown to ensure HWND exists
    HWND hwnd = (HWND)winId();
    if (hwnd) {
        LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
    }
#endif
}

void CountdownOverlay::keyPressEvent(QKeyEvent *event) {
    // Only ESC key can cancel (mouse events are passed through)
    if (event->key() == Qt::Key_Escape) {
        stopCountdown();
        emit countdownCancelled();
    } else {
        QWidget::keyPressEvent(event);
    }
}

bool CountdownOverlay::nativeEvent(const QByteArray &eventType, void *message, long *result) {
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG") {
        MSG *msg = static_cast<MSG*>(message);
        if (msg->message == WM_NCHITTEST) {
            // Make the entire window transparent to mouse events
            // This works in combination with WS_EX_TRANSPARENT
            *result = HTTRANSPARENT;
            return true;
        }
    }
#endif
    return QWidget::nativeEvent(eventType, message, result);
}
