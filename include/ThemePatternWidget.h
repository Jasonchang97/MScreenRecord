#pragma once

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QDateTime>
#include <QtMath>
#include <QSvgRenderer>
#include <QCoreApplication>
#include "SettingsDialog.h"

class ThemePatternWidget : public QWidget {
public:
    ThemePatternWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        
        // 加载SVG渲染器
        QString appPath = QCoreApplication::applicationDirPath();
        m_miaRenderer = new QSvgRenderer(appPath + "/resources/mia/mia.svg", this);
        m_candyRenderer = new QSvgRenderer(appPath + "/resources/mia/candy.svg", this);
        m_rainbowRenderer = new QSvgRenderer(appPath + "/resources/mia/rainbow.svg", this);
        m_starRenderer = new QSvgRenderer(appPath + "/resources/mia/star.svg", this);
    }
    
    ~ThemePatternWidget() {
        delete m_miaRenderer;
        delete m_candyRenderer;
        delete m_rainbowRenderer;
        delete m_starRenderer;
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        
        QString theme = SettingsDialog::getTheme();
        
        if (theme == "zijunpink") {
            drawSvgPatterns(p);
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
    // SVG 渲染器
    QSvgRenderer *m_miaRenderer;
    QSvgRenderer *m_candyRenderer;
    QSvgRenderer *m_rainbowRenderer;
    QSvgRenderer *m_starRenderer;
    
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
    
    // 绘制SVG图案（子君粉主题）
    void drawSvgPatterns(QPainter &p) {
        int w = width();
        int h = height();
        int area = w * h;
        
        // 根据窗口大小调整密度
        int totalPatterns;
        if (area < 200000) {
            totalPatterns = qBound(8, area / 25000, 20);  // 小窗口
        } else {
            totalPatterns = qBound(15, area / 30000, 40); // 大窗口
        }
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        // 独角兽(mia.svg)数量多一些，占总数的60%
        int miaCount = totalPatterns * 0.6;
        int decorationCount = totalPatterns - miaCount;
        
        // 绘制独角兽
        for (int i = 0; i < miaCount && m_miaRenderer->isValid(); ++i) {
            int size = 30 + qrand() % 40; // 30-70px大小变化
            int x, y;
            bool found = false;
            
            for (int attempt = 0; attempt < 50; ++attempt) {
                x = size/2 + qrand() % qMax(1, w - size);
                y = size/2 + qrand() % qMax(1, h - size);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            // 设置透明度和旋转
            p.setOpacity(0.7 + (qrand() % 30) / 100.0); // 0.7-1.0透明度
            
            p.save();
            p.translate(x, y);
            
            // 随机水平翻转
            if (qrand() % 2 == 0) {
                p.scale(-1, 1);
            }
            
            // 轻微随机旋转
            int rotation = -15 + qrand() % 30;
            p.rotate(rotation);
            
            // 渲染SVG
            QRect rect(-size/2, -size/2, size, size);
            m_miaRenderer->render(&p, rect);
            
            p.restore();
        }
        
        // 绘制装饰物
        QSvgRenderer* decorations[] = {m_candyRenderer, m_rainbowRenderer, m_starRenderer};
        QString decorationNames[] = {"candy", "rainbow", "star"};
        
        for (int i = 0; i < decorationCount; ++i) {
            int decorIndex = qrand() % 3;
            QSvgRenderer* renderer = decorations[decorIndex];
            
            if (!renderer->isValid()) continue;
            
            int size = 15 + qrand() % 25; // 15-40px装饰物较小
            int x, y;
            bool found = false;
            
            for (int attempt = 0; attempt < 50; ++attempt) {
                x = size/2 + qrand() % qMax(1, w - size);
                y = size/2 + qrand() % qMax(1, h - size);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            // 装饰物透明度稍低
            p.setOpacity(0.6 + (qrand() % 25) / 100.0); // 0.6-0.85透明度
            
            p.save();
            p.translate(x, y);
            
            // 装饰物可以更多旋转
            int rotation = qrand() % 360;
            p.rotate(rotation);
            
            // 渲染SVG
            QRect rect(-size/2, -size/2, size, size);
            renderer->render(&p, rect);
            
            p.restore();
        }
        
        p.setOpacity(1.0);
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
    
    
    // 绘制樱花图案（粉色主题）- 整朵樱花 + 飘零花瓣
    void drawSakura(QPainter &p, QColor sakuraColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        // 小窗口极低密度，大窗口适中密度
        int totalPatterns = (area < 200000) ? qBound(3, area / 30000, 8) : qBound(12, area / 8000, 50);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        // 整朵樱花数量 (40%)
        int fullFlowerCount = totalPatterns * 0.4;
        // 单独花瓣数量 (60%)，营造飘零感
        int petalCount = totalPatterns - fullFlowerCount;
        
        // 绘制整朵樱花
        for (int i = 0; i < fullFlowerCount; ++i) {
            int x, y;
            int size = 20 + qrand() % 25; // 20-44px
            bool found = false;
            
            for (int attempt = 0; attempt < 50; ++attempt) {
                x = size + qrand() % qMax(1, w - size * 2);
                y = size + qrand() % qMax(1, h - size * 2);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            p.setOpacity(0.2 + (qrand() % 20) / 100.0);
            
            p.save();
            p.translate(x, y);
            p.rotate(qrand() % 360);
            
            drawFullSakuraFlower(p, sakuraColor, size / 30.0);
            
            p.restore();
        }
        
        // 绘制飘零的花瓣
        for (int i = 0; i < petalCount; ++i) {
            int x, y;
            int size = 8 + qrand() % 18; // 8-25px 花瓣较小
            bool found = false;
            
            for (int attempt = 0; attempt < 50; ++attempt) {
                x = size + qrand() % qMax(1, w - size * 2);
                y = size + qrand() % qMax(1, h - size * 2);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            p.setOpacity(0.15 + (qrand() % 25) / 100.0);
            
            p.save();
            p.translate(x, y);
            
            // 花瓣飘零效果：更随机的旋转和倾斜
            p.rotate(qrand() % 360);
            p.scale(0.8 + (qrand() % 40) / 100.0, 0.8 + (qrand() % 40) / 100.0); // 大小变化
            
            drawSingleSakuraPetal(p, sakuraColor, size / 20.0);
            
            p.restore();
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制完整樱花
    void drawFullSakuraFlower(QPainter &p, QColor sakuraColor, double s) {
        // 绘制5片花瓣
        for (int petal = 0; petal < 5; ++petal) {
            p.save();
            p.rotate(petal * 72); // 每片花瓣相隔72度
            
            QColor petalColor = sakuraColor;
            petalColor.setAlpha(160 + qrand() % 60);
            
            QLinearGradient gradient(0, -12*s, 0, 0);
            gradient.setColorAt(0, petalColor);
            gradient.setColorAt(1, petalColor.lighter(130));
            
            p.setPen(QPen(sakuraColor.darker(110), 0.4*s));
            p.setBrush(gradient);
            
            // 樱花花瓣形状
            QPainterPath petal_path;
            petal_path.moveTo(0, -2*s);
            petal_path.cubicTo(4*s, -5*s, 5*s, -10*s, 2*s, -13*s);
            petal_path.quadTo(0, -11*s, -2*s, -13*s); // 凹口
            petal_path.cubicTo(-5*s, -10*s, -4*s, -5*s, 0, -2*s);
            p.drawPath(petal_path);
            
            p.restore();
        }
        
        // 花蕊
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 220, 100, 200));
        p.drawEllipse(QPointF(0, 0), 2.5*s, 2.5*s);
        
        p.setBrush(QColor(255, 180, 80, 180));
        for (int dot = 0; dot < 5; ++dot) {
            double angle = dot * 72 * M_PI / 180;
            double dx = 1.5*s * qCos(angle);
            double dy = 1.5*s * qSin(angle);
            p.drawEllipse(QPointF(dx, dy), 0.5*s, 0.5*s);
        }
    }
    
    // 绘制单个飘零花瓣
    void drawSingleSakuraPetal(QPainter &p, QColor sakuraColor, double s) {
        QColor petalColor = sakuraColor;
        petalColor.setAlpha(120 + qrand() % 80);
        
        QLinearGradient gradient(0, -8*s, 0, 8*s);
        gradient.setColorAt(0, petalColor.lighter(120));
        gradient.setColorAt(0.5, petalColor);
        gradient.setColorAt(1, petalColor.darker(110));
        
        p.setPen(QPen(sakuraColor.darker(115), 0.3*s));
        p.setBrush(gradient);
        
        // 单个花瓣形状
        QPainterPath petal_path;
        petal_path.moveTo(0, -2*s);
        petal_path.cubicTo(3*s, -4*s, 4*s, -8*s, 1.5*s, -10*s);
        petal_path.quadTo(0, -8.5*s, -1.5*s, -10*s);
        petal_path.cubicTo(-4*s, -8*s, -3*s, -4*s, 0, -2*s);
        petal_path.lineTo(0, 6*s); // 花瓣基部
        petal_path.closeSubpath();
        p.drawPath(petal_path);
        
        // 花瓣纹理
        p.setPen(QPen(petalColor.darker(120), 0.2*s));
        p.drawLine(QPointF(0, -8*s), QPointF(0, 4*s)); // 中脉
        p.drawLine(QPointF(0, -4*s), QPointF(2*s, -3*s)); // 侧脉
        p.drawLine(QPointF(0, -4*s), QPointF(-2*s, -3*s));
    }
    
    // 绘制四角星图案（深邃紫主题）
    void drawStars(QPainter &p, QColor starColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        // 小窗口极低密度，大窗口适中密度
        int starCount = (area < 200000) ? qBound(2, area / 40000, 6) : qBound(12, area / 8000, 50);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < starCount; ++i) {
            int x, y;
            int size = 8 + qrand() % 28; // 8-35px
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
            
            p.setOpacity(0.2 + (qrand() % 25) / 100.0);
            
            QColor color = starColor;
            color.setAlpha(160 + qrand() % 80);
            p.setPen(Qt::NoPen);
            p.setBrush(color);
            
            p.save();
            p.translate(x, y);
            p.rotate(qrand() % 90); // 随机旋转
            
            // 绘制四角星（菱形）
            double r = size / 2.0;
            QPainterPath fourPointStar;
            fourPointStar.moveTo(0, -r);        // 上
            fourPointStar.quadTo(r*0.3, -r*0.3, r, 0);   // 右
            fourPointStar.quadTo(r*0.3, r*0.3, 0, r);    // 下  
            fourPointStar.quadTo(-r*0.3, r*0.3, -r, 0);  // 左
            fourPointStar.quadTo(-r*0.3, -r*0.3, 0, -r); // 回到上
            fourPointStar.closeSubpath();
            p.drawPath(fourPointStar);
            
            p.restore();
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制飘零叶子图案（绿色主题）
    void drawLeaves(QPainter &p, QColor leafColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        // 小窗口极低密度，大窗口适中密度
        int leafCount = (area < 200000) ? qBound(2, area / 40000, 6) : qBound(10, area / 10000, 40);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < leafCount; ++i) {
            int x, y;
            int size = 12 + qrand() % 35; // 12-46px 大小变化更大
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
            
            p.setOpacity(0.15 + (qrand() % 25) / 100.0);
            
            QColor color = leafColor;
            color.setAlpha(130 + qrand() % 90);
            
            p.save();
            p.translate(x, y);
            
            // 飘零效果：大范围随机旋转和倾斜
            p.rotate(qrand() % 360);
            p.scale(0.7 + (qrand() % 60) / 100.0, 0.8 + (qrand() % 40) / 100.0); // 不规则缩放
            
            // 随机选择叶子类型
            int leafType = qrand() % 3;
            
            switch(leafType) {
                case 0:
                    drawMapleLeaf(p, color, size / 25.0);
                    break;
                case 1:
                    drawOvalLeaf(p, color, size / 30.0);
                    break;
                case 2:
                    drawPointedLeaf(p, color, size / 28.0);
                    break;
            }
            
            p.restore();
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制枫叶形状
    void drawMapleLeaf(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color.darker(120), 0.4*s));
        p.setBrush(color);
        
        QPainterPath maple;
        maple.moveTo(0, -12*s);
        maple.cubicTo(-4*s, -10*s, -8*s, -6*s, -6*s, -2*s);
        maple.cubicTo(-10*s, -4*s, -12*s, 0, -8*s, 4*s);
        maple.cubicTo(-6*s, 6*s, -2*s, 8*s, 0, 10*s);
        maple.cubicTo(2*s, 8*s, 6*s, 6*s, 8*s, 4*s);
        maple.cubicTo(12*s, 0, 10*s, -4*s, 6*s, -2*s);
        maple.cubicTo(8*s, -6*s, 4*s, -10*s, 0, -12*s);
        p.drawPath(maple);
        
        // 叶脉
        p.setPen(QPen(color.darker(140), 0.3*s));
        p.drawLine(QPointF(0, -10*s), QPointF(0, 8*s));
        p.drawLine(QPointF(0, -4*s), QPointF(-6*s, -1*s));
        p.drawLine(QPointF(0, -4*s), QPointF(6*s, -1*s));
        p.drawLine(QPointF(0, 2*s), QPointF(-4*s, 5*s));
        p.drawLine(QPointF(0, 2*s), QPointF(4*s, 5*s));
    }
    
    // 绘制椭圆叶子
    void drawOvalLeaf(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color.darker(120), 0.5*s));
        p.setBrush(color);
        
        QPainterPath oval;
        oval.moveTo(0, -10*s);
        oval.cubicTo(6*s, -8*s, 8*s, 0, 0, 10*s);
        oval.cubicTo(-8*s, 0, -6*s, -8*s, 0, -10*s);
        p.drawPath(oval);
        
        // 简单叶脉
        p.setPen(QPen(color.darker(130), 0.3*s));
        p.drawLine(QPointF(0, -8*s), QPointF(0, 8*s));
        p.drawLine(QPointF(0, -3*s), QPointF(3*s, -1*s));
        p.drawLine(QPointF(0, 0), QPointF(4*s, 2*s));
        p.drawLine(QPointF(0, 3*s), QPointF(3*s, 5*s));
        p.drawLine(QPointF(0, -3*s), QPointF(-3*s, -1*s));
        p.drawLine(QPointF(0, 0), QPointF(-4*s, 2*s));
        p.drawLine(QPointF(0, 3*s), QPointF(-3*s, 5*s));
    }
    
    // 绘制尖叶
    void drawPointedLeaf(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color.darker(120), 0.4*s));
        p.setBrush(color);
        
        QPainterPath pointed;
        pointed.moveTo(0, -12*s);
        pointed.cubicTo(3*s, -9*s, 5*s, -3*s, 4*s, 3*s);
        pointed.cubicTo(3*s, 7*s, 1*s, 9*s, 0, 10*s);
        pointed.cubicTo(-1*s, 9*s, -3*s, 7*s, -4*s, 3*s);
        pointed.cubicTo(-5*s, -3*s, -3*s, -9*s, 0, -12*s);
        p.drawPath(pointed);
        
        // 叶脉
        p.setPen(QPen(color.darker(130), 0.3*s));
        p.drawLine(QPointF(0, -10*s), QPointF(0, 8*s));
        p.drawLine(QPointF(0, -6*s), QPointF(2*s, -4*s));
        p.drawLine(QPointF(0, -2*s), QPointF(3*s, 0));
        p.drawLine(QPointF(0, 2*s), QPointF(2*s, 4*s));
        p.drawLine(QPointF(0, -6*s), QPointF(-2*s, -4*s));
        p.drawLine(QPointF(0, -2*s), QPointF(-3*s, 0));
        p.drawLine(QPointF(0, 2*s), QPointF(-2*s, 4*s));
    }
    
    // 绘制化合物结构式（蓝色科技主题）
    void drawTechDots(QPainter &p, QColor dotColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        // 小窗口极低密度，大窗口适中密度
        int moleculeCount = (area < 200000) ? qBound(1, area / 60000, 4) : qBound(8, area / 15000, 25);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < moleculeCount; ++i) {
            int x, y;
            int size = 40 + qrand() % 30; // 40-70px分子大小
            bool found = false;
            
            for (int attempt = 0; attempt < moleculeCount * 10; ++attempt) {
                x = size/2 + qrand() % qMax(1, w - size);
                y = size/2 + qrand() % qMax(1, h - size);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            p.setOpacity(0.25 + (qrand() % 20) / 100.0);
            
            QColor color = dotColor;
            color.setAlpha(180 + qrand() % 60);
            
            p.save();
            p.translate(x, y);
            p.rotate(qrand() % 360);
            
            double s = size / 40.0;
            
            // 随机选择分子类型
            int moleculeType = qrand() % 4;
            
            switch(moleculeType) {
                case 0: // 苯环
                    drawBenzeneRing(p, color, s);
                    break;
                case 1: // 链状分子
                    drawChainMolecule(p, color, s);
                    break;
                case 2: // 环状分子
                    drawCyclicMolecule(p, color, s);
                    break;
                case 3: // 分支分子
                    drawBranchedMolecule(p, color, s);
                    break;
            }
            
            p.restore();
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制苯环
    void drawBenzeneRing(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color, 1.5*s));
        p.setBrush(color);
        
        // 六边形苯环
        QPointF points[6];
        for(int i = 0; i < 6; ++i) {
            double angle = i * M_PI / 3;
            points[i] = QPointF(12*s * qCos(angle), 12*s * qSin(angle));
        }
        
        // 绘制键
        p.setBrush(Qt::NoBrush);
        for(int i = 0; i < 6; ++i) {
            p.drawLine(points[i], points[(i+1)%6]);
        }
        
        // 绘制原子
        p.setBrush(color);
        for(int i = 0; i < 6; ++i) {
            p.drawEllipse(points[i], 2*s, 2*s);
        }
    }
    
    // 绘制链状分子
    void drawChainMolecule(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color, 1.2*s));
        p.setBrush(color);
        
        QPointF atoms[5];
        atoms[0] = QPointF(-15*s, 0);
        atoms[1] = QPointF(-7*s, -8*s);
        atoms[2] = QPointF(3*s, -5*s);
        atoms[3] = QPointF(12*s, -10*s);
        atoms[4] = QPointF(18*s, -2*s);
        
        // 绘制键
        p.setBrush(Qt::NoBrush);
        for(int i = 0; i < 4; ++i) {
            p.drawLine(atoms[i], atoms[i+1]);
        }
        
        // 绘制原子
        p.setBrush(color);
        for(int i = 0; i < 5; ++i) {
            double atomSize = (i == 2) ? 2.5*s : 1.8*s; // 中心原子大一些
            p.drawEllipse(atoms[i], atomSize, atomSize);
        }
    }
    
    // 绘制环状分子
    void drawCyclicMolecule(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color, 1.3*s));
        p.setBrush(color);
        
        // 五边形环
        QPointF points[5];
        for(int i = 0; i < 5; ++i) {
            double angle = -M_PI/2 + i * 2 * M_PI / 5;
            points[i] = QPointF(10*s * qCos(angle), 10*s * qSin(angle));
        }
        
        // 绘制键
        p.setBrush(Qt::NoBrush);
        for(int i = 0; i < 5; ++i) {
            p.drawLine(points[i], points[(i+1)%5]);
        }
        
        // 绘制原子
        p.setBrush(color);
        for(int i = 0; i < 5; ++i) {
            p.drawEllipse(points[i], 2.2*s, 2.2*s);
        }
        
        // 添加侧链
        QPointF sideChain = QPointF(points[0].x(), points[0].y() - 12*s);
        p.setBrush(Qt::NoBrush);
        p.drawLine(points[0], sideChain);
        p.setBrush(color);
        p.drawEllipse(sideChain, 1.8*s, 1.8*s);
    }
    
    // 绘制分支分子
    void drawBranchedMolecule(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color, 1.2*s));
        p.setBrush(color);
        
        // 主链
        QPointF center(0, 0);
        QPointF left(-12*s, 0);
        QPointF right(12*s, 0);
        QPointF top(0, -10*s);
        QPointF bottom(0, 8*s);
        
        // 绘制键
        p.setBrush(Qt::NoBrush);
        p.drawLine(left, center);
        p.drawLine(center, right);
        p.drawLine(center, top);
        p.drawLine(center, bottom);
        
        // 绘制原子
        p.setBrush(color);
        p.drawEllipse(center, 2.5*s, 2.5*s); // 中心原子
        p.drawEllipse(left, 1.8*s, 1.8*s);
        p.drawEllipse(right, 1.8*s, 1.8*s);
        p.drawEllipse(top, 1.8*s, 1.8*s);
        p.drawEllipse(bottom, 1.8*s, 1.8*s);
    }
    
    // 绘制爱心图案（子君白主题）
    void drawHearts(QPainter &p, QColor heartColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        // 子君白保持原密度
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
