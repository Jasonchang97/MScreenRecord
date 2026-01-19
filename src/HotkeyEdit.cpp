#include "HotkeyEdit.h"
#include <QKeyEvent>
#include <QStyle>
#include <QPainter>
#include <QTimer>
#include <QApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// 辅助函数：根据颜色创建图标
static QIcon createColoredIcon(QStyle::StandardPixmap sp, const QColor &color) {
    QIcon baseIcon = qApp->style()->standardIcon(sp);
    if (baseIcon.isNull()) return QIcon();
    QPixmap pix = baseIcon.pixmap(16, 16);
    if (pix.isNull()) return QIcon();
    QImage img = pix.toImage();
    QPainter p(&img);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(img.rect(), color);
    p.end();
    return QIcon(QPixmap::fromImage(img));
}

// 绘制键盘图标
static QIcon createKeyboardIcon(const QColor &color) {
    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    
    QPen pen(color, 1.2);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    
    // 键盘外框
    p.drawRoundedRect(1, 3, 14, 10, 2, 2);
    
    // 按键行
    p.drawLine(4, 6, 5, 6);
    p.drawLine(7, 6, 8, 6);
    p.drawLine(10, 6, 11, 6);
    
    p.drawLine(3, 9, 4, 9);
    p.drawLine(6, 9, 10, 9); // 空格键
    p.drawLine(12, 9, 13, 9);
    
    p.end();
    return QIcon(pix);
}

// 绘制清除/关闭图标
static QIcon createClearIcon(const QColor &color) {
    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    
    QPen pen(color, 1.8);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    
    // X 图标
    p.drawLine(4, 4, 12, 12);
    p.drawLine(12, 4, 4, 12);
    
    p.end();
    return QIcon(pix);
}

// 绘制确认/勾选图标
static QIcon createCheckIcon(const QColor &color) {
    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    
    QPen pen(color, 2);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);
    
    // 勾选图标
    p.drawLine(3, 8, 6, 11);
    p.drawLine(6, 11, 13, 4);
    
    p.end();
    return QIcon(pix);
}

HotkeyEdit::HotkeyEdit(QWidget *parent)
    : QWidget(parent), m_modifiers(0), m_virtualKey(0), m_isEditing(false),
      m_borderColor("#555"), m_textColor("#fff"), m_bgColor("#1e1e1e")
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 1, 0, 1); // 上下各留 1px 边距确保边框可见
    layout->setSpacing(4);
    
    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setReadOnly(true);
    m_lineEdit->setAlignment(Qt::AlignCenter);
    m_lineEdit->setPlaceholderText("未设置");
    m_lineEdit->setCursor(Qt::PointingHandCursor);
    m_lineEdit->setFixedHeight(26);
    m_lineEdit->installEventFilter(this);
    
    m_btnEdit = new QPushButton(this);
    m_btnEdit->setFixedSize(26, 26);
    m_btnEdit->setToolTip("点击设置快捷键");
    m_btnEdit->setCursor(Qt::PointingHandCursor);
    m_btnEdit->setIconSize(QSize(16, 16));
    
    m_btnClear = new QPushButton(this);
    m_btnClear->setFixedSize(26, 26);
    m_btnClear->setToolTip("清除快捷键");
    m_btnClear->setCursor(Qt::PointingHandCursor);
    m_btnClear->setIconSize(QSize(16, 16));
    
    layout->addWidget(m_lineEdit, 1);
    layout->addWidget(m_btnEdit);
    layout->addWidget(m_btnClear);
    
    connect(m_btnClear, &QPushButton::clicked, this, &HotkeyEdit::onClearClicked);
    connect(m_btnEdit, &QPushButton::clicked, this, &HotkeyEdit::onEditClicked);
    
    applyThemeStyle();
    updateDisplay();
}

void HotkeyEdit::setThemeColors(const QString &borderColor, const QString &textColor, const QString &bgColor) {
    m_borderColor = borderColor;
    m_textColor = textColor;
    m_bgColor = bgColor;
    applyThemeStyle();
    updateDisplay();
}

void HotkeyEdit::applyThemeStyle() {
    QColor textCol(m_textColor);
    
    m_lineEdit->setStyleSheet(QString(
        "QLineEdit { border: 1px solid %1; border-radius: 4px; background: transparent; color: %2; padding: 2px; }"
    ).arg(m_borderColor, m_textColor));
    
    // 设置图标
    m_btnEdit->setIcon(createKeyboardIcon(textCol));
    m_btnClear->setIcon(createClearIcon(textCol));
    
    m_btnEdit->setStyleSheet(QString(
        "QPushButton { border: 1px solid %1; border-radius: 3px; background: transparent; }"
        "QPushButton:hover { background: rgba(128,128,128,0.3); }"
    ).arg(m_borderColor));
    
    m_btnClear->setStyleSheet(QString(
        "QPushButton { border: 1px solid %1; border-radius: 3px; background: transparent; }"
        "QPushButton:hover { background: rgba(255,100,100,0.3); }"
    ).arg(m_borderColor));
}

void HotkeyEdit::setFixedWidth(int w) {
    QWidget::setFixedWidth(w);
}

void HotkeyEdit::setKeySequence(const QKeySequence &seq) {
    m_keySequence = seq;
    parseKeySequence();
    updateDisplay();
}

void HotkeyEdit::clear() {
    m_keySequence = QKeySequence();
    m_modifiers = 0;
    m_virtualKey = 0;
    updateDisplay();
}

void HotkeyEdit::onClearClicked() {
    if (m_isEditing) {
        stopEditing();
    }
    clear();
    emit keySequenceChanged(QKeySequence());
}

void HotkeyEdit::onEditClicked() {
    if (m_isEditing) {
        stopEditing();
    } else {
        startEditing();
    }
}

void HotkeyEdit::startEditing() {
    m_isEditing = true;
    QColor textCol(m_textColor);
    
    m_lineEdit->setStyleSheet(QString(
        "QLineEdit { border: 2px solid #4a9eff; border-radius: 4px; background: rgba(74,158,255,0.1); color: %1; }"
    ).arg(m_textColor));
    m_lineEdit->setPlaceholderText("请按下快捷键...");
    m_lineEdit->setText("");
    m_lineEdit->setFocus();
    
    // 编辑状态显示确认图标
    m_btnEdit->setIcon(createCheckIcon(QColor("#4a9eff")));
    m_btnEdit->setToolTip("完成编辑");
    m_btnEdit->setStyleSheet(
        "QPushButton { border: 1px solid #4a9eff; border-radius: 3px; background: rgba(74,158,255,0.2); }"
        "QPushButton:hover { background: rgba(74,158,255,0.4); }"
    );
}

void HotkeyEdit::stopEditing() {
    m_isEditing = false;
    QColor textCol(m_textColor);
    
    m_lineEdit->setStyleSheet(QString(
        "QLineEdit { border: 1px solid %1; border-radius: 4px; background: transparent; color: %2; padding: 2px; }"
    ).arg(m_borderColor, m_textColor));
    
    // 恢复键盘图标
    m_btnEdit->setIcon(createKeyboardIcon(textCol));
    m_btnEdit->setToolTip("点击设置快捷键");
    m_btnEdit->setStyleSheet(QString(
        "QPushButton { border: 1px solid %1; border-radius: 3px; background: transparent; }"
        "QPushButton:hover { background: rgba(128,128,128,0.3); }"
    ).arg(m_borderColor));
    
    updateDisplay();
}

bool HotkeyEdit::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_lineEdit) {
        if (event->type() == QEvent::MouseButtonPress) {
            if (!m_isEditing) {
                startEditing();
            }
            return true;
        }
        if (event->type() == QEvent::KeyPress && m_isEditing) {
            handleKeyPress(static_cast<QKeyEvent*>(event));
            return true;
        }
        if (event->type() == QEvent::FocusOut && m_isEditing) {
            // 延迟停止编辑，以便点击按钮时不会立即停止
            QTimer::singleShot(100, this, [this]() {
                if (m_isEditing && !m_lineEdit->hasFocus() && 
                    !m_btnEdit->underMouse() && !m_btnClear->underMouse()) {
                    stopEditing();
                }
            });
        }
    }
    return QWidget::eventFilter(obj, event);
}

void HotkeyEdit::handleKeyPress(QKeyEvent *event) {
    int key = event->key();
    
    // ESC 取消编辑
    if (key == Qt::Key_Escape) {
        stopEditing();
        return;
    }
    
    // 忽略单独的修饰键
    if (key == Qt::Key_Control || key == Qt::Key_Shift || 
        key == Qt::Key_Alt || key == Qt::Key_Meta) {
        // 显示当前按下的修饰键
        QString modText;
        if (event->modifiers() & Qt::ControlModifier) modText += "Ctrl+";
        if (event->modifiers() & Qt::AltModifier) modText += "Alt+";
        if (event->modifiers() & Qt::ShiftModifier) modText += "Shift+";
        if (event->modifiers() & Qt::MetaModifier) modText += "Win+";
        if (!modText.isEmpty()) {
            modText.chop(1); // 移除最后的 +
            m_lineEdit->setText(modText + "...");
        }
        return;
    }
    
    // 构建快捷键
    int modifiers = event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier | Qt::MetaModifier);
    
    // 至少需要一个修饰键（Ctrl/Alt/Shift），除非是功能键
    if (modifiers == 0) {
        if (key < Qt::Key_F1 || key > Qt::Key_F12) {
            m_lineEdit->setText("需要组合键 (Ctrl/Alt/Shift)");
            QTimer::singleShot(1500, this, [this]() {
                if (m_isEditing) {
                    m_lineEdit->setText("");
                    m_lineEdit->setPlaceholderText("请按下快捷键...");
                }
            });
            return;
        }
    }
    
    m_keySequence = QKeySequence(modifiers | key);
    parseKeySequence();
    stopEditing();
    
    emit keySequenceChanged(m_keySequence);
}

void HotkeyEdit::updateDisplay() {
    QColor textCol(m_textColor);
    QColor disabledColor = textCol;
    disabledColor.setAlpha(80);
    
    if (m_keySequence.isEmpty()) {
        m_lineEdit->clear();
        m_lineEdit->setPlaceholderText("未设置");
        m_btnClear->setEnabled(false);
        m_btnClear->setIcon(createClearIcon(disabledColor));
        m_btnClear->setStyleSheet(QString(
            "QPushButton { border: 1px solid %1; border-radius: 3px; background: transparent; }"
        ).arg(m_borderColor));
    } else {
        m_lineEdit->setText(m_keySequence.toString(QKeySequence::NativeText));
        m_btnClear->setEnabled(true);
        m_btnClear->setIcon(createClearIcon(textCol));
        m_btnClear->setStyleSheet(QString(
            "QPushButton { border: 1px solid %1; border-radius: 3px; background: transparent; }"
            "QPushButton:hover { background: rgba(255,100,100,0.3); }"
        ).arg(m_borderColor));
    }
}

void HotkeyEdit::parseKeySequence() {
    m_modifiers = 0;
    m_virtualKey = 0;
    
    if (m_keySequence.isEmpty()) return;
    
#ifdef Q_OS_WIN
    int key = m_keySequence[0];
    
    // 解析修饰键
    if (key & Qt::ControlModifier) m_modifiers |= MOD_CONTROL;
    if (key & Qt::AltModifier) m_modifiers |= MOD_ALT;
    if (key & Qt::ShiftModifier) m_modifiers |= MOD_SHIFT;
    if (key & Qt::MetaModifier) m_modifiers |= MOD_WIN;
    
    // 解析虚拟键码
    int qtKey = key & ~(Qt::ControlModifier | Qt::AltModifier | Qt::ShiftModifier | Qt::MetaModifier);
    
    // 字母键
    if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z) {
        m_virtualKey = qtKey - Qt::Key_A + 'A';
    }
    // 数字键
    else if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9) {
        m_virtualKey = qtKey - Qt::Key_0 + '0';
    }
    // 功能键
    else if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F12) {
        m_virtualKey = qtKey - Qt::Key_F1 + VK_F1;
    }
    // 其他特殊键
    else {
        switch (qtKey) {
            case Qt::Key_Space: m_virtualKey = VK_SPACE; break;
            case Qt::Key_Return:
            case Qt::Key_Enter: m_virtualKey = VK_RETURN; break;
            case Qt::Key_Tab: m_virtualKey = VK_TAB; break;
            case Qt::Key_Home: m_virtualKey = VK_HOME; break;
            case Qt::Key_End: m_virtualKey = VK_END; break;
            case Qt::Key_Insert: m_virtualKey = VK_INSERT; break;
            case Qt::Key_PageUp: m_virtualKey = VK_PRIOR; break;
            case Qt::Key_PageDown: m_virtualKey = VK_NEXT; break;
            case Qt::Key_Up: m_virtualKey = VK_UP; break;
            case Qt::Key_Down: m_virtualKey = VK_DOWN; break;
            case Qt::Key_Left: m_virtualKey = VK_LEFT; break;
            case Qt::Key_Right: m_virtualKey = VK_RIGHT; break;
            default: m_virtualKey = 0; break;
        }
    }
#endif
}
