#pragma once

#include <QWidget>
#include <QMouseEvent>
#include <QScreen>
#include <QPainter>
#include <QApplication>
#include <QTimer>
#include <QPropertyAnimation>

class SelectionOverlay : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal countdownScale READ countdownScale WRITE setCountdownScale)
    Q_PROPERTY(qreal countdownOpacity READ countdownOpacity WRITE setCountdownOpacity)

public:
    explicit SelectionOverlay(QWidget *parent = nullptr);
    QRect getSelection() const;
    
    void startRecording();
    void stopRecording();
    void updateDuration(const QString &duration);
    bool isRecording() const { return m_isRecording; }
    
    void setCountdownEnabled(bool enabled, int seconds = 3);
    
    // Animation properties
    qreal countdownScale() const { return m_countdownScale; }
    void setCountdownScale(qreal scale) { m_countdownScale = scale; update(); }
    qreal countdownOpacity() const { return m_countdownOpacity; }
    void setCountdownOpacity(qreal opacity) { m_countdownOpacity = opacity; update(); }

signals:
    void areaSelected(const QRect &rect);
    void cancelled();
    void requestStartRecording();
    void requestStopRecording();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    // Window detection
    struct WindowInfo {
        quintptr hwnd;
        QRect rect;
        QString title;
    };
    
    QRect m_selection;
    QRect m_hoveredWindow;
    QList<WindowInfo> m_windows;
    
    // Toolbar state
    bool m_showToolbar;
    bool m_isRecording;
    QRect m_toolbarRect;
    QRect m_buttonRect;
    bool m_isDraggingToolbar;
    QPoint m_toolbarDragPos;
    QString m_durationText;
    
    // Countdown
    bool m_countdownEnabled;
    int m_countdownSecs;
    int m_currentCountdown;
    QTimer *m_countdownTimer;
    bool m_isCountingDown;
    qreal m_countdownScale;
    qreal m_countdownOpacity;
    QPropertyAnimation *m_scaleAnim;
    QPropertyAnimation *m_opacityAnim;
    
    // Throttle for window detection
    QTimer *m_updateThrottle;
    QPoint m_lastMousePos;
    bool m_pendingUpdate;
    
    void detectWindows();
    QRect getWindowAtPoint(const QPoint &pos);
    void updateToolbarPosition();
    bool isOverToolbar(const QPoint &pos) const;
    bool isOverButton(const QPoint &pos) const;
    void startCountdown();
    void onCountdownTick();
    void animateCountdownNumber();
};
