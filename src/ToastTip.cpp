#include "ToastTip.h"
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QScreen>
#include <QPainter>

ToastTip::ToastTip(QWidget *parent, const QString &message, IconType type, int durationMs)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::BypassWindowManagerHint)
    , m_durationMs(durationMs)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);
    
    // 主布局
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(10);
    
    // 图标
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(20, 20);
    
    // 绘制图标
    QPixmap iconPix(20, 20);
    iconPix.fill(Qt::transparent);
    QPainter p(&iconPix);
    p.setRenderHint(QPainter::Antialiasing);
    
    QColor iconColor;
    QString bgColor;
    switch (type) {
        case Success:
            iconColor = QColor("#52c41a");
            bgColor = "rgba(40, 40, 40, 240)";
            // 绘制勾选
            p.setPen(QPen(iconColor, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            p.drawLine(4, 10, 8, 14);
            p.drawLine(8, 14, 16, 5);
            break;
        case Warning:
            iconColor = QColor("#faad14");
            bgColor = "rgba(40, 40, 40, 240)";
            // 绘制感叹号
            p.setPen(QPen(iconColor, 2.5, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(10, 4, 10, 11);
            p.setBrush(iconColor);
            p.drawEllipse(8, 14, 4, 4);
            break;
        case Error:
            iconColor = QColor("#ff4d4f");
            bgColor = "rgba(40, 40, 40, 240)";
            // 绘制 X
            p.setPen(QPen(iconColor, 2.5, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(4, 4, 16, 16);
            p.drawLine(16, 4, 4, 16);
            break;
        case Info:
        default:
            iconColor = QColor("#1890ff");
            bgColor = "rgba(40, 40, 40, 240)";
            // 绘制 i
            p.setPen(QPen(iconColor, 2.5, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(10, 7, 10, 14);
            p.setBrush(iconColor);
            p.drawEllipse(8, 3, 4, 4);
            break;
    }
    p.end();
    m_iconLabel->setPixmap(iconPix);
    
    // 消息文本
    m_msgLabel = new QLabel(message, this);
    m_msgLabel->setStyleSheet("color: #ffffff; font-size: 13px;");
    m_msgLabel->setWordWrap(false);
    
    layout->addWidget(m_iconLabel);
    layout->addWidget(m_msgLabel);
    
    // 样式
    setStyleSheet(QString(
        "ToastTip { background: %1; border-radius: 8px; }"
    ).arg(bgColor));
    
    // 阴影效果
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 80));
    setGraphicsEffect(shadow);
    
    // 定时器
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &ToastTip::fadeOut);
    
    // 淡出动画
    m_fadeAnim = new QPropertyAnimation(this, "opacity", this);
    m_fadeAnim->setDuration(300);
    m_fadeAnim->setStartValue(1.0);
    m_fadeAnim->setEndValue(0.0);
    connect(m_fadeAnim, &QPropertyAnimation::finished, this, &ToastTip::close);
    
    adjustSize();
}

void ToastTip::showToast() {
    // 计算位置：父窗口顶部居中
    QWidget *p = parentWidget();
    QPoint pos;
    
    if (p) {
        int x = p->x() + (p->width() - width()) / 2;
        int y = p->y() + 60;
        pos = QPoint(x, y);
    } else {
        QScreen *screen = QApplication::primaryScreen();
        if (screen) {
            QRect screenGeom = screen->availableGeometry();
            pos = QPoint(screenGeom.x() + (screenGeom.width() - width()) / 2, 
                        screenGeom.y() + 80);
        }
    }
    
    move(pos);
    setWindowOpacity(0.0);
    QWidget::show();
    
    // 淡入动画
    QPropertyAnimation *fadeIn = new QPropertyAnimation(this, "opacity", this);
    fadeIn->setDuration(200);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    
    m_timer->start(m_durationMs);
}

void ToastTip::fadeOut() {
    m_fadeAnim->start();
}

void ToastTip::show(QWidget *parent, const QString &message, IconType type, int durationMs) {
    ToastTip *toast = new ToastTip(parent, message, type, durationMs);
    toast->showToast();
}

void ToastTip::info(QWidget *parent, const QString &message, int durationMs) {
    show(parent, message, Info, durationMs);
}

void ToastTip::success(QWidget *parent, const QString &message, int durationMs) {
    show(parent, message, Success, durationMs);
}

void ToastTip::warning(QWidget *parent, const QString &message, int durationMs) {
    show(parent, message, Warning, durationMs);
}

void ToastTip::error(QWidget *parent, const QString &message, int durationMs) {
    show(parent, message, Error, durationMs);
}
