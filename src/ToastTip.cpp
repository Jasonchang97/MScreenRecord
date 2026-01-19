#include "ToastTip.h"
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QPainterPath>
#include <QSettings>

// 静态成员定义
QList<ToastTip*> ToastTip::s_activeToasts;
QMutex ToastTip::s_mutex;

ToastTip::ToastTip(QWidget *parent, const QString &message, IconType type, int durationMs)
    : QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::BypassWindowManagerHint)
    , m_durationMs(durationMs)
    , m_type(type)
    , m_parent(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);
    
    // 读取当前皮肤设置
    QSettings settings("KSO", "MScreenRecord");
    QString theme = settings.value("theme", "dark").toString();
    
    // 根据皮肤设置背景和文字颜色
    if (theme == "light") {
        m_bgColor = QColor(255, 255, 255, 250);
        m_textColor = QColor(48, 48, 48);
        m_borderColor = QColor(230, 230, 230);
    } else {
        m_bgColor = QColor(45, 45, 48, 250);
        m_textColor = QColor(230, 230, 230);
        m_borderColor = QColor(70, 70, 75);
    }
    
    // 主布局
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(14, 10, 18, 10);
    layout->setSpacing(12);
    
    // 图标容器
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(22, 22);
    m_iconLabel->setAttribute(Qt::WA_TranslucentBackground);
    
    // 绘制图标（圆形背景 + 图标）
    QPixmap iconPix(22, 22);
    iconPix.fill(Qt::transparent);
    QPainter p(&iconPix);
    p.setRenderHint(QPainter::Antialiasing);
    
    QColor iconColor, iconBgColor;
    switch (type) {
        case Success:
            iconColor = QColor("#ffffff");
            iconBgColor = QColor("#52c41a");
            break;
        case Warning:
            iconColor = QColor("#ffffff");
            iconBgColor = QColor("#faad14");
            break;
        case Error:
            iconColor = QColor("#ffffff");
            iconBgColor = QColor("#ff4d4f");
            break;
        case Info:
        default:
            iconColor = QColor("#ffffff");
            iconBgColor = QColor("#1890ff");
            break;
    }
    
    // 绘制圆形背景
    p.setBrush(iconBgColor);
    p.setPen(Qt::NoPen);
    p.drawEllipse(1, 1, 20, 20);
    
    // 绘制图标
    p.setPen(QPen(iconColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    switch (type) {
        case Success:
            // 勾选
            p.drawLine(6, 11, 9, 14);
            p.drawLine(9, 14, 16, 7);
            break;
        case Warning:
            // 感叹号
            p.drawLine(11, 6, 11, 12);
            p.setBrush(iconColor);
            p.drawEllipse(9, 14, 4, 4);
            break;
        case Error:
            // X
            p.drawLine(7, 7, 15, 15);
            p.drawLine(15, 7, 7, 15);
            break;
        case Info:
        default:
            // i
            p.drawLine(11, 9, 11, 15);
            p.setBrush(iconColor);
            p.drawEllipse(9, 5, 4, 4);
            break;
    }
    p.end();
    m_iconLabel->setPixmap(iconPix);
    
    // 消息文本
    m_msgLabel = new QLabel(message, this);
    m_msgLabel->setStyleSheet(QString(
        "color: %1; "
        "font-size: 13px; "
        "font-weight: 500; "
        "background: transparent;"
    ).arg(m_textColor.name()));
    m_msgLabel->setWordWrap(false);
    
    layout->addWidget(m_iconLabel);
    layout->addWidget(m_msgLabel);
    layout->addStretch();
    
    // 定时器
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &ToastTip::fadeOut);
    
    // 淡出动画
    m_fadeAnim = new QPropertyAnimation(this, "windowOpacity", this);
    m_fadeAnim->setDuration(250);
    m_fadeAnim->setStartValue(1.0);
    m_fadeAnim->setEndValue(0.0);
    connect(m_fadeAnim, &QPropertyAnimation::finished, this, [this]() {
        removeFromQueue();
        close();
    });
    
    // 位置动画
    m_posAnim = new QPropertyAnimation(this, "pos", this);
    m_posAnim->setDuration(200);
    m_posAnim->setEasingCurve(QEasingCurve::OutCubic);
    
    setMinimumWidth(200);
    setMaximumWidth(400);
    adjustSize();
}

ToastTip::~ToastTip() {
    removeFromQueue();
}

void ToastTip::removeFromQueue() {
    QMutexLocker locker(&s_mutex);
    s_activeToasts.removeAll(this);
    // 重新排列剩余的 toast
    QMetaObject::invokeMethod(qApp, [this]() {
        repositionAll();
    }, Qt::QueuedConnection);
}

void ToastTip::repositionAll() {
    QMutexLocker locker(&s_mutex);
    int yOffset = 80;
    const int spacing = 10;
    
    for (ToastTip *toast : s_activeToasts) {
        if (toast && toast->isVisible()) {
            QPoint targetPos = toast->calculatePosition(yOffset);
            if (toast->pos() != targetPos) {
                toast->m_posAnim->stop();
                toast->m_posAnim->setStartValue(toast->pos());
                toast->m_posAnim->setEndValue(targetPos);
                toast->m_posAnim->start();
            }
            yOffset += toast->height() + spacing;
        }
    }
}

QPoint ToastTip::calculatePosition(int yOffset) {
    QScreen *screen = QApplication::primaryScreen();
    if (!screen) return QPoint(0, yOffset);
    
    QRect screenGeom = screen->availableGeometry();
    int x = screenGeom.x() + (screenGeom.width() - width()) / 2;
    int y = screenGeom.y() + yOffset;
    
    return QPoint(x, y);
}

void ToastTip::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QRect bgRect = rect().adjusted(1, 1, -1, -1);
    
    // 绘制阴影效果（多层模糊）
    for (int i = 4; i > 0; --i) {
        QColor shadowColor(0, 0, 0, 10 + i * 5);
        painter.setPen(Qt::NoPen);
        painter.setBrush(shadowColor);
        painter.drawRoundedRect(bgRect.adjusted(-i, -i + 2, i, i + 2), 10 + i, 10 + i);
    }
    
    // 绘制背景
    QPainterPath path;
    path.addRoundedRect(bgRect, 10, 10);
    painter.fillPath(path, m_bgColor);
    
    // 绘制边框
    painter.setPen(QPen(m_borderColor, 1));
    painter.drawPath(path);
}

void ToastTip::showToast() {
    // 关闭同类型的旧 toast（可选：避免重复提示堆积）
    {
        QMutexLocker locker(&s_mutex);
        // 限制最多显示3个toast
        while (s_activeToasts.size() >= 3) {
            ToastTip *oldest = s_activeToasts.first();
            if (oldest) {
                oldest->close();
            }
            s_activeToasts.removeFirst();
        }
        s_activeToasts.append(this);
    }
    
    // 计算位置
    int yOffset = 80;
    const int spacing = 10;
    {
        QMutexLocker locker(&s_mutex);
        for (ToastTip *toast : s_activeToasts) {
            if (toast != this && toast->isVisible()) {
                yOffset += toast->height() + spacing;
            }
        }
    }
    
    QPoint pos = calculatePosition(yOffset);
    move(pos);
    setWindowOpacity(0.0);
    QWidget::show();
    raise();
    
    // 淡入动画
    QPropertyAnimation *fadeIn = new QPropertyAnimation(this, "windowOpacity", this);
    fadeIn->setDuration(200);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    
    m_timer->start(m_durationMs);
}

void ToastTip::fadeOut() {
    m_fadeAnim->start();
}

void ToastTip::closeAll() {
    QMutexLocker locker(&s_mutex);
    for (ToastTip *toast : s_activeToasts) {
        if (toast) {
            toast->close();
        }
    }
    s_activeToasts.clear();
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
