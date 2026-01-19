#pragma once

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QColor>
#include <QList>
#include <QMutex>

class ToastTip : public QWidget {
    Q_OBJECT

public:
    enum IconType { Info, Success, Warning, Error };
    
    static void show(QWidget *parent, const QString &message, IconType type = Info, int durationMs = 3000);
    static void info(QWidget *parent, const QString &message, int durationMs = 3000);
    static void success(QWidget *parent, const QString &message, int durationMs = 3000);
    static void warning(QWidget *parent, const QString &message, int durationMs = 3000);
    static void error(QWidget *parent, const QString &message, int durationMs = 3000);
    static void closeAll();

    ~ToastTip();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    explicit ToastTip(QWidget *parent, const QString &message, IconType type, int durationMs);
    void showToast();
    void fadeOut();
    void removeFromQueue();
    QPoint calculatePosition(int yOffset);
    static void repositionAll();
    
    QLabel *m_iconLabel;
    QLabel *m_msgLabel;
    QTimer *m_timer;
    QPropertyAnimation *m_fadeAnim;
    QPropertyAnimation *m_posAnim;
    int m_durationMs;
    IconType m_type;
    QColor m_bgColor;
    QColor m_textColor;
    QColor m_borderColor;
    QColor m_shadowColor;
    QWidget *m_parent;
    
    // 静态成员：管理所有活动的 toast
    static QList<ToastTip*> s_activeToasts;
    static QMutex s_mutex;
};
