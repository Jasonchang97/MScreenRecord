#pragma once

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QDateTime>
#include <QtMath>
#include "SettingsDialog.h"

class ThemePatternWidget : public QWidget {
public:
    ThemePatternWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        
        QString theme = SettingsDialog::getTheme();
        
        if (theme == "zijunpink") {
            drawUnicorns(p);
        } else if (theme == "zijunwhite") {
            drawHearts(p, QColor(230, 199, 192));
        } else if (theme == "pink") {
            drawBubbles(p, QColor(255, 182, 193)); // 粉色泡泡
        } else if (theme == "purple") {
            drawStars(p, QColor(180, 160, 220)); // 紫色星星
        } else if (theme == "green") {
            drawLeaves(p, QColor(144, 238, 144)); // 绿色叶子
        } else if (theme == "tech") {
            drawTechDots(p, QColor(0, 200, 255)); // 科技点阵
        }
    }
    
private:
    // 检查是否重叠
    bool isOverlapping(int x, int y, int size, const QList<QRect> &occupied) {
        QRect newRect(x - size/2 - 5, y - size/2 - 5, size + 10, size + 10);
        for (const QRect &rect : occupied) {
            if (newRect.intersects(rect)) {
                return true;
            }
        }
        return false;
    }
    
    // 绘制五角星
    void drawStar(QPainter &p, double cx, double cy, double r) {
        QPainterPath star;
        for (int i = 0; i < 5; ++i) {
            double angle = -M_PI / 2 + i * 4 * M_PI / 5;
            double x = cx + r * qCos(angle);
            double y = cy + r * qSin(angle);
            if (i == 0) star.moveTo(x, y);
            else star.lineTo(x, y);
        }
        star.closeSubpath();
        p.drawPath(star);
    }
    
    // 绘制简化版独角兽 - Q版可爱风格
    void drawUnicorn(QPainter &p, int cx, int cy, double scale, int colorIndex, bool flip) {
        p.save();
        p.translate(cx, cy);
        if (flip) p.scale(-1, 1);
        
        double s = scale;
        
        // 颜色方案
        QColor bodyColor, maneColor, hornColor, outlineColor;
        switch (colorIndex % 4) {
            case 0: // 粉色系
                bodyColor = QColor(255, 240, 245);
                maneColor = QColor(255, 182, 193);
                hornColor = QColor(255, 215, 0);
                outlineColor = QColor(200, 150, 160);
                break;
            case 1: // 紫色系
                bodyColor = QColor(245, 240, 255);
                maneColor = QColor(200, 162, 200);
                hornColor = QColor(255, 200, 100);
                outlineColor = QColor(160, 140, 180);
                break;
            case 2: // 蓝色系
                bodyColor = QColor(240, 248, 255);
                maneColor = QColor(173, 216, 230);
                hornColor = QColor(255, 223, 100);
                outlineColor = QColor(150, 170, 190);
                break;
            case 3: // 米色系
                bodyColor = QColor(255, 250, 240);
                maneColor = QColor(230, 200, 180);
                hornColor = QColor(255, 210, 80);
                outlineColor = QColor(180, 160, 140);
                break;
        }
        
        p.setPen(QPen(outlineColor, 0.8*s));
        
        // 尾巴 - 波浪形
        p.setBrush(maneColor);
        QPainterPath tail;
        tail.moveTo(12*s, -2*s);
        tail.cubicTo(16*s, -6*s, 20*s, -2*s, 18*s, 4*s);
        tail.cubicTo(16*s, 8*s, 14*s, 6*s, 12*s, 2*s);
        p.drawPath(tail);
        
        // 后腿
        p.setBrush(bodyColor);
        p.drawRoundedRect(QRectF(6*s, 4*s, 4*s, 10*s), 2*s, 2*s);
        p.drawRoundedRect(QRectF(2*s, 5*s, 4*s, 9*s), 2*s, 2*s);
        
        // 前腿
        p.drawRoundedRect(QRectF(-6*s, 4*s, 4*s, 10*s), 2*s, 2*s);
        p.drawRoundedRect(QRectF(-10*s, 5*s, 4*s, 9*s), 2*s, 2*s);
        
        // 蹄子
        p.setBrush(QColor(180, 180, 190));
        p.drawEllipse(QPointF(8*s, 14*s), 2*s, 1.5*s);
        p.drawEllipse(QPointF(4*s, 14*s), 2*s, 1.5*s);
        p.drawEllipse(QPointF(-4*s, 14*s), 2*s, 1.5*s);
        p.drawEllipse(QPointF(-8*s, 14*s), 2*s, 1.5*s);
        
        // 身体 - 椭圆
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(0, 0), 12*s, 8*s);
        
        // 脖子
        QPainterPath neck;
        neck.moveTo(-8*s, -4*s);
        neck.cubicTo(-10*s, -8*s, -12*s, -12*s, -10*s, -16*s);
        neck.lineTo(-6*s, -14*s);
        neck.cubicTo(-8*s, -10*s, -6*s, -6*s, -4*s, -4*s);
        neck.closeSubpath();
        p.drawPath(neck);
        
        // 头部 - 圆形
        p.drawEllipse(QPointF(-10*s, -18*s), 6*s, 5*s);
        
        // 耳朵
        QPainterPath ear;
        ear.moveTo(-12*s, -22*s);
        ear.lineTo(-14*s, -28*s);
        ear.lineTo(-10*s, -24*s);
        ear.closeSubpath();
        p.drawPath(ear);
        
        QPainterPath ear2;
        ear2.moveTo(-8*s, -22*s);
        ear2.lineTo(-6*s, -27*s);
        ear2.lineTo(-6*s, -23*s);
        ear2.closeSubpath();
        p.drawPath(ear2);
        
        // 角
        p.setBrush(hornColor);
        QPainterPath horn;
        horn.moveTo(-10*s, -23*s);
        horn.lineTo(-8*s, -32*s);
        horn.lineTo(-6*s, -23*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // 角上的纹路
        p.setPen(QPen(QColor(255, 180, 100), 0.5*s));
        p.drawLine(QPointF(-9.5*s, -25*s), QPointF(-7*s, -25*s));
        p.drawLine(QPointF(-9*s, -27*s), QPointF(-7.5*s, -27*s));
        p.drawLine(QPointF(-8.5*s, -29*s), QPointF(-8*s, -29*s));
        
        // 鬃毛
        p.setPen(QPen(outlineColor, 0.6*s));
        p.setBrush(maneColor);
        QPainterPath mane;
        mane.moveTo(-6*s, -22*s);
        mane.cubicTo(-2*s, -20*s, 0*s, -16*s, -2*s, -12*s);
        mane.cubicTo(-4*s, -8*s, -2*s, -4*s, 0*s, -2*s);
        mane.lineTo(-4*s, -4*s);
        mane.cubicTo(-6*s, -8*s, -8*s, -12*s, -6*s, -16*s);
        mane.cubicTo(-4*s, -18*s, -6*s, -20*s, -6*s, -22*s);
        mane.closeSubpath();
        p.drawPath(mane);
        
        // 眼睛
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(60, 60, 80));
        p.drawEllipse(QPointF(-12*s, -18*s), 1.5*s, 2*s);
        
        // 眼睛高光
        p.setBrush(Qt::white);
        p.drawEllipse(QPointF(-12.5*s, -19*s), 0.6*s, 0.6*s);
        
        // 腮红
        p.setBrush(QColor(255, 180, 180, 100));
        p.drawEllipse(QPointF(-14*s, -16*s), 2*s, 1.2*s);
        
        p.restore();
    }
    
    // 绘制独角兽图案（自绘制版本）
    void drawUnicorns(QPainter &p) {
        int w = width();
        int h = height();
        
        int area = w * h;
        int unicornCount = qBound(3, area / 60000, 12);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < unicornCount; ++i) {
            // 大小变化范围更大：0.8 到 2.0
            double scale = 0.8 + (qrand() % 13) / 10.0;
            int size = static_cast<int>(40 * scale);
            
            int x, y;
            bool found = false;
            
            for (int attempt = 0; attempt < unicornCount * 15; ++attempt) {
                x = size + qrand() % qMax(1, w - size * 2);
                y = size + qrand() % qMax(1, h - size * 2);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size, y - size, size * 2, size * 2));
            
            p.setOpacity(0.15 + (qrand() % 15) / 100.0);
            
            int colorIndex = qrand() % 4;
            bool flip = qrand() % 2 == 0;
            
            drawUnicorn(p, x, y, scale, colorIndex, flip);
        }
        
        // 添加小星星装饰
        p.setOpacity(0.25);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 215, 100));
        
        int starCount = qBound(5, area / 40000, 20);
        for (int i = 0; i < starCount; ++i) {
            int sx = 15 + qrand() % qMax(1, w - 30);
            int sy = 15 + qrand() % qMax(1, h - 30);
            double sr = 3 + qrand() % 8; // 大小变化：3-10
            
            if (!isOverlapping(sx, sy, static_cast<int>(sr * 2), occupiedRects)) {
                drawStar(p, sx, sy, sr);
            }
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制泡泡图案（粉色主题）
    void drawBubbles(QPainter &p, QColor bubbleColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        int bubbleCount = qBound(20, area / 4000, 100);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < bubbleCount; ++i) {
            int x, y;
            int size = 8 + qrand() % 35; // 8-42px
            bool found = false;
            
            for (int attempt = 0; attempt < bubbleCount * 8; ++attempt) {
                x = size + qrand() % qMax(1, w - size * 2);
                y = size + qrand() % qMax(1, h - size * 2);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            p.setOpacity(0.1 + (qrand() % 20) / 100.0);
            
            QColor color = bubbleColor;
            color.setAlpha(100 + qrand() % 100);
            
            // 绘制泡泡
            QRadialGradient gradient(x - size/4, y - size/4, size);
            gradient.setColorAt(0, color.lighter(130));
            gradient.setColorAt(0.7, color);
            gradient.setColorAt(1, color.darker(110));
            
            p.setPen(QPen(color.darker(120), 1));
            p.setBrush(gradient);
            p.drawEllipse(QPointF(x, y), size/2, size/2);
            
            // 高光
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 255, 255, 80));
            p.drawEllipse(QPointF(x - size/5, y - size/5), size/6, size/8);
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制星星图案（紫色主题）
    void drawStars(QPainter &p, QColor starColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        int starCount = qBound(15, area / 5000, 80);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < starCount; ++i) {
            int x, y;
            int size = 6 + qrand() % 25; // 6-30px
            bool found = false;
            
            for (int attempt = 0; attempt < starCount * 10; ++attempt) {
                x = size + qrand() % qMax(1, w - size * 2);
                y = size + qrand() % qMax(1, h - size * 2);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            p.setOpacity(0.15 + (qrand() % 20) / 100.0);
            
            QColor color = starColor;
            color.setAlpha(150 + qrand() % 80);
            p.setPen(Qt::NoPen);
            p.setBrush(color);
            
            p.save();
            p.translate(x, y);
            p.rotate(qrand() % 72); // 随机旋转
            drawStar(p, 0, 0, size/2);
            p.restore();
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制叶子图案（绿色主题）
    void drawLeaves(QPainter &p, QColor leafColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        int leafCount = qBound(12, area / 6000, 60);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < leafCount; ++i) {
            int x, y;
            int size = 15 + qrand() % 40; // 15-54px
            bool found = false;
            
            for (int attempt = 0; attempt < leafCount * 10; ++attempt) {
                x = size + qrand() % qMax(1, w - size * 2);
                y = size + qrand() % qMax(1, h - size * 2);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            p.setOpacity(0.12 + (qrand() % 18) / 100.0);
            
            QColor color = leafColor;
            color.setAlpha(140 + qrand() % 80);
            
            p.save();
            p.translate(x, y);
            p.rotate(-30 + qrand() % 60);
            
            double s = size / 30.0;
            
            // 绘制叶子
            p.setPen(QPen(color.darker(120), 0.5*s));
            p.setBrush(color);
            
            QPainterPath leaf;
            leaf.moveTo(0, -10*s);
            leaf.cubicTo(8*s, -8*s, 10*s, 0, 0, 10*s);
            leaf.cubicTo(-10*s, 0, -8*s, -8*s, 0, -10*s);
            p.drawPath(leaf);
            
            // 叶脉
            p.setPen(QPen(color.darker(130), 0.3*s));
            p.drawLine(QPointF(0, -8*s), QPointF(0, 8*s));
            p.drawLine(QPointF(0, -4*s), QPointF(4*s, -2*s));
            p.drawLine(QPointF(0, 0), QPointF(5*s, 2*s));
            p.drawLine(QPointF(0, 4*s), QPointF(4*s, 5*s));
            p.drawLine(QPointF(0, -4*s), QPointF(-4*s, -2*s));
            p.drawLine(QPointF(0, 0), QPointF(-5*s, 2*s));
            p.drawLine(QPointF(0, 4*s), QPointF(-4*s, 5*s));
            
            p.restore();
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制科技点阵（科技主题）
    void drawTechDots(QPainter &p, QColor dotColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        int dotCount = qBound(25, area / 3000, 120);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        QList<QPointF> dots;
        
        for (int i = 0; i < dotCount; ++i) {
            int x, y;
            int size = 4 + qrand() % 12; // 4-15px
            bool found = false;
            
            for (int attempt = 0; attempt < dotCount * 8; ++attempt) {
                x = size + qrand() % qMax(1, w - size * 2);
                y = size + qrand() % qMax(1, h - size * 2);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            dots.append(QPointF(x, y));
            
            p.setOpacity(0.15 + (qrand() % 25) / 100.0);
            
            QColor color = dotColor;
            color.setAlpha(120 + qrand() % 100);
            
            // 绘制发光点
            QRadialGradient gradient(x, y, size);
            gradient.setColorAt(0, color);
            gradient.setColorAt(0.5, color.darker(110));
            gradient.setColorAt(1, QColor(color.red(), color.green(), color.blue(), 0));
            
            p.setPen(Qt::NoPen);
            p.setBrush(gradient);
            p.drawEllipse(QPointF(x, y), size, size);
            
            // 中心亮点
            p.setBrush(color.lighter(150));
            p.drawEllipse(QPointF(x, y), size/4, size/4);
        }
        
        // 绘制连接线
        p.setOpacity(0.08);
        p.setPen(QPen(dotColor, 1));
        for (int i = 0; i < dots.size(); ++i) {
            for (int j = i + 1; j < dots.size(); ++j) {
                double dist = QLineF(dots[i], dots[j]).length();
                if (dist < 80) {
                    p.drawLine(dots[i], dots[j]);
                }
            }
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制爱心图案（子君白主题）
    void drawHearts(QPainter &p, QColor heartColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        int heartCount = qBound(15, area / 5000, 80); // 更高密度
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < heartCount; ++i) {
            int x, y;
            // 大小变化更大：10-50像素
            int size = 10 + qrand() % 40;
            bool found = false;
            
            for (int attempt = 0; attempt < heartCount * 10; ++attempt) {
                x = size + qrand() % qMax(1, w - size * 2);
                y = size + qrand() % qMax(1, h - size * 2);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            p.setOpacity(0.12 + (qrand() % 20) / 100.0); // 0.12-0.32
            
            QColor color = heartColor;
            color.setAlpha(160 + qrand() % 70);
            p.setPen(Qt::NoPen);
            p.setBrush(color);
            
            p.save();
            p.translate(x, y);
            double rotation = -20 + qrand() % 40;
            p.rotate(rotation);
            
            // 绘制爱心
            QPainterPath heart;
            double s = size / 20.0;
            heart.moveTo(0, 4*s);
            heart.cubicTo(-5*s, -2*s, -10*s, 2*s, 0, 10*s);
            heart.moveTo(0, 4*s);
            heart.cubicTo(5*s, -2*s, 10*s, 2*s, 0, 10*s);
            p.drawPath(heart);
            
            p.restore();
        }
        
        p.setOpacity(1.0);
    }
};
