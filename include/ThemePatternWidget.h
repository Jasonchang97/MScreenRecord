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
    // 绘制简约扁平风格独角兽
    void drawUnicorn(QPainter &p, int x, int y, int size, int alpha) {
        p.save();
        p.translate(x, y);
        
        // 随机选择独角兽的颜色组合（参考新图片）
        int colorType = qrand() % 4;
        QColor bodyColor, maneColor, tailColor, hornColor, accentColor;
        bool hasWings = (qrand() % 4 == 0); // 25%概率有翅膀
        
        switch (colorType) {
            case 0: // 白色身体 + 紫色鬃毛
                bodyColor = QColor(255, 255, 255, alpha);
                maneColor = QColor(190, 170, 210, alpha);
                tailColor = QColor(200, 180, 220, alpha);
                hornColor = QColor(255, 220, 120, alpha);
                accentColor = QColor(100, 180, 230, alpha); // 蓝色气球
                break;
            case 1: // 粉色身体 + 粉色鬃毛（飞马款）
                bodyColor = QColor(255, 210, 220, alpha);
                maneColor = QColor(255, 180, 200, alpha);
                tailColor = QColor(255, 190, 210, alpha);
                hornColor = QColor(255, 100, 120, alpha); // 红色小冠
                accentColor = QColor(255, 180, 200, alpha);
                hasWings = true;
                break;
            case 2: // 黄色身体 + 黄色鬃毛
                bodyColor = QColor(255, 240, 180, alpha);
                maneColor = QColor(255, 230, 150, alpha);
                tailColor = QColor(255, 235, 160, alpha);
                hornColor = QColor(255, 220, 100, alpha);
                accentColor = QColor(100, 180, 230, alpha);
                break;
            default: // 白色身体 + 淡蓝鬃毛
                bodyColor = QColor(255, 255, 255, alpha);
                maneColor = QColor(180, 210, 230, alpha);
                tailColor = QColor(190, 200, 220, alpha);
                hornColor = QColor(255, 220, 120, alpha);
                accentColor = QColor(255, 150, 180, alpha);
                break;
        }
        
        double s = size / 45.0; // 缩放因子
        
        p.setPen(Qt::NoPen);
        
        // === 绘制尾巴（简单的波浪形）===
        p.setBrush(tailColor);
        QPainterPath tailPath;
        tailPath.moveTo(18*s, 2*s);
        tailPath.cubicTo(24*s, -2*s, 28*s, 4*s, 26*s, 12*s);
        tailPath.cubicTo(24*s, 18*s, 20*s, 16*s, 22*s, 10*s);
        tailPath.cubicTo(23*s, 6*s, 21*s, 4*s, 18*s, 6*s);
        p.drawPath(tailPath);
        
        // === 绘制后腿（简单的小短腿）===
        p.setBrush(bodyColor);
        p.drawRoundedRect(QRectF(8*s, 10*s, 7*s, 14*s), 3*s, 3*s);
        p.drawRoundedRect(QRectF(14*s, 10*s, 6*s, 12*s), 3*s, 3*s);
        
        // === 绘制身体（简单的椭圆）===
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(5*s, 2*s), 18*s, 12*s);
        
        // === 绘制翅膀（如果有）===
        if (hasWings) {
            p.setBrush(QColor(255, 255, 255, alpha * 0.9));
            QPainterPath wingPath;
            wingPath.moveTo(0*s, -2*s);
            wingPath.cubicTo(-5*s, -15*s, 8*s, -18*s, 12*s, -8*s);
            wingPath.cubicTo(10*s, -4*s, 5*s, -2*s, 0*s, -2*s);
            p.drawPath(wingPath);
            // 翅膀纹理
            p.setPen(QPen(QColor(230, 230, 240, alpha * 0.5), 0.8*s));
            p.drawLine(QPointF(2*s, -5*s), QPointF(6*s, -10*s));
            p.drawLine(QPointF(4*s, -4*s), QPointF(9*s, -9*s));
            p.setPen(Qt::NoPen);
        }
        
        // === 绘制前腿（简单的小短腿）===
        p.setBrush(bodyColor);
        p.drawRoundedRect(QRectF(-12*s, 8*s, 6*s, 14*s), 3*s, 3*s);
        p.drawRoundedRect(QRectF(-6*s, 9*s, 6*s, 13*s), 3*s, 3*s);
        
        // === 绘制头部（圆形，与身体相连）===
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(-14*s, -4*s), 11*s, 10*s);
        
        // === 绘制耳朵（简单三角）===
        QPainterPath earPath;
        earPath.moveTo(-10*s, -12*s);
        earPath.lineTo(-7*s, -20*s);
        earPath.lineTo(-4*s, -12*s);
        earPath.closeSubpath();
        p.drawPath(earPath);
        // 耳内
        p.setBrush(QColor(255, 200, 210, alpha * 0.6));
        QPainterPath earIn;
        earIn.moveTo(-9*s, -13*s);
        earIn.lineTo(-7*s, -18*s);
        earIn.lineTo(-5*s, -13*s);
        earIn.closeSubpath();
        p.drawPath(earIn);
        
        // === 绘制独角（简单金色三角）===
        p.setBrush(hornColor);
        QPainterPath hornPath;
        hornPath.moveTo(-9*s, -14*s);
        hornPath.lineTo(-7*s, -28*s);
        hornPath.lineTo(-5*s, -14*s);
        hornPath.closeSubpath();
        p.drawPath(hornPath);
        
        // === 绘制鬃毛（简单波浪）===
        p.setBrush(maneColor);
        QPainterPath manePath;
        manePath.moveTo(-4*s, -12*s);
        manePath.cubicTo(2*s, -16*s, 6*s, -12*s, 4*s, -6*s);
        manePath.cubicTo(2*s, -2*s, -2*s, -4*s, -4*s, -8*s);
        manePath.cubicTo(-5*s, -10*s, -3*s, -11*s, -4*s, -12*s);
        p.drawPath(manePath);
        
        // === 绘制眼睛（简单黑点）===
        p.setBrush(QColor(40, 40, 50, alpha));
        p.drawEllipse(QPointF(-17*s, -5*s), 2*s, 2.5*s);
        // 高光
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPointF(-18*s, -6*s), 0.8*s, 0.8*s);
        
        // === 绘制腮红 ===
        p.setBrush(QColor(255, 180, 190, alpha * 0.35));
        p.drawEllipse(QPointF(-20*s, -1*s), 2.5*s, 1.5*s);
        
        // === 随机添加装饰（气球/蝴蝶结）===
        int decorType = qrand() % 3;
        if (decorType == 0 && !hasWings) {
            // 气球
            p.setBrush(accentColor);
            p.drawEllipse(QPointF(-20*s, -22*s), 5*s, 6*s);
            // 气球线
            p.setPen(QPen(QColor(150, 150, 150, alpha * 0.6), 0.5*s));
            p.drawLine(QPointF(-20*s, -16*s), QPointF(-18*s, -8*s));
            p.setPen(Qt::NoPen);
            // 气球高光
            p.setBrush(QColor(255, 255, 255, alpha * 0.4));
            p.drawEllipse(QPointF(-22*s, -24*s), 1.5*s, 2*s);
        } else if (decorType == 1) {
            // 蝴蝶结
            p.setBrush(accentColor);
            QPainterPath bow;
            bow.moveTo(-8*s, 1*s);
            bow.cubicTo(-14*s, -3*s, -14*s, 5*s, -8*s, 1*s);
            bow.cubicTo(-2*s, -3*s, -2*s, 5*s, -8*s, 1*s);
            p.drawPath(bow);
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
    
    // 绘制彩虹
    void drawRainbow(QPainter &p, int x, int y, int size, int alpha) {
        p.save();
        p.translate(x, y);
        double s = size / 30.0;
        
        // 彩虹颜色（从外到内）
        QColor colors[] = {
            QColor(255, 100, 100, alpha),  // 红
            QColor(255, 180, 100, alpha),  // 橙
            QColor(255, 230, 120, alpha),  // 黄
            QColor(150, 220, 150, alpha),  // 绿
            QColor(130, 180, 230, alpha),  // 蓝
            QColor(180, 150, 210, alpha)   // 紫
        };
        
        p.setPen(Qt::NoPen);
        for (int i = 0; i < 6; ++i) {
            p.setBrush(colors[i]);
            int r = static_cast<int>((30 - i * 3) * s);
            p.drawChord(QRect(-r, -r/2, r*2, r), 0, 180 * 16);
        }
        
        // 云朵
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPointF(-28*s, 2*s), 8*s, 6*s);
        p.drawEllipse(QPointF(-22*s, -2*s), 6*s, 5*s);
        p.drawEllipse(QPointF(28*s, 2*s), 8*s, 6*s);
        p.drawEllipse(QPointF(22*s, -2*s), 6*s, 5*s);
        
        p.restore();
    }
    
    // 绘制太阳
    void drawSun(QPainter &p, int x, int y, int size, int alpha) {
        p.save();
        p.translate(x, y);
        double s = size / 20.0;
        
        QColor sunColor(255, 220, 80, alpha);
        p.setBrush(sunColor);
        p.setPen(Qt::NoPen);
        
        // 太阳光芒
        for (int i = 0; i < 8; ++i) {
            p.save();
            p.rotate(i * 45);
            QPainterPath ray;
            ray.moveTo(-2*s, -8*s);
            ray.lineTo(0, -16*s);
            ray.lineTo(2*s, -8*s);
            ray.closeSubpath();
            p.drawPath(ray);
            p.restore();
        }
        
        // 太阳本体
        p.drawEllipse(QPointF(0, 0), 8*s, 8*s);
        
        p.restore();
    }
    
    // 绘制多个独角兽和装饰
    void drawUnicorns(QPainter &p, int w, int h, QColor baseColor) {
        Q_UNUSED(baseColor);
        
        // 绘制几个彩虹
        for (int i = 0; i < 3; ++i) {
            int x = 50 + qrand() % (w - 100);
            int y = 80 + qrand() % (h - 160);
            int size = 25 + qrand() % 20;
            int alpha = 70 + qrand() % 50;
            drawRainbow(p, x, y, size, alpha);
        }
        
        // 绘制几个太阳
        for (int i = 0; i < 4; ++i) {
            int x = qrand() % w;
            int y = qrand() % h;
            int size = 15 + qrand() % 15;
            int alpha = 80 + qrand() % 60;
            drawSun(p, x, y, size, alpha);
        }
        
        // 绘制散落的独角兽
        for (int i = 0; i < 10; ++i) {
            int x = 30 + qrand() % (w - 60);
            int y = 30 + qrand() % (h - 60);
            double depth = (qrand() % 100) / 100.0;
            int size = 30 + static_cast<int>(depth * 25); // 30-55
            int alpha = 120 + static_cast<int>(depth * 80); // 120-200
            
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
        for (int i = 0; i < 15; ++i) {
            int x = qrand() % w;
            int y = qrand() % h;
            int alpha = 60 + qrand() % 80;
            p.setBrush(QColor(255, 220, 100, alpha));
            drawStar(p, x, y, 4 + qrand() % 6);
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
