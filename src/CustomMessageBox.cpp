#include "CustomMessageBox.h"
#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QStyle>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QScreen>
#include <cmath>

CustomMessageBox::CustomMessageBox(QWidget *parent, const QString &title, const QString &text, IconType icon)
    : QDialog(parent), m_iconType(icon), m_isDragging(false), m_popupScale(0.8)
{
    setObjectName("CustomMessageBox");
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::Window);
    setFixedSize(380, 220);
    
    loadThemeColors();
    setupUi(title, text, icon);
    
    // Popup animation
    m_animation = new QPropertyAnimation(this, "popupScale", this);
    m_animation->setDuration(200);
    m_animation->setStartValue(0.8);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::OutBack);
}

void CustomMessageBox::setPopupScale(qreal scale) {
    m_popupScale = scale;
    update();
}

void CustomMessageBox::loadThemeColors() {
    QString theme = SettingsDialog::getTheme().toLower();
    
    if (theme == "light") {
        m_bgColor = QColor("#ffffff");
        m_textColor = QColor("#333333");
        m_borderColor = QColor("#e0e0e0");
        m_btnBgColor = QColor("#f0f0f0");
        m_btnHoverColor = QColor("#e0e0e0");
        m_accentColor = QColor("#2196F3");
        m_shadowColor = QColor(0, 0, 0, 30);
    } else if (theme == "pink") {
        m_bgColor = QColor("#fff5f8");
        m_textColor = QColor("#552233");
        m_borderColor = QColor("#ffb6c1");
        m_btnBgColor = QColor("#ffe4e8");
        m_btnHoverColor = QColor("#ffd0d8");
        m_accentColor = QColor("#e91e63");
        m_shadowColor = QColor(255, 182, 193, 40);
    } else if (theme == "tech") {
        m_bgColor = QColor("#0d1421");
        m_textColor = QColor("#aaddff");
        m_borderColor = QColor("#1e3a5f");
        m_btnBgColor = QColor("#152238");
        m_btnHoverColor = QColor("#1e3a5f");
        m_accentColor = QColor("#00bcd4");
        m_shadowColor = QColor(0, 188, 212, 30);
    } else if (theme == "purple") {
        m_bgColor = QColor("#1a1025");
        m_textColor = QColor("#e0b0ff");
        m_borderColor = QColor("#3d2060");
        m_btnBgColor = QColor("#2a1540");
        m_btnHoverColor = QColor("#3d2060");
        m_accentColor = QColor("#9c27b0");
        m_shadowColor = QColor(156, 39, 176, 30);
    } else if (theme == "green") {
        m_bgColor = QColor("#0a1810");
        m_textColor = QColor("#a0e0a0");
        m_borderColor = QColor("#1a3020");
        m_btnBgColor = QColor("#122418");
        m_btnHoverColor = QColor("#1a3020");
        m_accentColor = QColor("#4caf50");
        m_shadowColor = QColor(76, 175, 80, 30);
    } else if (theme == "zijunpink") {
        m_bgColor = QColor("#F0D5CF");
        m_textColor = QColor("#5D4A42");
        m_borderColor = QColor("#C7BBA8");
        m_btnBgColor = QColor("#e8c8c0");
        m_btnHoverColor = QColor("#ddb8b0");
        m_accentColor = QColor("#C7BBA8");
        m_shadowColor = QColor(199, 187, 168, 40);
    } else if (theme == "zijunwhite") {
        m_bgColor = QColor("#F8F4EE");
        m_textColor = QColor("#6B5D53");
        m_borderColor = QColor("#E6C7C0");
        m_btnBgColor = QColor("#ffffff");
        m_btnHoverColor = QColor("#f0e8e0");
        m_accentColor = QColor("#E6C7C0");
        m_shadowColor = QColor(230, 199, 192, 40);
    } else { // dark (default)
        m_bgColor = QColor("#1e1e1e");
        m_textColor = QColor("#f0f0f0");
        m_borderColor = QColor("#3a3a3a");
        m_btnBgColor = QColor("#2d2d2d");
        m_btnHoverColor = QColor("#3a3a3a");
        m_accentColor = QColor("#0078d4");
        m_shadowColor = QColor(0, 0, 0, 50);
    }
}

void CustomMessageBox::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Apply scale transform for popup animation
    QRect contentRect = rect().adjusted(12, 12, -12, -12);
    QPointF center = contentRect.center();
    
    painter.translate(center);
    painter.scale(m_popupScale, m_popupScale);
    painter.translate(-center);
    
    // Draw shadow layers
    for (int i = 4; i >= 1; --i) {
        QColor shadowCol = m_shadowColor;
        shadowCol.setAlpha(shadowCol.alpha() / i);
        painter.setPen(Qt::NoPen);
        painter.setBrush(shadowCol);
        painter.drawRoundedRect(contentRect.adjusted(-i*2, -i*2, i*2, i*2), 12 + i, 12 + i);
    }
    
    // Draw background
    painter.setPen(QPen(m_borderColor, 1));
    painter.setBrush(m_bgColor);
    painter.drawRoundedRect(contentRect, 12, 12);
    
    // Draw title bar separator
    int titleBarBottom = contentRect.top() + 44;
    painter.setPen(QPen(m_borderColor, 1));
    painter.drawLine(contentRect.left() + 1, titleBarBottom, contentRect.right() - 1, titleBarBottom);
    
    // Draw icon
    QRect iconRect(contentRect.left() + 24, titleBarBottom + 20, 48, 48);
    drawIcon(painter, iconRect, m_iconType);
}

void CustomMessageBox::drawIcon(QPainter &painter, const QRect &rect, IconType type) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    
    QColor iconBgColor, iconFgColor;
    
    switch (type) {
        case Info:
            iconBgColor = QColor("#2196F3");
            iconFgColor = Qt::white;
            break;
        case Warning:
            iconBgColor = QColor("#FF9800");
            iconFgColor = Qt::white;
            break;
        case Error:
            iconBgColor = QColor("#f44336");
            iconFgColor = Qt::white;
            break;
        case Question:
            iconBgColor = QColor("#9C27B0");
            iconFgColor = Qt::white;
            break;
        case Success:
            iconBgColor = QColor("#4CAF50");
            iconFgColor = Qt::white;
            break;
    }
    
    // Draw circular background
    painter.setPen(Qt::NoPen);
    painter.setBrush(iconBgColor);
    painter.drawEllipse(rect);
    
    // Draw icon symbol
    painter.setPen(QPen(iconFgColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    
    QPointF center = rect.center();
    int r = rect.width() / 2 - 12;
    
    switch (type) {
        case Info: {
            // "i" icon
            painter.setBrush(iconFgColor);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPointF(center.x(), center.y() - r + 4), 3, 3);
            painter.setPen(QPen(iconFgColor, 3, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(QPointF(center.x(), center.y() - 2), QPointF(center.x(), center.y() + r - 2));
            break;
        }
        case Warning: {
            // "!" icon
            painter.setPen(QPen(iconFgColor, 3, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(QPointF(center.x(), center.y() - r + 2), QPointF(center.x(), center.y() + 2));
            painter.setBrush(iconFgColor);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPointF(center.x(), center.y() + r - 4), 3, 3);
            break;
        }
        case Error: {
            // "×" icon
            painter.drawLine(QPointF(center.x() - r + 4, center.y() - r + 4), 
                           QPointF(center.x() + r - 4, center.y() + r - 4));
            painter.drawLine(QPointF(center.x() + r - 4, center.y() - r + 4), 
                           QPointF(center.x() - r + 4, center.y() + r - 4));
            break;
        }
        case Question: {
            // "?" icon
            QPainterPath path;
            path.moveTo(center.x() - 6, center.y() - r + 6);
            path.quadTo(center.x() - 6, center.y() - r - 2, center.x() + 2, center.y() - r + 2);
            path.quadTo(center.x() + 8, center.y() - r + 6, center.x() + 4, center.y() - 2);
            path.quadTo(center.x(), center.y() + 2, center.x(), center.y() + 4);
            painter.drawPath(path);
            painter.setBrush(iconFgColor);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPointF(center.x(), center.y() + r - 4), 3, 3);
            break;
        }
        case Success: {
            // "✓" icon
            painter.drawLine(QPointF(center.x() - r + 6, center.y()), 
                           QPointF(center.x() - 2, center.y() + r - 8));
            painter.drawLine(QPointF(center.x() - 2, center.y() + r - 8), 
                           QPointF(center.x() + r - 4, center.y() - r + 6));
            break;
        }
    }
    
    painter.restore();
}

void CustomMessageBox::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    
    // Center on parent or screen
    if (parentWidget()) {
        QPoint parentCenter = parentWidget()->mapToGlobal(parentWidget()->rect().center());
        move(parentCenter.x() - width() / 2, parentCenter.y() - height() / 2);
    } else {
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->availableGeometry();
            move(screenGeometry.center() - rect().center());
        }
    }
    
    m_animation->start();
}

void CustomMessageBox::setupUi(const QString &title, const QString &text, IconType iconType) {
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(0);

    // Main container
    QWidget *container = new QWidget(this);
    container->setObjectName("Container");
    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Title Bar
    m_titleBar = new QWidget(container);
    m_titleBar->setObjectName("TitleBar");
    m_titleBar->setFixedHeight(44);
    QHBoxLayout *titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(20, 0, 12, 0);
    
    QLabel *lblTitle = new QLabel(title, this);
    lblTitle->setObjectName("TitleLabel");
    lblTitle->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 14px; background: transparent;")
        .arg(m_textColor.name()));
    
    QPushButton *btnClose = new QPushButton(this);
    btnClose->setFixedSize(28, 28);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setObjectName("CloseButton");
    btnClose->setStyleSheet(QString(
        "QPushButton { background: transparent; border: none; border-radius: 14px; color: %1; font-size: 16px; }"
        "QPushButton:hover { background: #c62828; color: white; }"
    ).arg(m_textColor.name()));
    btnClose->setText("×");
    connect(btnClose, &QPushButton::clicked, this, &CustomMessageBox::reject);
    
    titleLayout->addWidget(lblTitle);
    titleLayout->addStretch();
    titleLayout->addWidget(btnClose);
    containerLayout->addWidget(m_titleBar);

    // Content area
    QWidget *contentWidget = new QWidget(container);
    QHBoxLayout *contentLayout = new QHBoxLayout(contentWidget);
    contentLayout->setContentsMargins(24, 20, 24, 16);
    contentLayout->setSpacing(16);
    
    // Icon placeholder (drawn in paintEvent)
    QWidget *iconPlaceholder = new QWidget(this);
    iconPlaceholder->setFixedSize(48, 48);
    
    QLabel *msgLabel = new QLabel(text, this);
    msgLabel->setObjectName("MsgContent");
    msgLabel->setWordWrap(true);
    msgLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    msgLabel->setStyleSheet(QString("color: %1; font-size: 13px; background: transparent;")
        .arg(m_textColor.name()));
    msgLabel->setMinimumHeight(48);
    
    contentLayout->addWidget(iconPlaceholder);
    contentLayout->addWidget(msgLabel, 1);
    containerLayout->addWidget(contentWidget);
    
    // Buttons
    QWidget *btnWidget = new QWidget(container);
    QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);
    btnLayout->setContentsMargins(24, 8, 24, 20);
    btnLayout->addStretch();
    
    QString btnStyle = QString(
        "QPushButton { "
        "   background-color: %1; "
        "   border: 1px solid %2; "
        "   border-radius: 6px; "
        "   color: %3; "
        "   padding: 8px 20px; "
        "   font-size: 13px; "
        "   font-weight: 500; "
        "}"
        "QPushButton:hover { background-color: %4; }"
        "QPushButton:pressed { background-color: %2; }"
    ).arg(m_btnBgColor.name(), m_borderColor.name(), m_textColor.name(), m_btnHoverColor.name());
    
    QString primaryBtnStyle = QString(
        "QPushButton { "
        "   background-color: %1; "
        "   border: none; "
        "   border-radius: 6px; "
        "   color: white; "
        "   padding: 8px 20px; "
        "   font-size: 13px; "
        "   font-weight: bold; "
        "}"
        "QPushButton:hover { background-color: %2; }"
    ).arg(m_accentColor.name(), m_accentColor.darker(110).name());
    
    if (iconType == Question) {
        QPushButton *btnNo = new QPushButton("取消", this);
        QPushButton *btnYes = new QPushButton("确定", this);
        btnNo->setObjectName("BtnCancel");
        btnYes->setObjectName("BtnAction");
        btnNo->setCursor(Qt::PointingHandCursor);
        btnYes->setCursor(Qt::PointingHandCursor);
        btnNo->setFixedWidth(90);
        btnYes->setFixedWidth(90);
        btnNo->setStyleSheet(btnStyle);
        btnYes->setStyleSheet(primaryBtnStyle);
        
        connect(btnYes, &QPushButton::clicked, this, &CustomMessageBox::accept);
        connect(btnNo, &QPushButton::clicked, this, &CustomMessageBox::reject);
        
        btnLayout->addWidget(btnNo);
        btnLayout->addSpacing(12);
        btnLayout->addWidget(btnYes);
    } else {
        QPushButton *btnOk = new QPushButton("确定", this);
        btnOk->setObjectName("BtnAction"); 
        btnOk->setCursor(Qt::PointingHandCursor);
        btnOk->setFixedWidth(90);
        btnOk->setStyleSheet(primaryBtnStyle);
        connect(btnOk, &QPushButton::clicked, this, &CustomMessageBox::accept);
        btnLayout->addWidget(btnOk);
    }
    
    containerLayout->addWidget(btnWidget);
    rootLayout->addWidget(container);
}

int CustomMessageBox::question(QWidget *parent, const QString &title, const QString &text) {
    CustomMessageBox box(parent, title, text, Question);
    return box.exec();
}

void CustomMessageBox::information(QWidget *parent, const QString &title, const QString &text) {
    CustomMessageBox box(parent, title, text, Info);
    box.exec();
}

void CustomMessageBox::warning(QWidget *parent, const QString &title, const QString &text) {
    CustomMessageBox box(parent, title, text, Warning);
    box.exec();
}

// Drag logic
void CustomMessageBox::mousePressEvent(QMouseEvent *event) {
    QRect titleBarRect(12, 12, width() - 24, 44);
    if (event->button() == Qt::LeftButton && titleBarRect.contains(event->pos())) {
        m_isDragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void CustomMessageBox::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void CustomMessageBox::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) m_isDragging = false;
}
