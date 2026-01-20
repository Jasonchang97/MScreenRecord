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
