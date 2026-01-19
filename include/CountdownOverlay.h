#pragma once

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPropertyAnimation>

class CountdownOverlay : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int countdown READ getCountdown WRITE setCountdown)

public:
    explicit CountdownOverlay(QWidget *parent = nullptr);
    ~CountdownOverlay();
    
    void startCountdown(int seconds = 3);
    void stopCountdown();
    int getCountdown() const { return m_countdown; }
    void setCountdown(int value) { m_countdown = value; update(); }

signals:
    void countdownFinished();
    void countdownCancelled();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private slots:
    void onTimerTick();

private:
    int m_countdown;
    QTimer *m_timer;
    QPropertyAnimation *m_scaleAnimation;
    QPropertyAnimation *m_fadeAnimation;
    bool m_isActive;
};
