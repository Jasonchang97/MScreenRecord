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
        setAttribute(Qt::WA_TransparentForMouseEvents); // Click through
        setAttribute(Qt::WA_NoSystemBackground); // Don't draw background
    }

protected:
    // 绘制可爱Q版独角兽
    void drawUnicorn(QPainter &p, int x, int y, int size, int alpha) {
        p.save();
        p.translate(x, y);
        
        // 随机选择独角兽的颜色组合
        int colorType = qrand() % 4;
        QColor bodyColor, maneColor, maneColor2, hornColor, accentColor, hoofColor;
        
        switch (colorType) {
            case 0: // 白色身体 + 粉色鬃毛（参考图左上木马）
                bodyColor = QColor(255, 255, 255, alpha);
                maneColor = QColor(255, 200, 210, alpha);
                maneColor2 = QColor(255, 180, 195, alpha);
                hornColor = QColor(255, 230, 150, alpha);
                accentColor = QColor(180, 220, 220, alpha); // 浅青蝴蝶结
                hoofColor = QColor(255, 220, 230, alpha);
                break;
            case 1: // 淡粉身体 + 紫色鬃毛（参考图右上）
                bodyColor = QColor(255, 235, 240, alpha);
                maneColor = QColor(200, 180, 210, alpha);
                maneColor2 = QColor(180, 160, 200, alpha);
                hornColor = QColor(255, 230, 150, alpha);
                accentColor = QColor(180, 220, 200, alpha);
                hoofColor = QColor(210, 190, 210, alpha);
                break;
            case 2: // 白色身体 + 彩虹渐变鬃毛（参考图右下飞马）
                bodyColor = QColor(255, 255, 255, alpha);
                maneColor = QColor(200, 180, 220, alpha); // 淡紫
                maneColor2 = QColor(180, 220, 210, alpha); // 淡青
                hornColor = QColor(255, 230, 150, alpha);
                accentColor = QColor(255, 200, 210, alpha);
                hoofColor = QColor(220, 210, 230, alpha);
                break;
            default: // 白色身体 + 粉紫鬃毛（参考图左下）
                bodyColor = QColor(255, 255, 255, alpha);
                maneColor = QColor(255, 210, 220, alpha);
                maneColor2 = QColor(230, 190, 210, alpha);
                hornColor = QColor(255, 235, 160, alpha);
                accentColor = QColor(180, 210, 200, alpha);
                hoofColor = QColor(255, 225, 235, alpha);
                break;
        }
        
        double s = size / 50.0; // 缩放因子
        
        p.setPen(Qt::NoPen);
        
        // === 绘制尾巴（先画，在身体后面）===
        p.setBrush(maneColor);
        QPainterPath tailPath;
        tailPath.moveTo(22*s, 5*s);
        tailPath.cubicTo(30*s, 0, 35*s, 8*s, 32*s, 18*s);
        tailPath.cubicTo(28*s, 25*s, 24*s, 22*s, 26*s, 15*s);
        tailPath.cubicTo(28*s, 10*s, 25*s, 8*s, 22*s, 10*s);
        p.drawPath(tailPath);
        // 尾巴第二层颜色
        p.setBrush(maneColor2);
        QPainterPath tail2;
        tail2.moveTo(24*s, 8*s);
        tail2.cubicTo(30*s, 5*s, 33*s, 12*s, 30*s, 20*s);
        tail2.cubicTo(27*s, 24*s, 25*s, 18*s, 26*s, 12*s);
        p.drawPath(tail2);
        
        // === 绘制后腿 ===
        p.setBrush(bodyColor);
        // 后腿（圆润的）
        p.drawEllipse(QPointF(12*s, 18*s), 6*s, 10*s);
        p.drawEllipse(QPointF(18*s, 18*s), 5*s, 9*s);
        // 后蹄
        p.setBrush(hoofColor);
        p.drawEllipse(QPointF(12*s, 26*s), 4*s, 3*s);
        p.drawEllipse(QPointF(18*s, 25*s), 3.5*s, 2.5*s);
        
        // === 绘制身体（圆润的椭圆）===
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(5*s, 5*s), 22*s, 16*s);
        
        // === 绘制前腿 ===
        p.setBrush(bodyColor);
        // 前腿（圆润的）
        p.drawEllipse(QPointF(-12*s, 16*s), 5*s, 10*s);
        p.drawEllipse(QPointF(-5*s, 17*s), 5*s, 9*s);
        // 前蹄
        p.setBrush(hoofColor);
        p.drawEllipse(QPointF(-12*s, 24*s), 3.5*s, 3*s);
        p.drawEllipse(QPointF(-5*s, 24*s), 3.5*s, 2.5*s);
        
        // === 绘制头部（大圆头）===
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(-18*s, -8*s), 14*s, 13*s);
        
        // 口鼻部分（小椭圆突出）
        p.drawEllipse(QPointF(-28*s, -4*s), 6*s, 5*s);
        
        // === 绘制耳朵 ===
        QPainterPath earPath;
        earPath.moveTo(-14*s, -18*s);
        earPath.quadTo(-12*s, -28*s, -8*s, -20*s);
        earPath.quadTo(-10*s, -18*s, -14*s, -18*s);
        p.drawPath(earPath);
        // 耳朵内部
        p.setBrush(QColor(255, 210, 220, alpha));
        QPainterPath earInner;
        earInner.moveTo(-13*s, -19*s);
        earInner.quadTo(-11.5*s, -25*s, -9.5*s, -20*s);
        earInner.quadTo(-11*s, -19*s, -13*s, -19*s);
        p.drawPath(earInner);
        
        // === 绘制独角 ===
        QLinearGradient hornGrad(-12*s, -22*s, -10*s, -38*s);
        hornGrad.setColorAt(0, hornColor);
        hornGrad.setColorAt(1, QColor(255, 245, 200, alpha));
        p.setBrush(hornGrad);
        QPainterPath hornPath;
        hornPath.moveTo(-14*s, -22*s);
        hornPath.lineTo(-11*s, -40*s);
        hornPath.lineTo(-8*s, -22*s);
        hornPath.closeSubpath();
        p.drawPath(hornPath);
        
        // 独角螺纹线
        p.setPen(QPen(QColor(240, 210, 130, alpha * 0.6), 0.8*s));
        for (int i = 0; i < 4; ++i) {
            double yOff = -25*s - i * 4*s;
            p.drawLine(QPointF(-13*s + i*0.3*s, yOff), QPointF(-9*s - i*0.3*s, yOff + 1*s));
        }
        p.setPen(Qt::NoPen);
        
        // === 绘制鬃毛（蓬松波浪状）===
        p.setBrush(maneColor);
        QPainterPath manePath;
        manePath.moveTo(-8*s, -18*s);
        manePath.cubicTo(-2*s, -25*s, 5*s, -22*s, 8*s, -16*s);
        manePath.cubicTo(12*s, -10*s, 8*s, -5*s, 4*s, -8*s);
        manePath.cubicTo(0*s, -10*s, -4*s, -8*s, -6*s, -12*s);
        manePath.cubicTo(-8*s, -15*s, -6*s, -17*s, -8*s, -18*s);
        p.drawPath(manePath);
        
        // 鬃毛第二层（不同颜色增加层次）
        p.setBrush(maneColor2);
        QPainterPath mane2;
        mane2.moveTo(-5*s, -16*s);
        mane2.cubicTo(0*s, -22*s, 6*s, -18*s, 10*s, -12*s);
        mane2.cubicTo(12*s, -8*s, 8*s, -6*s, 5*s, -9*s);
        mane2.cubicTo(2*s, -11*s, -2*s, -10*s, -5*s, -16*s);
        p.drawPath(mane2);
        
        // === 绘制眼睛（大而可爱）===
        // 眼白
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPointF(-20*s, -10*s), 4*s, 4.5*s);
        // 眼珠
        p.setBrush(QColor(50, 40, 40, alpha));
        p.drawEllipse(QPointF(-20*s, -9*s), 3*s, 3.5*s);
        // 眼睛高光
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPointF(-21.5*s, -11*s), 1.2*s, 1.2*s);
        p.drawEllipse(QPointF(-19*s, -8*s), 0.6*s, 0.6*s);
        
        // === 绘制腮红 ===
        p.setBrush(QColor(255, 180, 190, alpha * 0.4));
        p.drawEllipse(QPointF(-26*s, -3*s), 3*s, 2*s);
        
        // === 随机添加装饰 ===
        int decorType = qrand() % 4;
        if (decorType == 0) {
            // 颈部蝴蝶结
            p.setBrush(accentColor);
            QPainterPath bow;
            bow.moveTo(-10*s, 2*s);
            bow.cubicTo(-16*s, -3*s, -16*s, 7*s, -10*s, 2*s);
            bow.cubicTo(-4*s, -3*s, -4*s, 7*s, -10*s, 2*s);
            p.drawPath(bow);
            // 蝴蝶结中心铃铛
            p.setBrush(QColor(255, 220, 100, alpha));
            p.drawEllipse(QPointF(-10*s, 2*s), 2*s, 2*s);
        } else if (decorType == 1) {
            // 身上的星星装饰
            p.setBrush(QColor(255, 230, 150, alpha * 0.8));
            drawStar(p, 0*s, 2*s, 4*s);
            drawStar(p, 8*s, -2*s, 3*s);
        } else if (decorType == 2) {
            // 项链
            p.setPen(QPen(QColor(180, 220, 200, alpha), 1.2*s));
            p.setBrush(Qt::NoBrush);
            QPainterPath necklace;
            necklace.moveTo(-24*s, -2*s);
            necklace.quadTo(-18*s, 6*s, -10*s, 4*s);
            p.drawPath(necklace);
            p.setPen(Qt::NoPen);
            // 吊坠星星
            p.setBrush(QColor(255, 230, 150, alpha));
            drawStar(p, -17*s, 4*s, 3*s);
        }
        
        p.restore();
    }
    
    // 绘制星星
    void drawStar(QPainter &p, double x, double y, double r) {
        QPainterPath star;
        for (int i = 0; i < 5; ++i) {
            double angle1 = (i * 72 - 90) * M_PI / 180.0;
            double angle2 = ((i * 72) + 36 - 90) * M_PI / 180.0;
            QPointF outer(x + r * qCos(angle1), y + r * qSin(angle1));
            QPointF inner(x + r * 0.4 * qCos(angle2), y + r * 0.4 * qSin(angle2));
            if (i == 0) star.moveTo(outer);
            else star.lineTo(outer);
            star.lineTo(inner);
        }
        star.closeSubpath();
        p.drawPath(star);
    }
    
    // 绘制多个独角兽
    void drawUnicorns(QPainter &p, int w, int h, QColor baseColor) {
        Q_UNUSED(baseColor);
        // 绘制散落的独角兽
        for (int i = 0; i < 12; ++i) {
            int x = qrand() % w;
            int y = qrand() % h;
            double depth = (qrand() % 100) / 100.0;
            int size = 25 + static_cast<int>(depth * 35); // 25-60
            int alpha = 60 + static_cast<int>(depth * 80); // 60-140
            
            // 随机翻转
            p.save();
            if (qrand() % 2 == 0) {
                p.translate(x, y);
                p.scale(-1, 1);
                p.translate(-x, -y);
            }
            
            drawUnicorn(p, x, y, size, alpha);
            p.restore();
        }
        
        // 添加一些小星星点缀
        p.setPen(Qt::NoPen);
        for (int i = 0; i < 20; ++i) {
            int x = qrand() % w;
            int y = qrand() % h;
            int alpha = 40 + qrand() % 60;
            p.setBrush(QColor(255, 215, 100, alpha));
            drawStar(p, x, y, 3 + qrand() % 5);
        }
    }
    
    void paintEvent(QPaintEvent *) override {
        QString theme = SettingsDialog::getTheme();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        
        QColor patternColor;
        if (theme == "light") patternColor = QColor(0, 0, 0, 5);
        else if (theme == "dark") patternColor = QColor(255, 255, 255, 5);
        else if (theme == "pink") patternColor = QColor(255, 182, 193, 40); // Light Pink
        else if (theme == "tech") patternColor = QColor(0, 229, 255, 20); // Cyan
        else if (theme == "purple") patternColor = QColor(224, 176, 255, 25); // Mauve
        else if (theme == "green") patternColor = QColor(144, 238, 144, 25); // Light Green
        else if (theme == "zijunpink") patternColor = QColor(212, 184, 196, 100); // 子君粉独角兽 #D4B8C4
        else if (theme == "zijunwhite") patternColor = QColor(230, 199, 192, 70); // 子君白爱心 #E6C7C0
        
        // No pattern for light/dark except subtle noise/grain if needed, but currently just clean.
        // We only draw specific patterns for colored themes.
        
        int w = width();
        int h = height();

        // Use a fixed seed relative to size so it doesn't jitter on resize too much,
        // or just use a fixed seed for consistency. 
        // Ideally, we want the pattern to be static relative to the window, 
        // but simple random generation each paint with fixed seed works if parameters are consistent.
        // However, using time in MainWindow was to "randomize" it once. 
        // Here we'll use a fixed seed to ensure it looks designed.
        qsrand(12345); 

        if (theme == "pink") {
            // Draw Falling Sakura Flowers (Depth Effect)
            for (int i = 0; i < 20; ++i) {
                int x = qrand() % w;
                int y = qrand() % h;
                
                // Depth: 0.0 (far) to 1.0 (near)
                double depth = (qrand() % 100) / 100.0;
                int s = 15 + static_cast<int>(depth * 20); // Size: 15-35
                int alpha = 30 + static_cast<int>(depth * 50); // Alpha: 30-80
                
                QColor c = patternColor;
                c.setAlpha(alpha);
                p.setBrush(c);
                p.setPen(Qt::NoPen);
                
                p.save();
                p.translate(x, y);
                p.rotate(qrand() % 360); // Random rotation
                
                // Draw 5 petals
                for(int k=0; k<5; ++k) {
                    p.rotate(72);
                    QPainterPath path;
                    path.moveTo(0, 0);
                    // Cubic petal
                    path.cubicTo(s/2.0, -s/4.0, s, -s/2.0, s, 0);
                    path.cubicTo(s, s/2.0, s/2.0, s/4.0, 0, 0);
                    p.drawPath(path);
                }
                // Center dot
                p.setBrush(QColor(255, 255, 240, alpha));
                p.drawEllipse(QPoint(0,0), s/5, s/5);
                
                p.restore();
            }
        } else if (theme == "tech") {
            // Digital Matrix / Circuit Nodes
            p.setPen(Qt::NoPen);
            for (int i = 0; i < 30; ++i) {
                int x = qrand() % w;
                int y = qrand() % h;
                double depth = (qrand() % 100) / 100.0;
                int s = 4 + static_cast<int>(depth * 10); // 4-14
                int alpha = 10 + static_cast<int>(depth * 40);
                
                QColor c = patternColor;
                c.setAlpha(alpha);
                p.setBrush(c);
                
                // Hexagon
                QPolygonF poly;
                for (int k = 0; k < 6; ++k) {
                    poly << QPointF(s * qCos(k * M_PI / 3), s * qSin(k * M_PI / 3));
                }
                p.save();
                p.translate(x, y);
                p.drawPolygon(poly);
                
                // Connecting lines for some
                if (i % 3 == 0) {
                    p.setPen(QPen(c, 1));
                    p.drawLine(0, 0, (qrand()%100 - 50), (qrand()%100 - 50));
                    p.setPen(Qt::NoPen);
                }
                p.restore();
            }
        } else if (theme == "purple") {
            // Galaxy / Stars
            for (int i = 0; i < 40; ++i) {
                int x = qrand() % w;
                int y = qrand() % h;
                double depth = (qrand() % 100) / 100.0;
                int s = 5 + static_cast<int>(depth * 15); // 5-20
                int alpha = 20 + static_cast<int>(depth * 60);
                
                QColor c = patternColor;
                c.setAlpha(alpha);
                p.setBrush(c);
                p.setPen(Qt::NoPen);
                
                p.save();
                p.translate(x, y);
                // 4-point star
                QPolygonF star;
                star << QPointF(0, -s) << QPointF(s/4.0, -s/4.0) 
                     << QPointF(s, 0) << QPointF(s/4.0, s/4.0) 
                     << QPointF(0, s) << QPointF(-s/4.0, s/4.0) 
                     << QPointF(-s, 0) << QPointF(-s/4.0, -s/4.0);
                p.drawPolygon(star);
                p.restore();
            }
        } else if (theme == "green") {
            // Falling Leaves
            for (int i = 0; i < 25; ++i) {
                int x = qrand() % w;
                int y = qrand() % h;
                double depth = (qrand() % 100) / 100.0;
                int s = 10 + static_cast<int>(depth * 15); // 10-25
                int alpha = 20 + static_cast<int>(depth * 40);
                
                QColor c = patternColor;
                c.setAlpha(alpha);
                p.setBrush(c);
                p.setPen(Qt::NoPen);
                
                p.save();
                p.translate(x, y);
                p.rotate(qrand() % 360);
                
                // Simple leaf shape
                QPainterPath path;
                path.moveTo(0, 0);
                path.quadTo(s, -s/2.0, s*2, 0);
                path.quadTo(s, s/2.0, 0, 0);
                p.drawPath(path);
                
                p.restore();
            }
        } else if (theme == "zijunpink") {
            // 子君粉 - 可爱独角兽
            drawUnicorns(p, w, h, patternColor);
        } else if (theme == "zijunwhite") {
            // 子君白 - 爱心
            for (int i = 0; i < 25; ++i) {
                int x = qrand() % w;
                int y = qrand() % h;
                double depth = (qrand() % 100) / 100.0;
                int s = 8 + static_cast<int>(depth * 18); // 8-26
                int alpha = 30 + static_cast<int>(depth * 50);
                
                QColor c = patternColor;
                c.setAlpha(alpha);
                p.setBrush(c);
                p.setPen(Qt::NoPen);
                
                p.save();
                p.translate(x, y);
                p.rotate((qrand() % 40) - 20); // 轻微随机旋转
                
                // 绘制爱心
                QPainterPath heartPath;
                heartPath.moveTo(0, s/2.0);
                heartPath.cubicTo(-s, -s/3.0, -s/2.0, -s, 0, -s/2.0);
                heartPath.cubicTo(s/2.0, -s, s, -s/3.0, 0, s/2.0);
                p.drawPath(heartPath);
                
                p.restore();
            }
        }
    }
};
