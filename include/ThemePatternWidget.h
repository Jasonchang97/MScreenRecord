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
            drawSakura(p, QColor(255, 183, 197)); // 粉色樱花
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
    
    // 绘制四角星装饰（太阳形状）
    void drawSunStar(QPainter &p, double cx, double cy, double r) {
        p.setPen(Qt::NoPen);
        // 中心圆
        p.drawEllipse(QPointF(cx, cy), r * 0.6, r * 0.6);
        // 光芒
        for (int i = 0; i < 8; ++i) {
            double angle = i * M_PI / 4;
            double x1 = cx + r * 0.5 * qCos(angle);
            double y1 = cy + r * 0.5 * qSin(angle);
            double x2 = cx + r * qCos(angle);
            double y2 = cy + r * qSin(angle);
            QPainterPath ray;
            ray.moveTo(x1 - r*0.15*qSin(angle), y1 + r*0.15*qCos(angle));
            ray.lineTo(x2, y2);
            ray.lineTo(x1 + r*0.15*qSin(angle), y1 - r*0.15*qCos(angle));
            ray.closeSubpath();
            p.drawPath(ray);
        }
    }
    
    // 绘制彩虹
    void drawRainbow(QPainter &p, double cx, double cy, double size) {
        double s = size;
        QColor colors[] = {
            QColor(255, 100, 100), // 红
            QColor(255, 180, 100), // 橙
            QColor(255, 240, 120), // 黄
            QColor(150, 220, 150), // 绿
            QColor(130, 180, 255), // 蓝
            QColor(180, 150, 220)  // 紫
        };
        
        p.setPen(Qt::NoPen);
        for (int i = 0; i < 6; ++i) {
            p.setBrush(colors[i]);
            double r = (6 - i) * 1.5 * s;
            p.drawChord(QRectF(cx - r, cy - r/2, r*2, r), 0, 180*16);
        }
        // 白色遮挡内部
        p.setBrush(QColor(255, 245, 250));
        p.drawChord(QRectF(cx - 2*s, cy - 1*s, 4*s, 2*s), 0, 180*16);
    }
    
    // 绘制气球
    void drawBalloon(QPainter &p, double cx, double cy, double size, QColor color) {
        double s = size;
        p.setPen(QPen(color.darker(110), 0.3*s));
        p.setBrush(color);
        
        // 气球身体
        p.drawEllipse(QPointF(cx, cy), 3*s, 4*s);
        
        // 气球底部小三角
        QPainterPath knot;
        knot.moveTo(cx - 0.8*s, cy + 4*s);
        knot.lineTo(cx, cy + 5*s);
        knot.lineTo(cx + 0.8*s, cy + 4*s);
        knot.closeSubpath();
        p.drawPath(knot);
        
        // 气球线
        p.setPen(QPen(QColor(150, 150, 150), 0.2*s));
        p.setBrush(Qt::NoBrush);
        QPainterPath string;
        string.moveTo(cx, cy + 5*s);
        string.cubicTo(cx - 1*s, cy + 8*s, cx + 1*s, cy + 11*s, cx, cy + 14*s);
        p.drawPath(string);
        
        // 高光
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 120));
        p.drawEllipse(QPointF(cx - 1*s, cy - 1.5*s), 0.8*s, 1*s);
    }
    
    // ===== 简化版Q萌独角兽 - 参照新图片 =====
    
    // 类型1：白色独角兽（紫色鬃毛）
    void drawWhiteUnicorn(QPainter &p, double s, QColor outlineColor) {
        QColor bodyColor(255, 255, 255);
        QColor maneColor(190, 170, 210);  // 淡紫色
        QColor hornColor(255, 200, 80);
        
        p.setPen(QPen(outlineColor, 0.3*s));
        
        // === 尾巴（简化） ===
        p.setBrush(maneColor);
        p.drawEllipse(QPointF(16*s, 2*s), 6*s, 4*s);
        
        // === 腿（极短） ===
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(8*s, 12*s), 3*s, 2*s);   // 后右腿
        p.drawEllipse(QPointF(4*s, 12*s), 3*s, 2*s);   // 后左腿
        p.drawEllipse(QPointF(-4*s, 12*s), 3*s, 2*s);  // 前右腿
        p.drawEllipse(QPointF(-8*s, 12*s), 3*s, 2*s);  // 前左腿
        
        // === 身体 - 单一大椭圆 ===
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(0, 0), 18*s, 12*s);
        
        // === 鼻子（小椭圆） ===
        p.drawEllipse(QPointF(-16*s, -1*s), 3*s, 2*s);
        
        // === 耳朵（小三角） ===
        p.setBrush(bodyColor);
        QPainterPath ear1;
        ear1.moveTo(-12*s, -8*s);
        ear1.lineTo(-14*s, -14*s);
        ear1.lineTo(-10*s, -10*s);
        ear1.closeSubpath();
        p.drawPath(ear1);
        
        QPainterPath ear2;
        ear2.moveTo(-6*s, -8*s);
        ear2.lineTo(-4*s, -14*s);
        ear2.lineTo(-8*s, -10*s);
        ear2.closeSubpath();
        p.drawPath(ear2);
        
        // === 角（短小） ===
        p.setBrush(hornColor);
        QPainterPath horn;
        horn.moveTo(-9*s, -8*s);
        horn.lineTo(-7*s, -16*s);
        horn.lineTo(-5*s, -8*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // === 鬃毛（简化波浪） ===
        p.setBrush(maneColor);
        QPainterPath mane;
        mane.moveTo(-2*s, -8*s);
        mane.cubicTo(4*s, -6*s, 8*s, -2*s, 6*s, 4*s);
        mane.cubicTo(4*s, 8*s, 2*s, 6*s, 0, 2*s);
        mane.cubicTo(-2*s, -2*s, -2*s, -6*s, -2*s, -8*s);
        p.drawPath(mane);
        
        // === 眼睛 - 两个小黑点 ===
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(20, 20, 30));
        p.drawEllipse(QPointF(-12*s, -2*s), 1*s, 1*s);
        p.drawEllipse(QPointF(-8*s, -2*s), 1*s, 1*s);
        
        // === 腮红 ===
        p.setBrush(QColor(255, 200, 200, 100));
        p.drawEllipse(QPointF(-16*s, 1*s), 2*s, 1*s);
    }
    
    // 类型2：粉色飞马（有翅膀，戴花）
    void drawPinkPegasus(QPainter &p, double s, QColor outlineColor) {
        QColor bodyColor(255, 210, 220);  // 粉色
        QColor maneColor(255, 180, 200);  // 深粉
        QColor hornColor(255, 200, 80);
        
        p.setPen(QPen(outlineColor, 0.3*s));
        
        // === 尾巴（简化） ===
        p.setBrush(maneColor);
        p.drawEllipse(QPointF(16*s, 2*s), 6*s, 4*s);
        
        // === 腿（极短） ===
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(8*s, 12*s), 3*s, 2*s);
        p.drawEllipse(QPointF(4*s, 12*s), 3*s, 2*s);
        p.drawEllipse(QPointF(-4*s, 12*s), 3*s, 2*s);
        p.drawEllipse(QPointF(-8*s, 12*s), 3*s, 2*s);
        
        // === 身体 - 单一大椭圆 ===
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(0, 0), 18*s, 12*s);
        
        // === 翅膀（简化） ===
        p.setBrush(QColor(255, 255, 255, 200));
        p.drawEllipse(QPointF(6*s, -3*s), 8*s, 6*s);
        
        // === 鼻子 ===
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(-16*s, -1*s), 3*s, 2*s);
        
        // === 耳朵 ===
        QPainterPath ear1;
        ear1.moveTo(-12*s, -8*s);
        ear1.lineTo(-14*s, -14*s);
        ear1.lineTo(-10*s, -10*s);
        ear1.closeSubpath();
        p.drawPath(ear1);
        
        QPainterPath ear2;
        ear2.moveTo(-6*s, -8*s);
        ear2.lineTo(-4*s, -14*s);
        ear2.lineTo(-8*s, -10*s);
        ear2.closeSubpath();
        p.drawPath(ear2);
        
        // === 角 ===
        p.setBrush(hornColor);
        QPainterPath horn;
        horn.moveTo(-9*s, -8*s);
        horn.lineTo(-7*s, -16*s);
        horn.lineTo(-5*s, -8*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // === 鬃毛 ===
        p.setBrush(maneColor);
        QPainterPath mane;
        mane.moveTo(-2*s, -8*s);
        mane.cubicTo(4*s, -6*s, 8*s, -2*s, 6*s, 4*s);
        mane.cubicTo(4*s, 8*s, 2*s, 6*s, 0, 2*s);
        mane.cubicTo(-2*s, -2*s, -2*s, -6*s, -2*s, -8*s);
        p.drawPath(mane);
        
        // === 头顶小花 ===
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 80, 120));
        p.drawEllipse(QPointF(-10*s, -10*s), 2*s, 2*s);
        p.setBrush(QColor(255, 200, 100));
        p.drawEllipse(QPointF(-10*s, -10*s), 0.8*s, 0.8*s);
        
        // === 眼睛 ===
        p.setBrush(QColor(20, 20, 30));
        p.drawEllipse(QPointF(-12*s, -2*s), 1*s, 1*s);
        p.drawEllipse(QPointF(-8*s, -2*s), 1*s, 1*s);
        
        // === 腮红 ===
        p.setBrush(QColor(255, 160, 180, 100));
        p.drawEllipse(QPointF(-16*s, 1*s), 2*s, 1*s);
    }
    
    // 类型3：黄色独角兽（戴蝴蝶结，拿气球）
    void drawYellowUnicorn(QPainter &p, double s, QColor outlineColor) {
        QColor bodyColor(255, 245, 180);  // 奶黄色
        QColor maneColor(255, 235, 150);  // 浅黄
        QColor hornColor(255, 200, 80);
        QColor bowColor(80, 160, 255);    // 蓝色蝴蝶结
        
        p.setPen(QPen(outlineColor, 0.3*s));
        
        // === 气球（右上角） ===
        drawBalloon(p, 14*s, -12*s, 1.2*s, QColor(80, 160, 255));
        drawBalloon(p, 18*s, -10*s, 1.0*s, QColor(255, 120, 150));
        
        // === 尾巴 ===
        p.setBrush(maneColor);
        p.drawEllipse(QPointF(16*s, 2*s), 6*s, 4*s);
        
        // === 腿 ===
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(8*s, 12*s), 3*s, 2*s);
        p.drawEllipse(QPointF(4*s, 12*s), 3*s, 2*s);
        p.drawEllipse(QPointF(-4*s, 12*s), 3*s, 2*s);
        p.drawEllipse(QPointF(-8*s, 12*s), 3*s, 2*s);
        
        // === 身体 ===
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(0, 0), 18*s, 12*s);
        
        // === 鼻子 ===
        p.drawEllipse(QPointF(-16*s, -1*s), 3*s, 2*s);
        
        // === 耳朵 ===
        QPainterPath ear1;
        ear1.moveTo(-12*s, -8*s);
        ear1.lineTo(-14*s, -14*s);
        ear1.lineTo(-10*s, -10*s);
        ear1.closeSubpath();
        p.drawPath(ear1);
        
        QPainterPath ear2;
        ear2.moveTo(-6*s, -8*s);
        ear2.lineTo(-4*s, -14*s);
        ear2.lineTo(-8*s, -10*s);
        ear2.closeSubpath();
        p.drawPath(ear2);
        
        // === 角 ===
        p.setBrush(hornColor);
        QPainterPath horn;
        horn.moveTo(-9*s, -8*s);
        horn.lineTo(-7*s, -16*s);
        horn.lineTo(-5*s, -8*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // === 鬃毛 ===
        p.setBrush(maneColor);
        QPainterPath mane;
        mane.moveTo(-2*s, -8*s);
        mane.cubicTo(4*s, -6*s, 8*s, -2*s, 6*s, 4*s);
        mane.cubicTo(4*s, 8*s, 2*s, 6*s, 0, 2*s);
        mane.cubicTo(-2*s, -2*s, -2*s, -6*s, -2*s, -8*s);
        p.drawPath(mane);
        
        // === 蝴蝶结（简化） ===
        p.setPen(Qt::NoPen);
        p.setBrush(bowColor);
        p.drawEllipse(QPointF(-13*s, -6*s), 2.5*s, 1.5*s);
        p.drawEllipse(QPointF(-7*s, -6*s), 2.5*s, 1.5*s);
        p.setBrush(bowColor.darker(110));
        p.drawEllipse(QPointF(-10*s, -6*s), 1*s, 1*s);
        
        // === 眼睛 ===
        p.setBrush(QColor(20, 20, 30));
        p.drawEllipse(QPointF(-12*s, -2*s), 1*s, 1*s);
        p.drawEllipse(QPointF(-8*s, -2*s), 1*s, 1*s);
        
        // === 腮红 ===
        p.setBrush(QColor(255, 200, 150, 100));
        p.drawEllipse(QPointF(-16*s, 1*s), 2*s, 1*s);
    }
    
    // 类型4：淡紫色独角兽
    void drawPurpleUnicorn(QPainter &p, double s, QColor outlineColor) {
        QColor bodyColor(235, 225, 255);  // 淡紫色
        QColor maneColor(200, 180, 230);  // 紫色鬃毛
        QColor hornColor(255, 200, 80);
        
        p.setPen(QPen(outlineColor, 0.3*s));
        
        // === 尾巴 ===
        p.setBrush(maneColor);
        p.drawEllipse(QPointF(16*s, 2*s), 6*s, 4*s);
        
        // === 腿 ===
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(8*s, 12*s), 3*s, 2*s);
        p.drawEllipse(QPointF(4*s, 12*s), 3*s, 2*s);
        p.drawEllipse(QPointF(-4*s, 12*s), 3*s, 2*s);
        p.drawEllipse(QPointF(-8*s, 12*s), 3*s, 2*s);
        
        // === 身体 ===
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(0, 0), 18*s, 12*s);
        
        // === 鼻子 ===
        p.drawEllipse(QPointF(-16*s, -1*s), 3*s, 2*s);
        
        // === 耳朵 ===
        QPainterPath ear1;
        ear1.moveTo(-12*s, -8*s);
        ear1.lineTo(-14*s, -14*s);
        ear1.lineTo(-10*s, -10*s);
        ear1.closeSubpath();
        p.drawPath(ear1);
        
        QPainterPath ear2;
        ear2.moveTo(-6*s, -8*s);
        ear2.lineTo(-4*s, -14*s);
        ear2.lineTo(-8*s, -10*s);
        ear2.closeSubpath();
        p.drawPath(ear2);
        
        // === 角 ===
        p.setBrush(hornColor);
        QPainterPath horn;
        horn.moveTo(-9*s, -8*s);
        horn.lineTo(-7*s, -16*s);
        horn.lineTo(-5*s, -8*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // === 鬃毛 ===
        p.setBrush(maneColor);
        QPainterPath mane;
        mane.moveTo(-2*s, -8*s);
        mane.cubicTo(4*s, -6*s, 8*s, -2*s, 6*s, 4*s);
        mane.cubicTo(4*s, 8*s, 2*s, 6*s, 0, 2*s);
        mane.cubicTo(-2*s, -2*s, -2*s, -6*s, -2*s, -8*s);
        p.drawPath(mane);
        
        // === 眼睛 ===
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(20, 20, 30));
        p.drawEllipse(QPointF(-12*s, -2*s), 1*s, 1*s);
        p.drawEllipse(QPointF(-8*s, -2*s), 1*s, 1*s);
        
        // === 腮红 ===
        p.setBrush(QColor(255, 200, 220, 100));
        p.drawEllipse(QPointF(-16*s, 1*s), 2*s, 1*s);
    }
    
    // 绘制独角兽图案（自绘制版本）
    void drawUnicorns(QPainter &p) {
        int w = width();
        int h = height();
        
        int area = w * h;
        
        // 根据窗口大小调整密度
        int unicornCount;
        if (area < 200000) {
            unicornCount = qBound(2, area / 50000, 4);
        } else {
            unicornCount = qBound(5, area / 35000, 18);
        }
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        QColor outlineColor(180, 160, 170);
        
        for (int i = 0; i < unicornCount; ++i) {
            double scale = 0.8 + (qrand() % 13) / 10.0;
            int size = static_cast<int>(35 * scale);
            
            int x, y;
            bool found = false;
            
            for (int attempt = 0; attempt < unicornCount * 20; ++attempt) {
                x = size + qrand() % qMax(1, w - size * 2);
                y = size + qrand() % qMax(1, h - size * 2);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size, y - size, size * 2, size * 2));
            
            p.setOpacity(0.85 + (qrand() % 15) / 100.0);
            
            p.save();
            p.translate(x, y);
            if (qrand() % 2 == 0) p.scale(-1, 1);
            
            // 随机选择4种独角兽类型
            int type = qrand() % 4;
            switch (type) {
                case 0: drawWhiteUnicorn(p, scale, outlineColor); break;
                case 1: drawPinkPegasus(p, scale, outlineColor); break;
                case 2: drawYellowUnicorn(p, scale, outlineColor); break;
                case 3: drawPurpleUnicorn(p, scale, outlineColor); break;
            }
            
            p.restore();
        }
        
        // 添加彩虹装饰
        int rainbowCount = qBound(1, area / 150000, 4);
        for (int i = 0; i < rainbowCount; ++i) {
            int rx = 40 + qrand() % qMax(1, w - 80);
            int ry = 30 + qrand() % qMax(1, h - 60);
            if (!isOverlapping(rx, ry, 30, occupiedRects)) {
                p.setOpacity(0.6 + (qrand() % 20) / 100.0);
                double rs = 0.8 + (qrand() % 10) / 10.0;
                drawRainbow(p, rx, ry, 4 * rs);
                occupiedRects.append(QRect(rx - 20, ry - 10, 40, 20));
            }
        }
        
        // 添加太阳装饰
        int sunCount = qBound(1, area / 100000, 5);
        p.setBrush(QColor(255, 230, 100));
        for (int i = 0; i < sunCount; ++i) {
            int sx = 20 + qrand() % qMax(1, w - 40);
            int sy = 20 + qrand() % qMax(1, h - 40);
            double sr = 6 + qrand() % 8;
            if (!isOverlapping(sx, sy, static_cast<int>(sr * 2), occupiedRects)) {
                p.setOpacity(0.7 + (qrand() % 20) / 100.0);
                drawSunStar(p, sx, sy, sr);
            }
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制樱花图案（粉色主题）
    void drawSakura(QPainter &p, QColor sakuraColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        // 小窗口低密度，大窗口高密度
        int sakuraCount = (area < 200000) ? qBound(5, area / 15000, 12) : qBound(15, area / 5000, 80);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < sakuraCount; ++i) {
            int x, y;
            int size = 12 + qrand() % 35; // 12-46px
            bool found = false;
            
            for (int attempt = 0; attempt < sakuraCount * 10; ++attempt) {
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
            
            p.save();
            p.translate(x, y);
            p.rotate(qrand() % 360); // 随机旋转
            
            double s = size / 30.0;
            
            // 绘制5片花瓣
            for (int petal = 0; petal < 5; ++petal) {
                p.save();
                p.rotate(petal * 72); // 每片花瓣相隔72度
                
                // 花瓣颜色渐变：外深内浅
                QColor petalColor = sakuraColor;
                petalColor.setAlpha(140 + qrand() % 60);
                
                QLinearGradient gradient(0, -12*s, 0, 0);
                gradient.setColorAt(0, petalColor);
                gradient.setColorAt(1, petalColor.lighter(120));
                
                p.setPen(QPen(sakuraColor.darker(110), 0.3*s));
                p.setBrush(gradient);
                
                // 樱花花瓣形状：顶端有凹口
                QPainterPath petal_path;
                petal_path.moveTo(0, -2*s);
                petal_path.cubicTo(4*s, -5*s, 5*s, -10*s, 2*s, -13*s);
                petal_path.quadTo(0, -11*s, -2*s, -13*s); // 凹口
                petal_path.cubicTo(-5*s, -10*s, -4*s, -5*s, 0, -2*s);
                p.drawPath(petal_path);
                
                p.restore();
            }
            
            // 花蕊 - 中心黄色小圆点
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 220, 100, 180));
            p.drawEllipse(QPointF(0, 0), 2*s, 2*s);
            
            // 花蕊细节 - 小点
            p.setBrush(QColor(255, 180, 80, 150));
            for (int dot = 0; dot < 5; ++dot) {
                double angle = dot * 72 * M_PI / 180;
                double dx = 1.2*s * qCos(angle);
                double dy = 1.2*s * qSin(angle);
                p.drawEllipse(QPointF(dx, dy), 0.4*s, 0.4*s);
            }
            
            p.restore();
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制星星图案（紫色主题）
    void drawStars(QPainter &p, QColor starColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        // 小窗口低密度，大窗口高密度
        int starCount = (area < 200000) ? qBound(5, area / 15000, 12) : qBound(15, area / 5000, 80);
        
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
        // 小窗口低密度，大窗口高密度
        int leafCount = (area < 200000) ? qBound(4, area / 18000, 10) : qBound(12, area / 6000, 60);
        
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
        // 小窗口低密度，大窗口高密度
        int dotCount = (area < 200000) ? qBound(8, area / 10000, 20) : qBound(25, area / 3000, 120);
        
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
        // 小窗口低密度，大窗口高密度
        int heartCount = (area < 200000) ? qBound(5, area / 15000, 12) : qBound(15, area / 5000, 80);
        
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
