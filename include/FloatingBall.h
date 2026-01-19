#pragma once

#include <QWidget>
#include <QMouseEvent>
#include <QPoint>

class FloatingBall : public QWidget {
    Q_OBJECT

public:
    explicit FloatingBall(QWidget *parent = nullptr);
    void setRecordingState(bool recording);
    void updateDuration(const QString& duration);

signals:
    void requestShowMainWindow();
    void requestStartRecording();
    void requestStopRecording();
    void requestExit();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void dockToEdge();
    void updateSize();

    bool m_isDragging;
    QPoint m_dragPos;
    bool m_isRightSide;
    bool m_isRecording;
    bool m_isHovered;
    QString m_durationText;
};
