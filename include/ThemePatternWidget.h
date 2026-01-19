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
    // 绘制独角兽（精确模拟参考图片 - 右上角站立款）
    void drawUnicorn(QPainter &p, int x, int y, int size, int alpha) {
        p.save();
        p.translate(x, y);
        
        // 随机选择颜色
        int colorType = qrand() % 4;
        QColor bodyColor, maneColor, hornColor;
        
        switch (colorType) {
            case 0: // 白色身体+紫色鬃毛（参考图右上）
                bodyColor = QColor(255, 240, 245, alpha);
                maneColor = QColor(200, 180, 210, alpha);
                hornColor = QColor(255, 230, 180, alpha);
                break;
            case 1: // 白色身体+粉色鬃毛（参考图左上/左下）
                bodyColor = QColor(255, 255, 255, alpha);
                maneColor = QColor(255, 210, 220, alpha);
                hornColor = QColor(255, 230, 180, alpha);
                break;
            case 2: // 白色身体+青蓝鬃毛（参考图右下）
                bodyColor = QColor(255, 255, 255, alpha);
                maneColor = QColor(180, 210, 210, alpha);
                hornColor = QColor(255, 230, 180, alpha);
                break;
            default: // 淡粉身体+紫色鬃毛
                bodyColor = QColor(255, 235, 240, alpha);
                maneColor = QColor(190, 175, 200, alpha);
                hornColor = QColor(255, 230, 180, alpha);
                break;
        }
        
        double s = size / 60.0;
        p.setPen(Qt::NoPen);
        
        // ====== 尾巴（波浪卷曲，在最后面）======
        p.setBrush(maneColor);
        QPainterPath tail;
        tail.moveTo(20*s, 5*s);
        tail.cubicTo(25*s, 0*s, 30*s, 5*s, 28*s, 15*s);
        tail.cubicTo(26*s, 22*s, 22*s, 20*s, 24*s, 12*s);
        tail.cubicTo(25*s, 8*s, 23*s, 6*s, 20*s, 8*s);
        tail.cubicTo(18*s, 10*s, 16*s, 8*s, 18*s, 5*s);
        tail.closeSubpath();
        p.drawPath(tail);
        
        // ====== 后腿 ======
        p.setBrush(bodyColor);
        QPainterPath backLegs;
        // 后腿1
        backLegs.moveTo(12*s, 8*s);
        backLegs.lineTo(14*s, 24*s);
        backLegs.cubicTo(14*s, 27*s, 10*s, 27*s, 10*s, 24*s);
        backLegs.lineTo(8*s, 10*s);
        backLegs.closeSubpath();
        // 后腿2（稍微靠前）
        backLegs.moveTo(6*s, 10*s);
        backLegs.lineTo(7*s, 22*s);
        backLegs.cubicTo(7*s, 25*s, 3*s, 25*s, 3*s, 22*s);
        backLegs.lineTo(2*s, 10*s);
        backLegs.closeSubpath();
        p.drawPath(backLegs);
        
        // ====== 身体（椭圆形）======
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(2*s, 0*s), 20*s, 12*s);
        
        // ====== 前腿 ======
        QPainterPath frontLegs;
        // 前腿1
        frontLegs.moveTo(-10*s, 8*s);
        frontLegs.lineTo(-9*s, 24*s);
        frontLegs.cubicTo(-9*s, 27*s, -13*s, 27*s, -13*s, 24*s);
        frontLegs.lineTo(-14*s, 8*s);
        frontLegs.closeSubpath();
        // 前腿2
        frontLegs.moveTo(-16*s, 6*s);
        frontLegs.lineTo(-16*s, 22*s);
        frontLegs.cubicTo(-16*s, 25*s, -20*s, 25*s, -20*s, 22*s);
        frontLegs.lineTo(-20*s, 6*s);
        frontLegs.closeSubpath();
        p.drawPath(frontLegs);
        
        // ====== 脖子和头部 ======
        // 脖子（连接身体和头）
        QPainterPath neck;
        neck.moveTo(-16*s, -4*s);
        neck.cubicTo(-20*s, -8*s, -22*s, -14*s, -20*s, -18*s);
        neck.cubicTo(-18*s, -14*s, -14*s, -10*s, -14*s, -4*s);
        neck.closeSubpath();
        p.drawPath(neck);
        
        // 头部（大圆）
        p.drawEllipse(QPointF(-24*s, -22*s), 10*s, 9*s);
        
        // 鼻子/嘴巴部分
        p.drawEllipse(QPointF(-32*s, -20*s), 5*s, 4*s);
        
        // ====== 耳朵 ======
        QPainterPath ear;
        ear.moveTo(-20*s, -28*s);
        ear.lineTo(-17*s, -36*s);
        ear.lineTo(-14*s, -28*s);
        ear.closeSubpath();
        p.drawPath(ear);
        // 耳内粉色
        p.setBrush(QColor(255, 210, 220, alpha * 0.7));
        QPainterPath earInner;
        earInner.moveTo(-19*s, -29*s);
        earInner.lineTo(-17*s, -34*s);
        earInner.lineTo(-15*s, -29*s);
        earInner.closeSubpath();
        p.drawPath(earInner);
        
        // ====== 独角 ======
        p.setBrush(hornColor);
        QPainterPath horn;
        horn.moveTo(-22*s, -30*s);
        horn.lineTo(-20*s, -46*s);
        horn.lineTo(-18*s, -30*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // ====== 鬃毛（波浪状从头顶延伸到背部）======
        p.setBrush(maneColor);
        QPainterPath mane;
        // 头顶鬃毛
        mane.moveTo(-14*s, -26*s);
        mane.cubicTo(-10*s, -30*s, -6*s, -26*s, -8*s, -20*s);
        mane.cubicTo(-10*s, -16*s, -6*s, -14*s, -2*s, -18*s);
        mane.cubicTo(2*s, -22*s, 6*s, -16*s, 4*s, -10*s);
        mane.cubicTo(2*s, -6*s, 6*s, -4*s, 10*s, -8*s);
        mane.cubicTo(14*s, -12*s, 16*s, -6*s, 14*s, 0*s);
        // 返回
        mane.cubicTo(12*s, -4*s, 8*s, -2*s, 4*s, -6*s);
        mane.cubicTo(0*s, -10*s, -4*s, -8*s, -6*s, -12*s);
        mane.cubicTo(-8*s, -16*s, -12*s, -14*s, -14*s, -18*s);
        mane.cubicTo(-16*s, -22*s, -14*s, -24*s, -14*s, -26*s);
        mane.closeSubpath();
        p.drawPath(mane);
        
        // ====== 眼睛 ======
        p.setBrush(QColor(50, 45, 50, alpha));
        p.drawEllipse(QPointF(-26*s, -22*s), 2*s, 2.5*s);
        // 眼睛高光
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPointF(-27*s, -23*s), 0.8*s, 0.8*s);
        
        // ====== 腮红 ======
        p.setBrush(QColor(255, 200, 210, alpha * 0.4));
        p.drawEllipse(QPointF(-30*s, -18*s), 2.5*s, 1.5*s);
        
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
    
    // 绘制多个独角兽剪影
    void drawUnicorns(QPainter &p, int w, int h, QColor baseColor) {
        Q_UNUSED(baseColor);
        
        // 绘制散落的独角兽
        for (int i = 0; i < 15; ++i) {
            int x = 40 + qrand() % (w - 80);
            int y = 40 + qrand() % (h - 80);
            double depth = (qrand() % 100) / 100.0;
            int size = 35 + static_cast<int>(depth * 30); // 35-65
            int alpha = 50 + static_cast<int>(depth * 60); // 50-110
            
            // 随机翻转（朝左或朝右）
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
        for (int i = 0; i < 12; ++i) {
            int x = qrand() % w;
            int y = qrand() % h;
            int alpha = 40 + qrand() % 50;
            p.setBrush(QColor(220, 200, 180, alpha));
            drawStar(p, x, y, 3 + qrand() % 4);
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
