#include "ToastTip.h"
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QPainterPath>
#include <QSettings>

ToastTip::ToastTip(QWidget *parent, const QString &message, IconType type, int durationMs)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::BypassWindowManagerHint)
    , m_durationMs(durationMs)
    , m_type(type)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);
    
    // 读取当前皮肤设置
    QSettings settings("KSO", "MScreenRecord");
    QString theme = settings.value("theme", "dark").toString();
    
    // 根据皮肤设置背景和文字颜色
    if (theme == "light") {
        m_bgColor = QColor(255, 255, 255, 245);
        m_textColor = QColor(30, 30, 30);
        m_borderColor = QColor(220, 220, 220);
    } else {
        m_bgColor = QColor(50, 50, 50, 245);
        m_textColor = QColor(240, 240, 240);
        m_borderColor = QColor(80, 80, 80);
    }
    
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
    switch (type) {
        case Success:
            iconColor = QColor("#52c41a");
            // 绘制勾选
            p.setPen(QPen(iconColor, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            p.drawLine(4, 10, 8, 14);
            p.drawLine(8, 14, 16, 5);
            break;
        case Warning:
            iconColor = QColor("#faad14");
            // 绘制感叹号
            p.setPen(QPen(iconColor, 2.5, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(10, 4, 10, 11);
            p.setBrush(iconColor);
            p.drawEllipse(8, 14, 4, 4);
            break;
        case Error:
            iconColor = QColor("#ff4d4f");
            // 绘制 X
            p.setPen(QPen(iconColor, 2.5, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(4, 4, 16, 16);
            p.drawLine(16, 4, 4, 16);
            break;
        case Info:
        default:
            iconColor = QColor("#1890ff");
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
    m_msgLabel->setStyleSheet(QString("color: %1; font-size: 13px; background: transparent;").arg(m_textColor.name()));
    m_msgLabel->setWordWrap(false);
    
    layout->addWidget(m_iconLabel);
    layout->addWidget(m_msgLabel);
    
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

void ToastTip::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(2, 2, -2, -2), 8, 8);
    
    // 背景填充
    painter.fillPath(path, m_bgColor);
    
    // 绘制边框
    painter.setPen(QPen(m_borderColor, 1));
    painter.drawPath(path);
    
    // 绘制左侧颜色指示条
    QColor indicatorColor;
    switch (m_type) {
        case Success: indicatorColor = QColor("#52c41a"); break;
        case Warning: indicatorColor = QColor("#faad14"); break;
        case Error:   indicatorColor = QColor("#ff4d4f"); break;
        case Info:
        default:      indicatorColor = QColor("#1890ff"); break;
    }
    
    QPainterPath indicatorPath;
    indicatorPath.addRoundedRect(QRect(2, 6, 4, height() - 12), 2, 2);
    painter.fillPath(indicatorPath, indicatorColor);
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
