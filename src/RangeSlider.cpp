#include "RangeSlider.h"
#include <QStyleOption>
#include <QToolTip>

RangeSlider::RangeSlider(QWidget *parent) : QWidget(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(30);
    setMouseTracking(false); 
}

void RangeSlider::setRange(int min, int max) {
    m_min = min;
    m_max = max;
    if (m_max < m_min) m_max = m_min;
    update();
}

void RangeSlider::setValues(int lower, int upper) {
    m_pos1 = lower;
    m_pos2 = upper;
    update();
    emit valuesChanged(lowerValue(), upperValue());
}

void RangeSlider::setPlaybackValue(int value) {
    m_playbackVal = value;
    update();
}

void RangeSlider::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int cy = height() / 2;
    int xStart = valToPos(lowerValue());
    int xEnd = valToPos(upperValue());

    // Groove (Background)
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#404040")); 
    p.drawRoundedRect(m_margin, cy - 2, width() - 2 * m_margin, 4, 2, 2);

    // Selection Range
    p.setBrush(QColor("#0078d7")); // Highlighted Range (Blue)
    p.drawRoundedRect(xStart, cy - 2, xEnd - xStart, 4, 2, 2);

    // Playback Progress
    if (m_playbackVal >= lowerValue()) {
        int effectiveVal = qMin(m_playbackVal, upperValue());
        int xPlay = valToPos(effectiveVal);
        // Progress Color (Cyan/Bright)
        p.setBrush(QColor("#00bcd4"));
        p.setPen(Qt::NoPen); // Ensure no pen
        if (xPlay > xStart)
            p.drawRoundedRect(xStart, cy - 2, xPlay - xStart, 4, 2, 2);
    }
    
    // Debug: Draw playhead (Red line)
    // int xHead = valToPos(m_playbackVal);
    // p.setPen(QPen(Qt::red, 1));
    // p.drawLine(xHead, cy - 10, xHead, cy + 10);

    // Handles
    drawHandle(p, m_pos1, m_draggingHandle == 1);
    drawHandle(p, m_pos2, m_draggingHandle == 2);
}

void RangeSlider::drawHandle(QPainter &p, int val, bool highlight) {
    int x = valToPos(val);
    int cy = height() / 2;
    
    p.setPen(QColor("#cccccc"));
    p.setBrush(highlight ? Qt::white : QColor("#e0e0e0"));
    p.drawEllipse(QPoint(x, cy), 8, 8);
    
    if (highlight) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor("#0078d7"), 2));
        p.drawEllipse(QPoint(x, cy), 8, 8);
    }
}

int RangeSlider::valToPos(int val) const {
    if (m_max == m_min) return m_margin;
    double ratio = (double)(val - m_min) / (m_max - m_min);
    int w = width() - 2 * m_margin;
    return m_margin + (int)(ratio * w);
}

int RangeSlider::posToVal(int pos) const {
    int w = width() - 2 * m_margin;
    if (w <= 0) return m_min;
    
    int relX = pos - m_margin;
    if (relX < 0) relX = 0;
    if (relX > w) relX = w;
    
    double ratio = (double)relX / w;
    return m_min + (int)(ratio * (m_max - m_min));
}

void RangeSlider::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        int x = event->pos().x();
        int val = posToVal(x);
        
        int x1 = valToPos(m_pos1);
        int x2 = valToPos(m_pos2);
        
        int dist1 = abs(x - x1);
        int dist2 = abs(x - x2);
        
        // Always pick closest
        if (dist1 <= dist2) {
            m_draggingHandle = 1;
            m_pos1 = val;
        } else {
            m_draggingHandle = 2;
            m_pos2 = val;
        }
        
        update();
        emit valuesChanged(lowerValue(), upperValue());
        emit valuePreview(m_draggingHandle == 1 ? m_pos1 : m_pos2);
    }
}

void RangeSlider::mouseMoveEvent(QMouseEvent *event) {
    if (m_draggingHandle != 0) {
        int val = posToVal(event->pos().x());
        if (m_draggingHandle == 1) m_pos1 = val;
        else m_pos2 = val;
        
        update();
        emit valuePreview(val);
        emit valuesChanged(lowerValue(), upperValue());
    } else if (event->buttons() & Qt::LeftButton) {
        // Dragging seek
        int val = posToVal(event->pos().x());
        emit valuePreview(val);
    }
}

void RangeSlider::mouseReleaseEvent(QMouseEvent *) {
    // Ensure final position triggers preview even if the user just clicked (or stopped exactly at a value)
    if (m_draggingHandle == 1) emit valuePreview(m_pos1);
    else if (m_draggingHandle == 2) emit valuePreview(m_pos2);
    m_draggingHandle = 0;
    update();
}

void RangeSlider::resizeEvent(QResizeEvent *) {
    update();
}
