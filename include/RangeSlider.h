#pragma once

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

class RangeSlider : public QWidget {
    Q_OBJECT

public:
    explicit RangeSlider(QWidget *parent = nullptr);

    void setRange(int min, int max);
    void setValues(int lower, int upper);
    void setPlaybackValue(int value); // -1 to disable
    int playbackValue() const { return m_playbackVal; }
    
    int minimum() const { return m_min; }
    int maximum() const { return m_max; }
    int lowerValue() const { return qMin(m_pos1, m_pos2); }
    int upperValue() const { return qMax(m_pos1, m_pos2); }

signals:
    void valuesChanged(int lower, int upper);
    void valuePreview(int value); 

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    int valToPos(int val) const;
    int posToVal(int pos) const;
    void drawHandle(QPainter &p, int val, bool highlight);

    int m_min = 0;
    int m_max = 100;
    int m_pos1 = 0;
    int m_pos2 = 100;
    int m_playbackVal = -1; // Added
    
    int m_handleWidth = 16;
    int m_margin = 10;
    
    int m_draggingHandle = 0; 
};
