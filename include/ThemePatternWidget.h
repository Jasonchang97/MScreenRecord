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
            totalPatterns = qBound(25, area / 20000, 80); // 大窗口密度提高
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
        // 设置界面超低密度，分布均匀
        int totalPatterns = (area < 200000) ? qBound(1, area / 100000, 3) : qBound(18, area / 6000, 70);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        // 整朵樱花数量 (30%)
        int fullFlowerCount = totalPatterns * 0.3;
        // 散落花瓣数量 (70%)，营造飘零感
        int petalCount = totalPatterns - fullFlowerCount;
        
        // 绘制整朵圆边樱花
        for (int i = 0; i < fullFlowerCount; ++i) {
            int x, y;
            int size = 20 + qrand() % 25; // 20-44px，稍大一些
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
            
            p.setOpacity(0.35 + (qrand() % 25) / 100.0); // 提高透明度
            
            p.save();
            p.translate(x, y);
            p.rotate(qrand() % 360);
            
            drawRoundSakuraFlower(p, sakuraColor, size / 22.0); // 稍微调整比例
            
            p.restore();
        }
        
        // 绘制散落的花瓣（营造飘零美感）
        for (int i = 0; i < petalCount; ++i) {
            int x, y;
            int size = 8 + qrand() % 18; // 8-25px 花瓣大小变化
            
            x = size + qrand() % qMax(1, w - size * 2);
            y = size + qrand() % qMax(1, h - size * 2);
            
            p.setOpacity(0.18 + (qrand() % 25) / 100.0); // 提高可见度
            
            p.save();
            p.translate(x, y);
            
            // 花瓣飘零效果 - 多样化变形
            double rotAngle = qrand() % 360;
            double scaleX = 0.7 + (qrand() % 60) / 100.0;
            double scaleY = 0.8 + (qrand() % 40) / 100.0;
            
            p.rotate(rotAngle);
            p.scale(scaleX, scaleY);
            
            // 随机选择花瓣类型
            int petalType = qrand() % 3;
            switch(petalType) {
                case 0: drawFloatingSakuraPetal(p, sakuraColor, size / 12.0); break;
                case 1: drawCurvedSakuraPetal(p, sakuraColor, size / 14.0); break;
                case 2: drawTinyPetalFragment(p, sakuraColor, size / 10.0); break;
            }
            
            p.restore();
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制圆边樱花朵
    void drawRoundSakuraFlower(QPainter &p, QColor sakuraColor, double s) {
        // 花瓣渐变色
        QColor petalColor = sakuraColor;
        QColor petalLight = petalColor.lighter(125);
        QColor petalDark = petalColor.darker(110);
        
        // 绘制5片圆润花瓣
        for (int petal = 0; petal < 5; ++petal) {
            p.save();
            p.rotate(petal * 72);
            
            // 花瓣渐变
            QRadialGradient petalGrad(0, -6*s, 8*s);
            petalGrad.setColorAt(0, petalLight);
            petalGrad.setColorAt(0.6, petalColor);
            petalGrad.setColorAt(1, petalDark);
            
            p.setPen(QPen(sakuraColor.darker(108), 0.2*s));
            p.setBrush(petalGrad);
            
            // 更优雅的圆润花瓣
            QPainterPath roundPetal;
            roundPetal.moveTo(0, -1.5*s);
            roundPetal.cubicTo(3*s, -2*s, 5*s, -6*s, 4*s, -9*s);
            roundPetal.cubicTo(2*s, -11*s, 0, -10.5*s, 0, -10.5*s);
            roundPetal.cubicTo(0, -10.5*s, -2*s, -11*s, -4*s, -9*s);
            roundPetal.cubicTo(-5*s, -6*s, -3*s, -2*s, 0, -1.5*s);
            roundPetal.closeSubpath();
            p.drawPath(roundPetal);
            
            // 花瓣轻微脉络
            p.setPen(QPen(petalDark, 0.15*s));
            p.setBrush(Qt::NoBrush);
            QPainterPath vein;
            vein.moveTo(0, -1.5*s);
            vein.quadTo(1*s, -5*s, 2*s, -8*s);
            p.drawPath(vein);
            vein.moveTo(0, -1.5*s);
            vein.quadTo(-1*s, -5*s, -2*s, -8*s);
            p.drawPath(vein);
            
            p.restore();
        }
        
        // 美丽花心 - 渐变效果
        QRadialGradient centerGrad(0, 0, 2.5*s);
        centerGrad.setColorAt(0, QColor(255, 230, 150));
        centerGrad.setColorAt(0.5, QColor(255, 200, 120));
        centerGrad.setColorAt(1, QColor(240, 180, 100));
        
        p.setPen(Qt::NoPen);
        p.setBrush(centerGrad);
        p.drawEllipse(QPointF(0, 0), 2.5*s, 2.5*s);
        
        // 花蕊细节
        p.setBrush(QColor(255, 160, 80, 180));
        for (int dot = 0; dot < 8; ++dot) {
            double angle = dot * 45 * M_PI / 180;
            double dx = 1.2*s * qCos(angle);
            double dy = 1.2*s * qSin(angle);
            p.drawEllipse(QPointF(dx, dy), 0.4*s, 0.4*s);
        }
    }
    
    // 绘制飘零花瓣
    void drawFloatingSakuraPetal(QPainter &p, QColor sakuraColor, double s) {
        QColor petalColor = sakuraColor;
        QColor petalLight = petalColor.lighter(140);
        QColor petalDark = petalColor.darker(105);
        petalColor.setAlpha(120 + qrand() % 70);
        
        // 优雅的花瓣渐变
        QLinearGradient gradient(0, -5*s, 2*s, 3*s);
        gradient.setColorAt(0, petalLight);
        gradient.setColorAt(0.4, petalColor);
        gradient.setColorAt(1, petalDark);
        
        p.setPen(QPen(sakuraColor.darker(105), 0.15*s));
        p.setBrush(gradient);
        
        // 更优雅的花瓣形状
        QPainterPath petal;
        petal.moveTo(0, -1*s);
        petal.cubicTo(2*s, -1.5*s, 3.5*s, -4*s, 2.5*s, -6*s);
        petal.cubicTo(1*s, -7*s, 0, -6.8*s, 0, -6.8*s);
        petal.cubicTo(0, -6.8*s, -1*s, -7*s, -2.5*s, -6*s);
        petal.cubicTo(-3.5*s, -4*s, -2*s, -1.5*s, 0, -1*s);
        petal.cubicTo(-0.5*s, 1*s, 0, 3*s, 0.5*s, 1*s);
        petal.closeSubpath();
        p.drawPath(petal);
        
        // 细腻的花瓣脉络
        p.setPen(QPen(petalDark, 0.1*s));
        p.setBrush(Qt::NoBrush);
        QPainterPath vein1;
        vein1.moveTo(0, -1*s);
        vein1.quadTo(1*s, -3*s, 1.5*s, -5*s);
        p.drawPath(vein1);
        
        QPainterPath vein2;
        vein2.moveTo(0, -1*s);
        vein2.quadTo(-1*s, -3*s, -1.5*s, -5*s);
        p.drawPath(vein2);
        
        QPainterPath centerVein;
        centerVein.moveTo(0, -1*s);
        centerVein.quadTo(0, -3*s, 0, -5.5*s);
        p.drawPath(centerVein);
    }
    
    // 绘制弯曲花瓣
    void drawCurvedSakuraPetal(QPainter &p, QColor sakuraColor, double s) {
        QColor petalColor = sakuraColor;
        QColor petalLight = petalColor.lighter(130);
        
        QLinearGradient gradient(-2*s, -4*s, 2*s, 2*s);
        gradient.setColorAt(0, petalLight);
        gradient.setColorAt(0.7, petalColor);
        gradient.setColorAt(1, petalColor.darker(108));
        
        p.setPen(QPen(sakuraColor.darker(105), 0.12*s));
        p.setBrush(gradient);
        
        // 弯曲的花瓣形状
        QPainterPath curvedPetal;
        curvedPetal.moveTo(0, -0.5*s);
        curvedPetal.cubicTo(1.5*s, -1*s, 2.8*s, -3*s, 2*s, -5*s);
        curvedPetal.cubicTo(1*s, -5.8*s, 0.2*s, -5.5*s, 0, -5.2*s);
        curvedPetal.cubicTo(-0.2*s, -5.5*s, -1*s, -5.8*s, -2*s, -5*s);
        curvedPetal.cubicTo(-2.8*s, -3*s, -1.5*s, -1*s, 0, -0.5*s);
        curvedPetal.cubicTo(-0.3*s, 1.5*s, 0, 2.5*s, 0.3*s, 1.5*s);
        curvedPetal.closeSubpath();
        p.drawPath(curvedPetal);
    }
    
    // 绘制花瓣碎片
    void drawTinyPetalFragment(QPainter &p, QColor sakuraColor, double s) {
        QColor petalColor = sakuraColor;
        petalColor.setAlpha(100 + qrand() % 60);
        
        p.setPen(QPen(sakuraColor.darker(110), 0.1*s));
        p.setBrush(petalColor);
        
        // 小花瓣碎片
        QPainterPath fragment;
        fragment.moveTo(0, -0.8*s);
        fragment.cubicTo(1*s, -1.2*s, 1.5*s, -2.5*s, 1*s, -3.5*s);
        fragment.cubicTo(0.5*s, -3.8*s, -0.5*s, -3.8*s, -1*s, -3.5*s);
        fragment.cubicTo(-1.5*s, -2.5*s, -1*s, -1.2*s, 0, -0.8*s);
        fragment.lineTo(0, 1.5*s);
        fragment.closeSubpath();
        p.drawPath(fragment);
    }
    
    // 绘制四角星图案（深邃紫主题）
    void drawStars(QPainter &p, QColor starColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        // 设置界面超低密度，分布均匀
        int starCount = (area < 200000) ? qBound(1, area / 100000, 2) : qBound(20, area / 8000, 60);
        
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
    
    // 绘制优雅叶子图案（绿色主题）
    void drawLeaves(QPainter &p, QColor leafColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        // 设置界面超低密度，均匀分布
        int leafCount = (area < 200000) ? qBound(1, area / 100000, 2) : qBound(18, area / 8000, 55);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < leafCount; ++i) {
            int x, y;
            int size = 18 + qrand() % 28; // 18-45px
            bool found = false;
            
            // 设置界面均匀分布，避免挤到一坨
            if (area < 200000) {
                int cellW = w / 2;
                int cellH = h / 2;
                int gridX = i % 2;
                int gridY = i / 2;
                x = gridX * cellW + cellW/3 + qrand() % qMax(1, cellW/3);
                y = gridY * cellH + cellH/3 + qrand() % qMax(1, cellH/3);
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
            
            p.setOpacity(0.25 + (qrand() % 25) / 100.0);
            
            QColor color = leafColor;
            color.setAlpha(150 + qrand() % 80);
            
            p.save();
            p.translate(x, y);
            
            // 飘零效果：随机旋转和自然变形
            p.rotate(qrand() % 360);
            p.scale(0.8 + (qrand() % 40) / 100.0, 0.9 + (qrand() % 20) / 100.0);
            
            // 统一使用优雅叶子
            drawElegantLeaf(p, color, size / 30.0);
            
            p.restore();
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制优雅叶子（单一美观图案）
    void drawElegantLeaf(QPainter &p, QColor color, double s) {
        QColor leafLight = color.lighter(120);
        QColor leafDark = color.darker(115);
        
        // 叶子渐变
        QLinearGradient leafGrad(-3*s, -8*s, 3*s, 8*s);
        leafGrad.setColorAt(0, leafLight);
        leafGrad.setColorAt(0.3, color);
        leafGrad.setColorAt(0.7, color);
        leafGrad.setColorAt(1, leafDark);
        
        p.setPen(QPen(color.darker(110), 0.3*s));
        p.setBrush(leafGrad);
        
        // 优雅的叶子形状 - 椭圆形但有自然弯曲
        QPainterPath leaf;
        leaf.moveTo(0, -11*s);
        leaf.cubicTo(4*s, -9*s, 6*s, -4*s, 5.5*s, 2*s);
        leaf.cubicTo(4*s, 6*s, 2*s, 8*s, 0.5*s, 9.5*s);
        leaf.cubicTo(0, 10*s, 0, 10*s, 0, 10*s); // 叶尖
        leaf.cubicTo(0, 10*s, 0, 10*s, -0.5*s, 9.5*s);
        leaf.cubicTo(-2*s, 8*s, -4*s, 6*s, -5.5*s, 2*s);
        leaf.cubicTo(-6*s, -4*s, -4*s, -9*s, 0, -11*s);
        leaf.closeSubpath();
        p.drawPath(leaf);
        
        // 优雅的叶脉系统
        p.setPen(QPen(leafDark, 0.25*s));
        p.setBrush(Qt::NoBrush);
        
        // 主脉
        QPainterPath mainVein;
        mainVein.moveTo(0, -9*s);
        mainVein.quadTo(-0.5*s, -2*s, 0, 8*s);
        p.drawPath(mainVein);
        
        // 侧脉 - 自然弯曲
        for (int i = 0; i < 5; ++i) {
            double y_pos = -6*s + i * 3*s;
            double curve_strength = 2*s + i * 0.5*s;
            
            QPainterPath rightVein;
            rightVein.moveTo(0, y_pos);
            rightVein.quadTo(curve_strength, y_pos + 1*s, curve_strength * 0.8, y_pos + 2*s);
            p.drawPath(rightVein);
            
            QPainterPath leftVein;
            leftVein.moveTo(0, y_pos);
            leftVein.quadTo(-curve_strength, y_pos + 1*s, -curve_strength * 0.8, y_pos + 2*s);
            p.drawPath(leftVein);
        }
        
        // 叶缘细节
        p.setPen(QPen(leafDark.lighter(110), 0.15*s));
        QPainterPath edgeDetail;
        edgeDetail.moveTo(2*s, -5*s);
        edgeDetail.quadTo(4*s, -3*s, 3.5*s, 1*s);
        p.drawPath(edgeDetail);
        
        QPainterPath edgeDetail2;
        edgeDetail2.moveTo(-2*s, -5*s);
        edgeDetail2.quadTo(-4*s, -3*s, -3.5*s, 1*s);
        p.drawPath(edgeDetail2);
    }
    
    // 绘制科技点阵（赛博蓝主题）- 恢复第一版设计
    void drawTechDots(QPainter &p, QColor dotColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        // 设置界面超低密度，主界面恢复原密度
        int dotCount = (area < 200000) ? qBound(3, area / 50000, 8) : qBound(25, area / 3000, 120);
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        QList<QPointF> dots;
        
        for (int i = 0; i < dotCount; ++i) {
            int x, y;
            int size = 4 + qrand() % 12; // 4-15px 恢复原始大小
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
        // 子君白保持原密度
        int heartCount = (area < 200000) ? qBound(5, area / 15000, 12) : qBound(25, area / 4000, 100);
        
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
