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
    QString theme = settings.value("theme", "dark").toString().toLower().trimmed();
    
    // 根据不同皮肤设置颜色
    if (theme == "light") {
        m_bgColor = QColor(255, 255, 255, 252);
        m_textColor = QColor(48, 48, 48);
        m_borderColor = QColor(220, 220, 220);
        m_shadowColor = QColor(0, 0, 0, 25);
    } else if (theme == "pink") {
        m_bgColor = QColor(255, 245, 248, 252);
        m_textColor = QColor(85, 34, 51);
        m_borderColor = QColor(255, 182, 193);
        m_shadowColor = QColor(255, 105, 180, 30);
    } else if (theme == "tech") {
        m_bgColor = QColor(18, 26, 46, 252);
        m_textColor = QColor(170, 221, 255);
        m_borderColor = QColor(42, 61, 92);
        m_shadowColor = QColor(0, 229, 255, 30);
    } else if (theme == "purple") {
        m_bgColor = QColor(62, 46, 78, 252);
        m_textColor = QColor(224, 176, 255);
        m_borderColor = QColor(122, 90, 154);
        m_shadowColor = QColor(224, 176, 255, 30);
    } else if (theme == "green") {
        m_bgColor = QColor(46, 62, 46, 252);
        m_textColor = QColor(160, 224, 160);
        m_borderColor = QColor(90, 122, 90);
        m_shadowColor = QColor(144, 238, 144, 30);
    } else if (theme == "zijunpink") {
        // 子君粉：底色 #F0D5CF（浅粉），文字 #5D4A42，边框 #C7BBA8
        m_bgColor = QColor(240, 213, 207, 252); // #F0D5CF
        m_textColor = QColor(93, 74, 66);       // #5D4A42
        m_borderColor = QColor(199, 187, 168);  // #C7BBA8
        m_shadowColor = QColor(199, 187, 168, 40);
    } else if (theme == "zijunwhite") {
        // 子君白：底色 #F8F4EE，文字 #6B5D53，边框 #E6C7C0
        m_bgColor = QColor(248, 244, 238, 252); // #F8F4EE
        m_textColor = QColor(107, 93, 83);      // #6B5D53
        m_borderColor = QColor(230, 199, 192);  // #E6C7C0
        m_shadowColor = QColor(230, 199, 192, 40);
    } else {
        // dark (default)
        m_bgColor = QColor(45, 45, 48, 252);
        m_textColor = QColor(230, 230, 230);
        m_borderColor = QColor(70, 70, 75);
        m_shadowColor = QColor(0, 0, 0, 40);
    }
    
    // 主布局 - 增加边距给阴影留空间
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(18, 14, 22, 14);
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
            p.drawLine(6, 11, 9, 14);
            p.drawLine(9, 14, 16, 7);
            break;
        case Warning:
            p.drawLine(11, 6, 11, 12);
            p.setBrush(iconColor);
            p.drawEllipse(9, 14, 4, 4);
            break;
        case Error:
            p.drawLine(7, 7, 15, 15);
            p.drawLine(15, 7, 7, 15);
            break;
        case Info:
        default:
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
    
    const int shadowSize = 6;
    const int radius = 10;
    QRect bgRect = rect().adjusted(shadowSize, shadowSize - 2, -shadowSize, -shadowSize + 2);
    
    // 绘制柔和阴影（使用渐变模拟模糊效果）
    for (int i = shadowSize; i > 0; --i) {
        QColor shadow = m_shadowColor;
        // 使用非线性衰减使阴影更柔和
        int alpha = m_shadowColor.alpha() * (shadowSize - i + 1) / (shadowSize + 2);
        shadow.setAlpha(alpha);
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(shadow);
        
        QRect shadowRect = bgRect.adjusted(-i, -i + 2, i, i + 2);
        painter.drawRoundedRect(shadowRect, radius + i/2.0, radius + i/2.0);
    }
    
    // 绘制背景
    QPainterPath bgPath;
    bgPath.addRoundedRect(bgRect, radius, radius);
    painter.fillPath(bgPath, m_bgColor);
    
    // 绘制边框
    painter.setPen(QPen(m_borderColor, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(bgRect, radius, radius);
}

void ToastTip::showToast() {
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
