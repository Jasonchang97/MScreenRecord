#include "CustomMessageBox.h"
#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QMouseEvent>

// Helper (reused)
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

CustomMessageBox::CustomMessageBox(QWidget *parent, const QString &title, const QString &text, IconType icon)
    : QDialog(parent), m_isDragging(false)
{
    setObjectName("CustomMessageBox");
    setAttribute(Qt::WA_StyledBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::Window);
    setFixedSize(350, 200);
    
    // Inherit styles? Maybe better to apply explicitly
    
    setupUi(title, text, icon);
    
    // Apply theme
    QString theme = SettingsDialog::getTheme();
    QString bg, fg, border, btnBg, btnHover;
    QColor iconColor;
    
    if (theme == "light") {
        bg = "#f5f5f5"; fg = "#333333"; border = "#cccccc";
        btnBg = "#e0e0e0"; btnHover = "#d0d0d0";
        iconColor = Qt::black;
    } else if (theme == "tech") {
        bg = "#121a2e"; fg = "#aaddff"; border = "#2a3d5c";
        btnBg = "#1c2841"; btnHover = "#2a3d5c";
        iconColor = QColor("#4facfe");
    } else if (theme == "pink") {
        bg = "#fff0f5"; fg = "#552233"; border = "#ffb6c1";
        btnBg = "#ffe4e1"; btnHover = "#ffc0cb";
        iconColor = QColor("#552233");
    } else { // dark
        bg = "#1e1e1e"; fg = "#f0f0f0"; border = "#333333";
        btnBg = "#3a3a3a"; btnHover = "#4a4a4a";
        iconColor = Qt::white;
    }
    
    QString style = QString(
        "#CustomMessageBox { border: 1px solid %1; border-radius: 6px; background-color: %2; }"
        "#TitleBar { background-color: transparent; border-bottom: 1px solid %1; }"
        "#TitleLabel { color: %3; font-weight: bold; font-size: 13px; }"
        "#MsgContent { color: %3; font-size: 14px; }"
        "QPushButton { background-color: %4; border: 1px solid %1; border-radius: 4px; color: %3; padding: 6px; }"
        "QPushButton:hover { background-color: %5; }"
        "#CloseButton { background: transparent; border: none; }"
        "#CloseButton:hover { background-color: #c62828; }"
    ).arg(border, bg, fg, btnBg, btnHover);
    
    this->setStyleSheet(style);
    
    // Icon color
    QPushButton *btnClose = findChild<QPushButton*>("CloseButton");
    if (btnClose) btnClose->setIcon(createIcon(this->style()->standardIcon(QStyle::SP_TitleBarCloseButton), iconColor));
}

void CustomMessageBox::setupUi(const QString &title, const QString &text, IconType iconType) {
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Title Bar
    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("TitleBar");
    m_titleBar->setFixedHeight(36);
    QHBoxLayout *titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(15, 0, 10, 0);
    
    QLabel *lblTitle = new QLabel(title, this);
    lblTitle->setObjectName("TitleLabel");
    
    QPushButton *btnClose = new QPushButton(this);
    btnClose->setFixedSize(24, 24);
    btnClose->setFlat(true);
    btnClose->setObjectName("CloseButton");
    connect(btnClose, &QPushButton::clicked, this, &CustomMessageBox::reject);
    
    titleLayout->addWidget(lblTitle);
    titleLayout->addStretch();
    titleLayout->addWidget(btnClose);
    rootLayout->addWidget(m_titleBar);

    // Content
    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(15);
    
    QHBoxLayout *msgLayout = new QHBoxLayout();
    QLabel *iconLabel = new QLabel(this);
    QStyle::StandardPixmap sp;
    switch(iconType) {
        case Info: sp = QStyle::SP_MessageBoxInformation; break;
        case Warning: sp = QStyle::SP_MessageBoxWarning; break;
        case Error: sp = QStyle::SP_MessageBoxCritical; break;
        case Question: sp = QStyle::SP_MessageBoxQuestion; break;
        default: sp = QStyle::SP_MessageBoxInformation; break;
    }
    // Icon color handled in paint or just use standard
    // For custom coloring, we need to know the color here.
    // Re-fetch theme color logic or just use white/black based on theme string
    QString theme = SettingsDialog::getTheme();
    QColor iconColor = (theme == "light") ? Qt::black : Qt::white;
    if (theme == "tech") iconColor = QColor("#4facfe");
    if (theme == "pink") iconColor = QColor("#552233");
    
    iconLabel->setPixmap(createIcon(style()->standardIcon(sp), iconColor).pixmap(48, 48));
    iconLabel->setFixedSize(48, 48);
    
    QLabel *msgLabel = new QLabel(text, this);
    msgLabel->setObjectName("MsgContent");
    msgLabel->setWordWrap(true);
    msgLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    msgLayout->addWidget(iconLabel);
    msgLayout->addWidget(msgLabel, 1);
    
    contentLayout->addLayout(msgLayout);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    
    if (iconType == Question) {
        QPushButton *btnYes = new QPushButton("确定", this);
        QPushButton *btnNo = new QPushButton("取消", this);
        btnYes->setObjectName("BtnAction");
        btnNo->setObjectName("BtnCancel");
        btnYes->setCursor(Qt::PointingHandCursor);
        btnNo->setCursor(Qt::PointingHandCursor);
        btnYes->setFixedWidth(80);
        btnNo->setFixedWidth(80);
        
        connect(btnYes, &QPushButton::clicked, this, &CustomMessageBox::accept);
        connect(btnNo, &QPushButton::clicked, this, &CustomMessageBox::reject);
        
        btnLayout->addWidget(btnYes);
        btnLayout->addWidget(btnNo);
    } else {
        QPushButton *btnOk = new QPushButton("确定", this);
        btnOk->setObjectName("BtnAction"); 
        btnOk->setCursor(Qt::PointingHandCursor);
        btnOk->setFixedWidth(80);
        connect(btnOk, &QPushButton::clicked, this, &CustomMessageBox::accept);
        btnLayout->addWidget(btnOk);
    }
    
    contentLayout->addLayout(btnLayout);
    rootLayout->addWidget(contentWidget);
}

int CustomMessageBox::question(QWidget *parent, const QString &title, const QString &text) {
    CustomMessageBox box(parent, title, text, Question);
    return box.exec();
}

void CustomMessageBox::information(QWidget *parent, const QString &title, const QString &text) {
    CustomMessageBox box(parent, title, text, Info);
    box.exec();
}

// Drag logic
void CustomMessageBox::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_titleBar->geometry().contains(event->pos())) {
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
