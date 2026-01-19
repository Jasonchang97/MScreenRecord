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
        else if (theme == "zijunpink") patternColor = QColor(199, 187, 168, 80); // 子君粉泡泡 #C7BBA8
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
            // 子君粉 - 泡泡
            for (int i = 0; i < 35; ++i) {
                int x = qrand() % w;
                int y = qrand() % h;
                double depth = (qrand() % 100) / 100.0;
                int s = 8 + static_cast<int>(depth * 30); // 8-38
                int alpha = 25 + static_cast<int>(depth * 45);
                
                QColor c = patternColor;
                c.setAlpha(alpha);
                
                p.save();
                p.translate(x, y);
                
                // 绘制泡泡（圆形 + 高光）
                p.setPen(QPen(c, 1.5));
                p.setBrush(QColor(c.red(), c.green(), c.blue(), alpha / 3));
                p.drawEllipse(QPoint(0, 0), s, s);
                
                // 泡泡高光
                QColor highlight = QColor(255, 255, 255, alpha / 2);
                p.setPen(Qt::NoPen);
                p.setBrush(highlight);
                p.drawEllipse(QPoint(-s/3, -s/3), s/4, s/5);
                
                p.restore();
            }
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
