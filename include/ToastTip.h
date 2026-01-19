#pragma once

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QColor>

class ToastTip : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ windowOpacity WRITE setWindowOpacity)

public:
    enum IconType { Info, Success, Warning, Error };
    
    static void show(QWidget *parent, const QString &message, IconType type = Info, int durationMs = 3000);
    static void info(QWidget *parent, const QString &message, int durationMs = 3000);
    static void success(QWidget *parent, const QString &message, int durationMs = 3000);
    static void warning(QWidget *parent, const QString &message, int durationMs = 3000);
    static void error(QWidget *parent, const QString &message, int durationMs = 3000);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    explicit ToastTip(QWidget *parent, const QString &message, IconType type, int durationMs);
    void showToast();
    void fadeOut();
    
    QLabel *m_iconLabel;
    QLabel *m_msgLabel;
    QTimer *m_timer;
    QPropertyAnimation *m_fadeAnim;
    int m_durationMs;
    IconType m_type;
    QColor m_bgColor;
    QColor m_textColor;
    QColor m_borderColor;
};
