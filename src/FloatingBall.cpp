#include "FloatingBall.h"
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QPropertyAnimation>
#include <QMenu>
#include <QDebug>

FloatingBall::FloatingBall(QWidget *parent)
    : QWidget(parent), m_isDragging(false), m_isRightSide(false), m_isRecording(false), m_isHovered(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    
    // Initial: Thinner Collapsed Toolbar
    setFixedSize(12, 70); 
    setMouseTracking(true);
    
    QRect screenGeom = QApplication::primaryScreen()->availableGeometry();
    // Default dock right
    move(screenGeom.right() - width(), screenGeom.height() / 2 - 35);
    m_isRightSide = true;
    m_durationText = "00:00:00";
}

void FloatingBall::updateDuration(const QString& duration) {
    m_durationText = duration;
    if (isVisible()) update();
}

void FloatingBall::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    QColor bgColor = QColor(40, 40, 40, 230);
    if (m_isRecording) bgColor = QColor(220, 50, 50, 230);

    if (m_isHovered) {
        // --- Expanded Mode ---
        // Background
        p.setBrush(QColor(30, 30, 30, 240));
        p.setPen(QPen(QColor(60, 60, 60), 1));
        p.drawRoundedRect(0, 0, width(), height(), 8, 8);
        
        // Duration Text
        p.setPen(Qt::white);
        QFont font = p.font();
        font.setPixelSize(14);
        font.setBold(true);
        p.setFont(font);
        QRect textRect(0, 8, width(), 20);
        p.drawText(textRect, Qt::AlignCenter, m_durationText);
        
        // Single Button (Stop/Record) Centered
        int btnW = 32;
        int btnH = 32;
        int startX = (width() - btnW) / 2;
        int startY = 30;
        
        QRect stopRect(startX, startY, btnW, btnH);
        
        // Draw hover effect on button
        QPoint mousePos = mapFromGlobal(QCursor::pos());
        if (stopRect.contains(mousePos)) {
            p.setBrush(m_isRecording ? QColor(255, 80, 80) : QColor(80, 200, 80)); // Lighter Red/Green
        } else {
            p.setBrush(m_isRecording ? Qt::red : QColor(0, 180, 0)); // Red / Green
        }
        
        p.setPen(Qt::NoPen);
        p.drawEllipse(stopRect);
        
        p.setBrush(Qt::white);
        if (m_isRecording) {
            p.drawRoundedRect(stopRect.adjusted(10, 10, -10, -10), 2, 2); // Square Stop Icon
        } else {
            // Play Icon (Triangle)
             QPainterPath path;
             int cx = stopRect.center().x();
             int cy = stopRect.center().y();
             // Triangle pointing right
             path.moveTo(cx - 3, cy - 5);
             path.lineTo(cx + 5, cy);
             path.lineTo(cx - 3, cy + 5);
             path.closeSubpath();
             p.drawPath(path);
        }
        
    } else {
        // --- Collapsed Mode (Thin Bar) ---
        p.setBrush(bgColor);
        p.setPen(Qt::NoPen);
        
        if (m_isRightSide) {
            // Draw on Right edge
            p.drawRoundedRect(0, 0, width() + 5, height(), 6, 6);
            // Simple Grip
            p.setPen(QPen(Qt::white, 2));
            p.drawLine(4, height()/2 - 6, 4, height()/2 + 6);
        } else {
            // Draw on Left edge
            p.drawRoundedRect(-5, 0, width() + 5, height(), 6, 6);
            // Simple Grip
            p.setPen(QPen(Qt::white, 2));
            p.drawLine(width() - 4, height()/2 - 6, width() - 4, height()/2 + 6);
        }
    }
}

void FloatingBall::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragPos = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    } else if (event->button() == Qt::RightButton) {
        QMenu menu;
        menu.addAction("显示主界面", this, &FloatingBall::requestShowMainWindow);
        menu.addAction("退出程序", this, &FloatingBall::requestExit);
        menu.exec(event->globalPos());
    }
}

void FloatingBall::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        QPoint newPos = event->globalPos() - m_dragPos;
        
        // Bounds check
        QRect screenGeom = QApplication::primaryScreen()->availableGeometry();
        int x = qBound(screenGeom.left(), newPos.x(), screenGeom.right() - width());
        int y = qBound(screenGeom.top(), newPos.y(), screenGeom.bottom() - height());
        
        move(x, y);
        event->accept();
    }
}

void FloatingBall::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        bool wasDragging = m_isDragging && (event->globalPos() - (pos() + m_dragPos)).manhattanLength() > 5;
        m_isDragging = false;

        if (!wasDragging && m_isHovered) {
             // Check if clicked on button area
             // Button area in expanded mode is roughly centered bottom
             if (width() > 50) { // Expanded
                 int btnW = 32; int btnH = 32;
                 int startX = (width() - btnW) / 2;
                 int startY = 30;
                 QRect btnRect(startX, startY, btnW, btnH);
                 
                 if (btnRect.contains(event->pos())) {
                     if (m_isRecording) emit requestStopRecording();
                     else emit requestStartRecording();
                     return;
                 }
             }
        } else {
            dockToEdge();
        }
    }
}

void FloatingBall::dockToEdge()
{
    QRect screenGeom = QApplication::primaryScreen()->availableGeometry();
    int centerX = x() + width() / 2;
    int screenCenterX = screenGeom.center().x();
    
    int targetX;
    if (centerX < screenCenterX) {
        targetX = screenGeom.left();
        m_isRightSide = false;
    } else {
        targetX = screenGeom.right() - width();
        m_isRightSide = true;
    }
    
    QPropertyAnimation *anim = new QPropertyAnimation(this, "pos");
    anim->setDuration(200);
    anim->setStartValue(pos());
    anim->setEndValue(QPoint(targetX, y()));
    anim->setEasingCurve(QEasingCurve::OutQuad);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    
    update();
}

void FloatingBall::enterEvent(QEvent *)
{
    if (m_isDragging) return;
    m_isHovered = true;
    
    // Expand: 12 -> 110
    int oldW = width();
    int newW = 110;
    int diff = newW - oldW;
    
    setFixedSize(newW, 70); 
    
    if (m_isRightSide) {
        move(x() - diff, y());
        m_dragPos.setX(m_dragPos.x() + diff); 
    }
    update();
}

void FloatingBall::leaveEvent(QEvent *)
{
    if (m_isDragging) return;
    m_isHovered = false;
    
    int oldW = width();
    int newW = 12;
    int diff = oldW - newW;
    
    if (m_isRightSide) {
        move(x() + diff, y());
    }
    
    setFixedSize(newW, 70);
    update();
}

void FloatingBall::setRecordingState(bool recording)
{
    m_isRecording = recording;
    update();
}
