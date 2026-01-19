#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QKeySequence>
#include <QHBoxLayout>

class HotkeyEdit : public QWidget {
    Q_OBJECT

public:
    explicit HotkeyEdit(QWidget *parent = nullptr);
    
    QKeySequence keySequence() const { return m_keySequence; }
    void setKeySequence(const QKeySequence &seq);
    void clear();
    
    // 获取修饰键和虚拟键码（用于 Windows RegisterHotKey）
    quint32 getModifiers() const { return m_modifiers; }
    quint32 getVirtualKey() const { return m_virtualKey; }
    
    void setFixedWidth(int w);
    
    // 设置主题颜色
    void setThemeColors(const QString &borderColor, const QString &textColor, const QString &bgColor);
    
signals:
    void keySequenceChanged(const QKeySequence &seq);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onClearClicked();
    void onEditClicked();

private:
    QLineEdit *m_lineEdit;
    QPushButton *m_btnClear;
    QPushButton *m_btnEdit;
    
    QKeySequence m_keySequence;
    quint32 m_modifiers;
    quint32 m_virtualKey;
    bool m_isEditing;
    
    // 主题颜色
    QString m_borderColor;
    QString m_textColor;
    QString m_bgColor;
    
    void updateDisplay();
    void parseKeySequence();
    void startEditing();
    void stopEditing();
    void handleKeyPress(QKeyEvent *event);
    void applyThemeStyle();
};
