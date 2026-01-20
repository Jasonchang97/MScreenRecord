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
    
    
    // 绘制樱花图案（粉色主题）- 圆边樱花朵 + 散落花瓣
    void drawSakura(QPainter &p, QColor sakuraColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        // 设置界面极低密度，分布均匀
        int totalPatterns = (area < 200000) ? qBound(2, area / 50000, 5) : qBound(10, area / 10000, 40);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        // 整朵樱花数量 (30%)
        int fullFlowerCount = totalPatterns * 0.3;
        // 散落花瓣数量 (70%)，营造飘零感
        int petalCount = totalPatterns - fullFlowerCount;
        
        // 绘制整朵圆边樱花
        for (int i = 0; i < fullFlowerCount; ++i) {
            int x, y;
            int size = 18 + qrand() % 22; // 18-39px
            bool found = false;
            
            // 均匀分布逻辑
            if (area < 200000 && fullFlowerCount > 0) {
                int gridCols = qSqrt(fullFlowerCount) + 1;
                int gridRows = (fullFlowerCount + gridCols - 1) / gridCols;
                int cellW = w / gridCols;
                int cellH = h / gridRows;
                int gridX = i % gridCols;
                int gridY = i / gridCols;
                x = gridX * cellW + cellW/4 + qrand() % qMax(1, cellW/2);
                y = gridY * cellH + cellH/4 + qrand() % qMax(1, cellH/2);
                found = true;
            } else {
                for (int attempt = 0; attempt < 50; ++attempt) {
                    x = size + qrand() % qMax(1, w - size * 2);
                    y = size + qrand() % qMax(1, h - size * 2);
                    
                    if (!isOverlapping(x, y, size, occupiedRects)) {
                        found = true;
                        break;
                    }
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            p.setOpacity(0.25 + (qrand() % 20) / 100.0);
            
            p.save();
            p.translate(x, y);
            p.rotate(qrand() % 360);
            
            drawRoundSakuraFlower(p, sakuraColor, size / 25.0);
            
            p.restore();
        }
        
        // 绘制散落的花瓣
        for (int i = 0; i < petalCount; ++i) {
            int x, y;
            int size = 6 + qrand() % 15; // 6-20px 花瓣较小
            
            x = size + qrand() % qMax(1, w - size * 2);
            y = size + qrand() % qMax(1, h - size * 2);
            
            p.setOpacity(0.12 + (qrand() % 20) / 100.0);
            
            p.save();
            p.translate(x, y);
            
            // 花瓣飘零效果
            p.rotate(qrand() % 360);
            p.scale(0.6 + (qrand() % 80) / 100.0, 0.7 + (qrand() % 60) / 100.0);
            
            drawFloatingSakuraPetal(p, sakuraColor, size / 15.0);
            
            p.restore();
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制圆边樱花朵
    void drawRoundSakuraFlower(QPainter &p, QColor sakuraColor, double s) {
        QColor petalColor = sakuraColor;
        petalColor.setAlpha(170 + qrand() % 50);
        
        p.setPen(QPen(sakuraColor.darker(105), 0.3*s));
        p.setBrush(petalColor);
        
        // 绘制5片圆润花瓣
        for (int petal = 0; petal < 5; ++petal) {
            p.save();
            p.rotate(petal * 72);
            
            // 圆润的花瓣形状（无尖角）
            QPainterPath roundPetal;
            roundPetal.moveTo(0, -1*s);
            roundPetal.cubicTo(4*s, -3*s, 6*s, -8*s, 3*s, -11*s);
            roundPetal.cubicTo(1*s, -12*s, -1*s, -12*s, -3*s, -11*s);
            roundPetal.cubicTo(-6*s, -8*s, -4*s, -3*s, 0, -1*s);
            roundPetal.closeSubpath();
            p.drawPath(roundPetal);
            
            p.restore();
        }
        
        // 简单花心
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 220, 120, 200));
        p.drawEllipse(QPointF(0, 0), 2*s, 2*s);
    }
    
    // 绘制飘零花瓣
    void drawFloatingSakuraPetal(QPainter &p, QColor sakuraColor, double s) {
        QColor petalColor = sakuraColor;
        petalColor.setAlpha(100 + qrand() % 80);
        
        QLinearGradient gradient(0, -6*s, 0, 6*s);
        gradient.setColorAt(0, petalColor.lighter(130));
        gradient.setColorAt(1, petalColor);
        
        p.setPen(QPen(sakuraColor.darker(108), 0.2*s));
        p.setBrush(gradient);
        
        // 简单的花瓣形状
        QPainterPath petal;
        petal.moveTo(0, -1*s);
        petal.cubicTo(2.5*s, -2*s, 3*s, -5*s, 1.5*s, -7*s);
        petal.cubicTo(0, -7.5*s, -1.5*s, -7*s, -1.5*s, -7*s);
        petal.cubicTo(-3*s, -5*s, -2.5*s, -2*s, 0, -1*s);
        petal.lineTo(0, 4*s);
        petal.closeSubpath();
        p.drawPath(petal);
        
        // 简单中脉
        p.setPen(QPen(petalColor.darker(115), 0.15*s));
        p.drawLine(QPointF(0, -6*s), QPointF(0, 3*s));
    }
    
    // 绘制四角星图案（深邃紫主题）
    void drawStars(QPainter &p, QColor starColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        // 设置界面极低密度，分布均匀
        int starCount = (area < 200000) ? qBound(1, area / 60000, 3) : qBound(8, area / 12000, 35);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < starCount; ++i) {
            int x, y;
            int size = 10 + qrand() % 25; // 10-34px
            bool found = false;
            
            // 均匀分布逻辑：将窗口分成网格
            if (area < 200000) {
                int gridCols = qSqrt(starCount) + 1;
                int gridRows = (starCount + gridCols - 1) / gridCols;
                int cellW = w / gridCols;
                int cellH = h / gridRows;
                int gridX = i % gridCols;
                int gridY = i / gridCols;
                x = gridX * cellW + cellW/4 + qrand() % (cellW/2);
                y = gridY * cellH + cellH/4 + qrand() % (cellH/2);
                found = true;
            } else {
                for (int attempt = 0; attempt < 50; ++attempt) {
                    x = size + qrand() % qMax(1, w - size * 2);
                    y = size + qrand() % qMax(1, h - size * 2);
                    
                    if (!isOverlapping(x, y, size, occupiedRects)) {
                        found = true;
                        break;
                    }
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            p.setOpacity(0.3 + (qrand() % 25) / 100.0);
            
            QColor color = starColor;
            color.setAlpha(180 + qrand() % 70);
            
            p.save();
            p.translate(x, y);
            p.rotate(qrand() % 90);
            
            // 绘制尖锐闪亮的四角星
            double r = size / 2.0;
            QPainterPath sharpStar;
            sharpStar.moveTo(0, -r);               // 上尖
            sharpStar.lineTo(r*0.15, -r*0.15);    // 右上
            sharpStar.lineTo(r, 0);               // 右尖
            sharpStar.lineTo(r*0.15, r*0.15);     // 右下
            sharpStar.lineTo(0, r);               // 下尖
            sharpStar.lineTo(-r*0.15, r*0.15);    // 左下
            sharpStar.lineTo(-r, 0);              // 左尖
            sharpStar.lineTo(-r*0.15, -r*0.15);   // 左上
            sharpStar.closeSubpath();
            
            // 渐变效果增加闪亮感
            QRadialGradient starGrad(0, 0, r);
            starGrad.setColorAt(0, color.lighter(150));
            starGrad.setColorAt(0.7, color);
            starGrad.setColorAt(1, color.darker(110));
            
            p.setPen(Qt::NoPen);
            p.setBrush(starGrad);
            p.drawPath(sharpStar);
            
            p.restore();
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制飘零叶子图案（绿色主题）
    void drawLeaves(QPainter &p, QColor leafColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        // 设置界面极低密度，分布均匀
        int leafCount = (area < 200000) ? qBound(1, area / 60000, 3) : qBound(8, area / 12000, 30);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < leafCount; ++i) {
            int x, y;
            int size = 15 + qrand() % 30; // 15-44px
            bool found = false;
            
            // 均匀分布逻辑
            if (area < 200000 && leafCount > 0) {
                int gridCols = qSqrt(leafCount) + 1;
                int gridRows = (leafCount + gridCols - 1) / gridCols;
                int cellW = w / gridCols;
                int cellH = h / gridRows;
                int gridX = i % gridCols;
                int gridY = i / gridCols;
                x = gridX * cellW + cellW/4 + qrand() % qMax(1, cellW/2);
                y = gridY * cellH + cellH/4 + qrand() % qMax(1, cellH/2);
                found = true;
            } else {
                for (int attempt = 0; attempt < 50; ++attempt) {
                    x = size + qrand() % qMax(1, w - size * 2);
                    y = size + qrand() % qMax(1, h - size * 2);
                    
                    if (!isOverlapping(x, y, size, occupiedRects)) {
                        found = true;
                        break;
                    }
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            p.setOpacity(0.18 + (qrand() % 25) / 100.0);
            
            QColor color = leafColor;
            color.setAlpha(140 + qrand() % 80);
            
            p.save();
            p.translate(x, y);
            
            // 飘零效果：随机旋转和倾斜
            p.rotate(qrand() % 360);
            p.scale(0.8 + (qrand() % 40) / 100.0, 0.9 + (qrand() % 20) / 100.0);
            
            // 随机选择叶子类型
            int leafType = qrand() % 4;
            
            switch(leafType) {
                case 0: drawNaturalLeaf1(p, color, size / 30.0); break;
                case 1: drawNaturalLeaf2(p, color, size / 28.0); break;
                case 2: drawNaturalLeaf3(p, color, size / 32.0); break;
                case 3: drawNaturalLeaf4(p, color, size / 30.0); break;
            }
            
            p.restore();
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制自然叶子1 - 椭圆形
    void drawNaturalLeaf1(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color.darker(115), 0.4*s));
        p.setBrush(color);
        
        QPainterPath leaf;
        leaf.moveTo(0, -10*s);
        leaf.cubicTo(5*s, -8*s, 7*s, -2*s, 6*s, 4*s);
        leaf.cubicTo(4*s, 8*s, 1*s, 9*s, 0, 10*s);
        leaf.cubicTo(-1*s, 9*s, -4*s, 8*s, -6*s, 4*s);
        leaf.cubicTo(-7*s, -2*s, -5*s, -8*s, 0, -10*s);
        p.drawPath(leaf);
        
        // 自然叶脉
        p.setPen(QPen(color.darker(125), 0.25*s));
        p.drawLine(QPointF(0, -8*s), QPointF(0, 8*s)); // 主脉
        p.drawLine(QPointF(0, -4*s), QPointF(3*s, -2*s));
        p.drawLine(QPointF(0, 0), QPointF(4*s, 2*s));
        p.drawLine(QPointF(0, 4*s), QPointF(3*s, 6*s));
        p.drawLine(QPointF(0, -4*s), QPointF(-3*s, -2*s));
        p.drawLine(QPointF(0, 0), QPointF(-4*s, 2*s));
        p.drawLine(QPointF(0, 4*s), QPointF(-3*s, 6*s));
    }
    
    // 绘制自然叶子2 - 心形
    void drawNaturalLeaf2(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color.darker(115), 0.4*s));
        p.setBrush(color);
        
        QPainterPath leaf;
        leaf.moveTo(0, 10*s);
        leaf.cubicTo(-3*s, 6*s, -6*s, 2*s, -4*s, -4*s);
        leaf.cubicTo(-3*s, -8*s, 0, -9*s, 0, -9*s);
        leaf.cubicTo(0, -9*s, 3*s, -8*s, 4*s, -4*s);
        leaf.cubicTo(6*s, 2*s, 3*s, 6*s, 0, 10*s);
        p.drawPath(leaf);
        
        // 叶脉
        p.setPen(QPen(color.darker(125), 0.25*s));
        p.drawLine(QPointF(0, -7*s), QPointF(0, 8*s));
        p.drawLine(QPointF(0, -3*s), QPointF(-2*s, -1*s));
        p.drawLine(QPointF(0, 1*s), QPointF(-3*s, 3*s));
        p.drawLine(QPointF(0, -3*s), QPointF(2*s, -1*s));
        p.drawLine(QPointF(0, 1*s), QPointF(3*s, 3*s));
    }
    
    // 绘制自然叶子3 - 柳叶形
    void drawNaturalLeaf3(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color.darker(115), 0.4*s));
        p.setBrush(color);
        
        QPainterPath leaf;
        leaf.moveTo(0, -12*s);
        leaf.cubicTo(2*s, -10*s, 3*s, -6*s, 3*s, 0);
        leaf.cubicTo(3*s, 6*s, 2*s, 9*s, 0, 10*s);
        leaf.cubicTo(-2*s, 9*s, -3*s, 6*s, -3*s, 0);
        leaf.cubicTo(-3*s, -6*s, -2*s, -10*s, 0, -12*s);
        p.drawPath(leaf);
        
        // 简单叶脉
        p.setPen(QPen(color.darker(125), 0.25*s));
        p.drawLine(QPointF(0, -10*s), QPointF(0, 8*s));
        p.drawLine(QPointF(0, -6*s), QPointF(1.5*s, -4*s));
        p.drawLine(QPointF(0, -2*s), QPointF(2*s, 0));
        p.drawLine(QPointF(0, 2*s), QPointF(1.5*s, 4*s));
        p.drawLine(QPointF(0, -6*s), QPointF(-1.5*s, -4*s));
        p.drawLine(QPointF(0, -2*s), QPointF(-2*s, 0));
        p.drawLine(QPointF(0, 2*s), QPointF(-1.5*s, 4*s));
    }
    
    // 绘制自然叶子4 - 锯齿叶
    void drawNaturalLeaf4(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color.darker(115), 0.4*s));
        p.setBrush(color);
        
        QPainterPath leaf;
        leaf.moveTo(0, -10*s);
        leaf.lineTo(2*s, -8*s);
        leaf.lineTo(1*s, -6*s);
        leaf.lineTo(4*s, -3*s);
        leaf.lineTo(3*s, 0);
        leaf.lineTo(5*s, 3*s);
        leaf.lineTo(2*s, 6*s);
        leaf.lineTo(1*s, 8*s);
        leaf.lineTo(0, 10*s);
        leaf.lineTo(-1*s, 8*s);
        leaf.lineTo(-2*s, 6*s);
        leaf.lineTo(-5*s, 3*s);
        leaf.lineTo(-3*s, 0);
        leaf.lineTo(-4*s, -3*s);
        leaf.lineTo(-1*s, -6*s);
        leaf.lineTo(-2*s, -8*s);
        leaf.closeSubpath();
        p.drawPath(leaf);
        
        // 叶脉
        p.setPen(QPen(color.darker(125), 0.3*s));
        p.drawLine(QPointF(0, -8*s), QPointF(0, 8*s));
        p.drawLine(QPointF(0, -4*s), QPointF(2*s, -2*s));
        p.drawLine(QPointF(0, 0), QPointF(3*s, 2*s));
        p.drawLine(QPointF(0, -4*s), QPointF(-2*s, -2*s));
        p.drawLine(QPointF(0, 0), QPointF(-3*s, 2*s));
    }
    
    // 绘制科技图案（蓝色科技主题）
    void drawTechDots(QPainter &p, QColor dotColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        // 设置界面极低密度，分布均匀
        int techCount = (area < 200000) ? qBound(1, area / 60000, 3) : qBound(6, area / 15000, 20);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < techCount; ++i) {
            int x, y;
            int size = 25 + qrand() % 25; // 25-49px
            bool found = false;
            
            // 均匀分布逻辑
            if (area < 200000) {
                int gridCols = qSqrt(techCount) + 1;
                int gridRows = (techCount + gridCols - 1) / gridCols;
                int cellW = w / gridCols;
                int cellH = h / gridRows;
                int gridX = i % gridCols;
                int gridY = i / gridCols;
                x = gridX * cellW + cellW/4 + qrand() % (cellW/2);
                y = gridY * cellH + cellH/4 + qrand() % (cellH/2);
                found = true;
            } else {
                for (int attempt = 0; attempt < 50; ++attempt) {
                    x = size + qrand() % qMax(1, w - size * 2);
                    y = size + qrand() % qMax(1, h - size * 2);
                    
                    if (!isOverlapping(x, y, size, occupiedRects)) {
                        found = true;
                        break;
                    }
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            p.setOpacity(0.3 + (qrand() % 20) / 100.0);
            
            p.save();
            p.translate(x, y);
            p.rotate(qrand() % 360);
            
            // 随机选择科技图案
            int techType = qrand() % 4;
            
            switch(techType) {
                case 0: drawTechCircuit(p, dotColor, size / 30.0); break;
                case 1: drawTechGrid(p, dotColor, size / 25.0); break;
                case 2: drawTechHex(p, dotColor, size / 28.0); break;
                case 3: drawTechWave(p, dotColor, size / 30.0); break;
            }
            
            p.restore();
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制电路板图案
    void drawTechCircuit(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color, 1*s));
        p.setBrush(color);
        
        // 中心芯片
        p.drawRoundedRect(QRectF(-4*s, -4*s, 8*s, 8*s), 1*s, 1*s);
        
        // 电路线
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(-4*s, 0), QPointF(-12*s, 0));  // 左
        p.drawLine(QPointF(4*s, 0), QPointF(12*s, 0));    // 右
        p.drawLine(QPointF(0, -4*s), QPointF(0, -12*s));  // 上
        p.drawLine(QPointF(0, 4*s), QPointF(0, 12*s));    // 下
        
        // 连接点
        p.setBrush(color);
        p.drawEllipse(QPointF(-12*s, 0), 2*s, 2*s);
        p.drawEllipse(QPointF(12*s, 0), 2*s, 2*s);
        p.drawEllipse(QPointF(0, -12*s), 2*s, 2*s);
        p.drawEllipse(QPointF(0, 12*s), 2*s, 2*s);
    }
    
    // 绘制科技网格
    void drawTechGrid(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color, 0.8*s));
        p.setBrush(Qt::NoBrush);
        
        // 网格线
        for (int i = -2; i <= 2; ++i) {
            p.drawLine(QPointF(i*4*s, -10*s), QPointF(i*4*s, 10*s));
            p.drawLine(QPointF(-10*s, i*4*s), QPointF(10*s, i*4*s));
        }
        
        // 节点高光
        p.setBrush(color);
        for (int i = -2; i <= 2; ++i) {
            for (int j = -2; j <= 2; ++j) {
                if ((i + j) % 2 == 0) {
                    p.drawEllipse(QPointF(i*4*s, j*4*s), 1.5*s, 1.5*s);
                }
            }
        }
    }
    
    // 绘制科技六边形
    void drawTechHex(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color, 1.2*s));
        p.setBrush(Qt::NoBrush);
        
        // 外层六边形
        QPainterPath hexOuter;
        for(int i = 0; i < 6; ++i) {
            double angle = i * M_PI / 3;
            double x = 12*s * qCos(angle);
            double y = 12*s * qSin(angle);
            if(i == 0) hexOuter.moveTo(x, y);
            else hexOuter.lineTo(x, y);
        }
        hexOuter.closeSubpath();
        p.drawPath(hexOuter);
        
        // 内层六边形
        QPainterPath hexInner;
        for(int i = 0; i < 6; ++i) {
            double angle = i * M_PI / 3;
            double x = 6*s * qCos(angle);
            double y = 6*s * qSin(angle);
            if(i == 0) hexInner.moveTo(x, y);
            else hexInner.lineTo(x, y);
        }
        hexInner.closeSubpath();
        p.drawPath(hexInner);
        
        // 中心点
        p.setBrush(color);
        p.drawEllipse(QPointF(0, 0), 2*s, 2*s);
        
        // 连接线
        for(int i = 0; i < 6; ++i) {
            double angle = i * M_PI / 3;
            double x1 = 6*s * qCos(angle);
            double y1 = 6*s * qSin(angle);
            double x2 = 12*s * qCos(angle);
            double y2 = 12*s * qSin(angle);
            p.setBrush(Qt::NoBrush);
            p.drawLine(QPointF(x1, y1), QPointF(x2, y2));
            p.setBrush(color);
            p.drawEllipse(QPointF(x1, y1), 1.5*s, 1.5*s);
        }
    }
    
    // 绘制科技波纹
    void drawTechWave(QPainter &p, QColor color, double s) {
        p.setPen(QPen(color, 1*s));
        p.setBrush(Qt::NoBrush);
        
        // 同心圆
        for(int i = 1; i <= 4; ++i) {
            p.drawEllipse(QPointF(0, 0), i*3*s, i*3*s);
        }
        
        // 扫描线
        p.setPen(QPen(color.lighter(120), 0.8*s));
        for(int i = 0; i < 8; ++i) {
            double angle = i * M_PI / 4;
            double x = 12*s * qCos(angle);
            double y = 12*s * qSin(angle);
            p.drawLine(QPointF(0, 0), QPointF(x, y));
        }
        
        // 中心发光点
        QRadialGradient centerGrad(0, 0, 3*s);
        centerGrad.setColorAt(0, color);
        centerGrad.setColorAt(1, QColor(color.red(), color.green(), color.blue(), 0));
        p.setBrush(centerGrad);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(0, 0), 3*s, 3*s);
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
