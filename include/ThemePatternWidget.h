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
    // 绘制可爱独角兽
    void drawUnicorn(QPainter &p, int x, int y, int size, int alpha) {
        p.save();
        p.translate(x, y);
        
        // 随机选择独角兽的颜色组合
        int colorType = qrand() % 4;
        QColor bodyColor, maneColor, hornColor, accentColor;
        
        switch (colorType) {
            case 0: // 白色身体 + 粉色鬃毛
                bodyColor = QColor(255, 255, 255, alpha);
                maneColor = QColor(255, 182, 193, alpha);
                hornColor = QColor(255, 215, 100, alpha);
                accentColor = QColor(173, 216, 230, alpha); // 浅蓝蝴蝶结
                break;
            case 1: // 淡粉身体 + 紫色鬃毛
                bodyColor = QColor(255, 228, 225, alpha);
                maneColor = QColor(200, 162, 200, alpha);
                hornColor = QColor(255, 215, 100, alpha);
                accentColor = QColor(255, 182, 193, alpha);
                break;
            case 2: // 白色身体 + 彩虹鬃毛
                bodyColor = QColor(255, 255, 255, alpha);
                maneColor = QColor(180, 210, 230, alpha); // 淡蓝
                hornColor = QColor(255, 215, 100, alpha);
                accentColor = QColor(255, 192, 203, alpha);
                break;
            default: // 淡紫身体
                bodyColor = QColor(245, 230, 245, alpha);
                maneColor = QColor(216, 191, 216, alpha);
                hornColor = QColor(255, 223, 120, alpha);
                accentColor = QColor(176, 224, 230, alpha);
                break;
        }
        
        double s = size / 40.0; // 缩放因子
        
        // 绘制身体（椭圆形）
        p.setPen(Qt::NoPen);
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(0, 0), 18*s, 12*s);
        
        // 绘制头部
        p.drawEllipse(QPointF(-16*s, -8*s), 10*s, 9*s);
        
        // 绘制耳朵
        QPainterPath earPath;
        earPath.moveTo(-20*s, -16*s);
        earPath.lineTo(-18*s, -22*s);
        earPath.lineTo(-14*s, -16*s);
        earPath.closeSubpath();
        p.drawPath(earPath);
        
        // 绘制独角 (金色渐变效果)
        QLinearGradient hornGrad(-18*s, -22*s, -16*s, -34*s);
        hornGrad.setColorAt(0, hornColor);
        hornGrad.setColorAt(1, QColor(255, 240, 180, alpha));
        p.setBrush(hornGrad);
        QPainterPath hornPath;
        hornPath.moveTo(-20*s, -20*s);
        hornPath.lineTo(-17*s, -34*s);
        hornPath.lineTo(-14*s, -20*s);
        hornPath.closeSubpath();
        p.drawPath(hornPath);
        
        // 独角螺纹
        p.setPen(QPen(QColor(255, 200, 100, alpha/2), s));
        p.drawLine(QPointF(-19*s, -23*s), QPointF(-15*s, -22*s));
        p.drawLine(QPointF(-18.5*s, -26*s), QPointF(-15.5*s, -25*s));
        p.drawLine(QPointF(-18*s, -29*s), QPointF(-16*s, -28*s));
        p.setPen(Qt::NoPen);
        
        // 绘制鬃毛
        p.setBrush(maneColor);
        QPainterPath manePath;
        manePath.moveTo(-8*s, -14*s);
        manePath.cubicTo(-4*s, -20*s, 4*s, -18*s, 8*s, -14*s);
        manePath.cubicTo(10*s, -10*s, 6*s, -8*s, 2*s, -10*s);
        manePath.cubicTo(-2*s, -12*s, -6*s, -10*s, -8*s, -14*s);
        p.drawPath(manePath);
        
        // 绘制尾巴
        QPainterPath tailPath;
        tailPath.moveTo(18*s, -2*s);
        tailPath.cubicTo(26*s, -8*s, 30*s, 0, 26*s, 8*s);
        tailPath.cubicTo(22*s, 12*s, 20*s, 6*s, 18*s, 2*s);
        p.drawPath(tailPath);
        
        // 绘制四条腿
        p.setBrush(bodyColor);
        p.drawRoundedRect(QRectF(-10*s, 8*s, 5*s, 14*s), 2*s, 2*s);
        p.drawRoundedRect(QRectF(-2*s, 8*s, 5*s, 14*s), 2*s, 2*s);
        p.drawRoundedRect(QRectF(6*s, 8*s, 5*s, 12*s), 2*s, 2*s);
        p.drawRoundedRect(QRectF(12*s, 8*s, 5*s, 12*s), 2*s, 2*s);
        
        // 绘制眼睛
        p.setBrush(QColor(40, 40, 40, alpha));
        p.drawEllipse(QPointF(-20*s, -8*s), 2*s, 2.5*s);
        // 眼睛高光
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPointF(-21*s, -9*s), 0.8*s, 0.8*s);
        
        // 绘制腮红
        p.setBrush(QColor(255, 180, 180, alpha/2));
        p.drawEllipse(QPointF(-22*s, -4*s), 2.5*s, 1.5*s);
        
        // 随机添加装饰
        int decorType = qrand() % 3;
        if (decorType == 0) {
            // 蝴蝶结
            p.setBrush(accentColor);
            QPainterPath bowPath;
            bowPath.moveTo(-16*s, 0);
            bowPath.cubicTo(-22*s, -4*s, -22*s, 4*s, -16*s, 0);
            bowPath.cubicTo(-10*s, -4*s, -10*s, 4*s, -16*s, 0);
            p.drawPath(bowPath);
            // 蝴蝶结中心
            p.setBrush(QColor(255, 215, 100, alpha));
            p.drawEllipse(QPointF(-16*s, 0), 1.5*s, 1.5*s);
        } else if (decorType == 1) {
            // 星星装饰
            QColor starColor(255, 215, 100, alpha);
            p.setBrush(starColor);
            drawStar(p, -5*s, -4*s, 3*s);
            drawStar(p, 8*s, -6*s, 2*s);
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
