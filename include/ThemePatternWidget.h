#pragma once

#include <QWidget>
#include <QPainter>
#include <QSvgRenderer>
#include <QDateTime>
#include <QtMath>
#include <QCoreApplication>
#include "SettingsDialog.h"

class ThemePatternWidget : public QWidget {
public:
    ThemePatternWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        
        // 加载 SVG 渲染器
        QString appPath = QCoreApplication::applicationDirPath();
        m_svgStanding = new QSvgRenderer(appPath + "/resources/mia/standing-unicorn.svg", this);
        m_svgBow = new QSvgRenderer(appPath + "/resources/mia/bow-unicorn.svg", this);
        m_svgFlying = new QSvgRenderer(appPath + "/resources/mia/flying-unicorn.svg", this);
        m_svgRainbow = new QSvgRenderer(appPath + "/resources/mia/rainbow-unicorn.svg", this);
    }
    
    ~ThemePatternWidget() {
        delete m_svgStanding;
        delete m_svgBow;
        delete m_svgFlying;
        delete m_svgRainbow;
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        
        QString theme = SettingsDialog::getTheme();
        
        if (theme == "zijunpink") {
            drawUnicorns(p);
        } else if (theme == "zijunwhite") {
            drawHearts(p, QColor(230, 199, 192));
        }
    }
    
private:
    QSvgRenderer *m_svgStanding = nullptr;
    QSvgRenderer *m_svgBow = nullptr;
    QSvgRenderer *m_svgFlying = nullptr;
    QSvgRenderer *m_svgRainbow = nullptr;
    
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
    
    // 检查是否重叠
    bool isOverlapping(int x, int y, int size, const QList<QRect> &occupied) {
        QRect newRect(x - size/2 - 10, y - size/2 - 10, size + 20, size + 20);
        for (const QRect &rect : occupied) {
            if (newRect.intersects(rect)) {
                return true;
            }
        }
        return false;
    }
    
    // 绘制独角兽图案（使用 SVG）
    void drawUnicorns(QPainter &p) {
        int w = width();
        int h = height();
        
        // 计算独角兽数量（根据面积）
        int area = w * h;
        int unicornCount = qBound(2, area / 80000, 8);
        
        // 随机种子
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < unicornCount; ++i) {
            // 随机位置
            int x, y;
            int size = 60 + qrand() % 40; // 60-100 像素
            bool found = false;
            
            // 尝试找到不重叠的位置
            for (int attempt = 0; attempt < unicornCount * 10; ++attempt) {
                x = size/2 + qrand() % (w - size);
                y = size/2 + qrand() % (h - size);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            // 记录占用区域
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            // 随机选择一个 SVG
            int svgIndex = qrand() % 4;
            QSvgRenderer *renderer = nullptr;
            switch (svgIndex) {
                case 0: renderer = m_svgStanding; break;
                case 1: renderer = m_svgBow; break;
                case 2: renderer = m_svgFlying; break;
                case 3: renderer = m_svgRainbow; break;
            }
            
            if (renderer && renderer->isValid()) {
                // 设置透明度
                p.setOpacity(0.15 + (qrand() % 10) / 100.0); // 0.15-0.25
                
                // 随机水平翻转
                p.save();
                p.translate(x, y);
                if (qrand() % 2 == 0) {
                    p.scale(-1, 1);
                }
                
                // 渲染 SVG
                QRectF targetRect(-size/2, -size/2, size, size);
                renderer->render(&p, targetRect);
                
                p.restore();
            }
        }
        
        // 添加一些小星星装饰
        p.setOpacity(0.2);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(254, 219, 100)); // 金黄色
        
        int starCount = qBound(3, area / 50000, 12);
        for (int i = 0; i < starCount; ++i) {
            int sx = 20 + qrand() % (w - 40);
            int sy = 20 + qrand() % (h - 40);
            double sr = 4 + qrand() % 6;
            
            // 检查是否和独角兽重叠
            if (!isOverlapping(sx, sy, static_cast<int>(sr * 2), occupiedRects)) {
                drawStar(p, sx, sy, sr);
            }
        }
        
        p.setOpacity(1.0);
    }
    
    // 绘制爱心图案（子君白主题）
    void drawHearts(QPainter &p, QColor heartColor) {
        int w = width();
        int h = height();
        
        int area = w * h;
        int heartCount = qBound(8, area / 8000, 50); // 提高密度
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        for (int i = 0; i < heartCount; ++i) {
            int x, y;
            int size = 15 + qrand() % 20;
            bool found = false;
            
            for (int attempt = 0; attempt < heartCount * 10; ++attempt) {
                x = size + qrand() % (w - size * 2);
                y = size + qrand() % (h - size * 2);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size/2, y - size/2, size, size));
            
            p.setOpacity(0.15 + (qrand() % 15) / 100.0);
            
            QColor color = heartColor;
            color.setAlpha(180 + qrand() % 50);
            p.setPen(Qt::NoPen);
            p.setBrush(color);
            
            p.save();
            p.translate(x, y);
            double rotation = -15 + qrand() % 30;
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
