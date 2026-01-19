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
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        
        QString theme = SettingsDialog::getTheme();
        
        if (theme == "zijunpink") {
            drawUnicorns(p);
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
    
    // 绘制四角星装饰
    void drawFourPointStar(QPainter &p, double cx, double cy, double r) {
        QPainterPath star;
        star.moveTo(cx, cy - r);
        star.quadTo(cx + r*0.2, cy - r*0.2, cx + r, cy);
        star.quadTo(cx + r*0.2, cy + r*0.2, cx, cy + r);
        star.quadTo(cx - r*0.2, cy + r*0.2, cx - r, cy);
        star.quadTo(cx - r*0.2, cy - r*0.2, cx, cy - r);
        p.drawPath(star);
    }
    
    // 绘制蝴蝶结
    void drawBow(QPainter &p, double cx, double cy, double size, QColor color, QColor outline) {
        double s = size;
        p.setPen(QPen(outline, 0.4*s));
        p.setBrush(color);
        
        // 左边蝴蝶结
        QPainterPath leftBow;
        leftBow.moveTo(cx, cy);
        leftBow.cubicTo(cx - 3*s, cy - 2*s, cx - 5*s, cy - 1*s, cx - 4*s, cy + 1*s);
        leftBow.cubicTo(cx - 3*s, cy + 2*s, cx - 1*s, cy + 1*s, cx, cy);
        p.drawPath(leftBow);
        
        // 右边蝴蝶结
        QPainterPath rightBow;
        rightBow.moveTo(cx, cy);
        rightBow.cubicTo(cx + 3*s, cy - 2*s, cx + 5*s, cy - 1*s, cx + 4*s, cy + 1*s);
        rightBow.cubicTo(cx + 3*s, cy + 2*s, cx + 1*s, cy + 1*s, cx, cy);
        p.drawPath(rightBow);
        
        // 中心结
        p.drawEllipse(QPointF(cx, cy), 1*s, 0.8*s);
    }
    
    // 绘制铃铛
    void drawBell(QPainter &p, double cx, double cy, double size, QColor color) {
        double s = size;
        p.setPen(QPen(color.darker(120), 0.3*s));
        p.setBrush(color);
        
        // 铃铛身体
        QPainterPath bell;
        bell.moveTo(cx - 1.5*s, cy - 2*s);
        bell.quadTo(cx - 2*s, cy, cx - 2*s, cy + 1*s);
        bell.quadTo(cx - 2*s, cy + 2.5*s, cx, cy + 2.5*s);
        bell.quadTo(cx + 2*s, cy + 2.5*s, cx + 2*s, cy + 1*s);
        bell.quadTo(cx + 2*s, cy, cx + 1.5*s, cy - 2*s);
        bell.closeSubpath();
        p.drawPath(bell);
        
        // 铃铛顶部
        p.drawEllipse(QPointF(cx, cy - 2.5*s), 1*s, 0.6*s);
        
        // 铃铛底部小球
        p.setBrush(color.darker(110));
        p.drawEllipse(QPointF(cx, cy + 2*s), 0.6*s, 0.6*s);
    }
    
    // 类型1：摇摇马独角兽
    void drawRockingUnicorn(QPainter &p, double s, QColor bodyColor, QColor maneColor, QColor hornColor, QColor outlineColor, QColor accentColor) {
        // 摇摇马底座
        p.setPen(QPen(accentColor.darker(120), 0.6*s));
        p.setBrush(accentColor);
        
        QPainterPath rocker;
        rocker.moveTo(-18*s, 18*s);
        rocker.cubicTo(-20*s, 20*s, -15*s, 22*s, 0, 20*s);
        rocker.cubicTo(15*s, 22*s, 20*s, 20*s, 18*s, 18*s);
        rocker.cubicTo(15*s, 16*s, -15*s, 16*s, -18*s, 18*s);
        p.drawPath(rocker);
        
        // 摇摇马支撑柱
        p.drawRoundedRect(QRectF(-8*s, 10*s, 3*s, 8*s), 1*s, 1*s);
        p.drawRoundedRect(QRectF(5*s, 10*s, 3*s, 8*s), 1*s, 1*s);
        
        // 摇摇马上的星星装饰
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 230, 150));
        drawFourPointStar(p, -14*s, 19*s, 2*s);
        drawFourPointStar(p, 14*s, 19*s, 2*s);
        
        p.setPen(QPen(outlineColor, 0.6*s));
        
        // 尾巴 - 蓬松卷曲
        p.setBrush(maneColor);
        QPainterPath tail;
        tail.moveTo(10*s, 0);
        tail.cubicTo(14*s, -4*s, 18*s, -2*s, 16*s, 3*s);
        tail.cubicTo(14*s, 6*s, 16*s, 8*s, 14*s, 10*s);
        tail.cubicTo(12*s, 8*s, 11*s, 4*s, 10*s, 2*s);
        tail.closeSubpath();
        p.drawPath(tail);
        
        // 后腿（粗短）
        p.setBrush(bodyColor);
        p.drawRoundedRect(QRectF(4*s, 4*s, 6*s, 8*s), 3*s, 3*s);
        
        // 前腿（粗短）
        p.drawRoundedRect(QRectF(-10*s, 4*s, 6*s, 8*s), 3*s, 3*s);
        
        // 蹄子
        p.setBrush(QColor(200, 190, 210));
        p.drawEllipse(QPointF(7*s, 12*s), 3*s, 2*s);
        p.drawEllipse(QPointF(-7*s, 12*s), 3*s, 2*s);
        
        // 身体 - 圆润饱满
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(0, 2*s), 14*s, 10*s);
        
        // 脖子（粗）
        QPainterPath neck;
        neck.moveTo(-10*s, -2*s);
        neck.cubicTo(-14*s, -6*s, -14*s, -14*s, -10*s, -16*s);
        neck.lineTo(-4*s, -12*s);
        neck.cubicTo(-6*s, -8*s, -6*s, -4*s, -6*s, 0);
        neck.closeSubpath();
        p.drawPath(neck);
        
        // 头部 - 大圆脸
        p.drawEllipse(QPointF(-10*s, -20*s), 8*s, 7*s);
        
        // 嘴巴/鼻子
        p.drawEllipse(QPointF(-16*s, -18*s), 3*s, 2.5*s);
        
        // 耳朵
        QPainterPath ear;
        ear.moveTo(-14*s, -26*s);
        ear.quadTo(-16*s, -32*s, -12*s, -28*s);
        ear.closeSubpath();
        p.drawPath(ear);
        
        QPainterPath ear2;
        ear2.moveTo(-6*s, -26*s);
        ear2.quadTo(-4*s, -32*s, -8*s, -28*s);
        ear2.closeSubpath();
        p.drawPath(ear2);
        
        // 角
        p.setBrush(hornColor);
        QPainterPath horn;
        horn.moveTo(-10*s, -27*s);
        horn.lineTo(-8*s, -38*s);
        horn.lineTo(-6*s, -27*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // 角纹路
        p.setPen(QPen(hornColor.darker(115), 0.4*s));
        p.drawLine(QPointF(-9.5*s, -30*s), QPointF(-6.5*s, -30*s));
        p.drawLine(QPointF(-9*s, -33*s), QPointF(-7*s, -33*s));
        
        // 鬃毛 - 蓬松波浪
        p.setPen(QPen(outlineColor, 0.5*s));
        p.setBrush(maneColor);
        QPainterPath mane;
        mane.moveTo(-4*s, -26*s);
        mane.cubicTo(0, -24*s, 2*s, -18*s, 0, -12*s);
        mane.cubicTo(-2*s, -6*s, 0, -2*s, 2*s, 2*s);
        mane.lineTo(-2*s, 0);
        mane.cubicTo(-4*s, -4*s, -6*s, -10*s, -4*s, -16*s);
        mane.cubicTo(-2*s, -20*s, -4*s, -24*s, -4*s, -26*s);
        mane.closeSubpath();
        p.drawPath(mane);
        
        // 眼睛
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(50, 50, 70));
        p.drawEllipse(QPointF(-12*s, -20*s), 2*s, 2.5*s);
        
        // 眼睛高光
        p.setBrush(Qt::white);
        p.drawEllipse(QPointF(-13*s, -21*s), 0.8*s, 0.8*s);
        
        // 腮红
        p.setBrush(QColor(255, 180, 180, 120));
        p.drawEllipse(QPointF(-16*s, -16*s), 2.5*s, 1.5*s);
        
        // 蝴蝶结和铃铛
        drawBow(p, -10*s, -10*s, 1.2*s, accentColor, outlineColor);
        drawBell(p, -10*s, -6*s, 0.8*s, QColor(255, 220, 100));
    }
    
    // 类型2：站立独角兽（戴项链，身上有星星）
    void drawStandingUnicorn(QPainter &p, double s, QColor bodyColor, QColor maneColor, QColor hornColor, QColor outlineColor) {
        p.setPen(QPen(outlineColor, 0.6*s));
        
        // 尾巴
        p.setBrush(maneColor);
        QPainterPath tail;
        tail.moveTo(12*s, 2*s);
        tail.cubicTo(16*s, 0, 20*s, 4*s, 18*s, 10*s);
        tail.cubicTo(16*s, 14*s, 14*s, 12*s, 12*s, 6*s);
        tail.closeSubpath();
        p.drawPath(tail);
        
        // 后腿
        p.setBrush(bodyColor);
        p.drawRoundedRect(QRectF(5*s, 6*s, 6*s, 10*s), 3*s, 3*s);
        p.drawRoundedRect(QRectF(0, 7*s, 6*s, 9*s), 3*s, 3*s);
        
        // 前腿
        p.drawRoundedRect(QRectF(-8*s, 6*s, 6*s, 10*s), 3*s, 3*s);
        p.drawRoundedRect(QRectF(-13*s, 7*s, 6*s, 9*s), 3*s, 3*s);
        
        // 蹄子
        p.setBrush(QColor(200, 190, 210));
        p.drawEllipse(QPointF(8*s, 16*s), 3*s, 2*s);
        p.drawEllipse(QPointF(3*s, 16*s), 3*s, 2*s);
        p.drawEllipse(QPointF(-5*s, 16*s), 3*s, 2*s);
        p.drawEllipse(QPointF(-10*s, 16*s), 3*s, 2*s);
        
        // 身体
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(0, 2*s), 14*s, 10*s);
        
        // 身体上的星星装饰
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 230, 150, 200));
        drawFourPointStar(p, -4*s, 0, 2.5*s);
        drawFourPointStar(p, 4*s, 4*s, 2*s);
        
        p.setPen(QPen(outlineColor, 0.6*s));
        
        // 脖子
        p.setBrush(bodyColor);
        QPainterPath neck;
        neck.moveTo(-10*s, -2*s);
        neck.cubicTo(-14*s, -8*s, -14*s, -16*s, -10*s, -18*s);
        neck.lineTo(-4*s, -14*s);
        neck.cubicTo(-6*s, -10*s, -6*s, -4*s, -6*s, 0);
        neck.closeSubpath();
        p.drawPath(neck);
        
        // 头部
        p.drawEllipse(QPointF(-10*s, -22*s), 8*s, 7*s);
        
        // 嘴巴
        p.drawEllipse(QPointF(-16*s, -20*s), 3*s, 2.5*s);
        
        // 耳朵
        QPainterPath ear;
        ear.moveTo(-14*s, -28*s);
        ear.quadTo(-16*s, -35*s, -12*s, -30*s);
        ear.closeSubpath();
        p.drawPath(ear);
        
        QPainterPath ear2;
        ear2.moveTo(-6*s, -28*s);
        ear2.quadTo(-4*s, -35*s, -8*s, -30*s);
        ear2.closeSubpath();
        p.drawPath(ear2);
        
        // 角
        p.setBrush(hornColor);
        QPainterPath horn;
        horn.moveTo(-10*s, -29*s);
        horn.lineTo(-8*s, -40*s);
        horn.lineTo(-6*s, -29*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // 鬃毛
        p.setBrush(maneColor);
        QPainterPath mane;
        mane.moveTo(-4*s, -28*s);
        mane.cubicTo(2*s, -26*s, 4*s, -18*s, 2*s, -10*s);
        mane.cubicTo(0, -4*s, 2*s, 0, 4*s, 4*s);
        mane.lineTo(0, 2*s);
        mane.cubicTo(-2*s, -2*s, -4*s, -8*s, -2*s, -16*s);
        mane.cubicTo(0, -22*s, -2*s, -26*s, -4*s, -28*s);
        mane.closeSubpath();
        p.drawPath(mane);
        
        // 眼睛
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(50, 50, 70));
        p.drawEllipse(QPointF(-12*s, -22*s), 2*s, 2.5*s);
        
        // 眼睛高光
        p.setBrush(Qt::white);
        p.drawEllipse(QPointF(-13*s, -23*s), 0.8*s, 0.8*s);
        
        // 腮红
        p.setBrush(QColor(255, 180, 180, 120));
        p.drawEllipse(QPointF(-16*s, -18*s), 2.5*s, 1.5*s);
        
        // 项链
        p.setPen(QPen(QColor(180, 220, 210), 0.5*s));
        p.setBrush(Qt::NoBrush);
        QPainterPath necklace;
        necklace.moveTo(-14*s, -12*s);
        necklace.quadTo(-10*s, -8*s, -6*s, -12*s);
        p.drawPath(necklace);
        
        // 项链珠子
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(180, 220, 210));
        p.drawEllipse(QPointF(-14*s, -12*s), 1*s, 1*s);
        p.drawEllipse(QPointF(-10*s, -10*s), 1.2*s, 1.2*s);
        p.drawEllipse(QPointF(-6*s, -12*s), 1*s, 1*s);
    }
    
    // 类型3：坐着的独角兽（戴蝴蝶结）
    void drawSittingUnicorn(QPainter &p, double s, QColor bodyColor, QColor maneColor, QColor hornColor, QColor outlineColor, QColor bowColor) {
        p.setPen(QPen(outlineColor, 0.6*s));
        
        // 尾巴
        p.setBrush(maneColor);
        QPainterPath tail;
        tail.moveTo(10*s, 6*s);
        tail.cubicTo(14*s, 4*s, 18*s, 8*s, 16*s, 14*s);
        tail.cubicTo(14*s, 18*s, 12*s, 16*s, 10*s, 10*s);
        tail.closeSubpath();
        p.drawPath(tail);
        
        // 后腿（坐姿）
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(4*s, 10*s), 7*s, 5*s);
        
        // 前腿
        p.drawRoundedRect(QRectF(-10*s, 6*s, 6*s, 10*s), 3*s, 3*s);
        p.drawRoundedRect(QRectF(-5*s, 7*s, 6*s, 9*s), 3*s, 3*s);
        
        // 蹄子
        p.setBrush(QColor(200, 190, 210));
        p.drawEllipse(QPointF(-7*s, 16*s), 3*s, 2*s);
        p.drawEllipse(QPointF(-2*s, 16*s), 3*s, 2*s);
        
        // 身体
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(0, 4*s), 12*s, 10*s);
        
        // 脖子
        QPainterPath neck;
        neck.moveTo(-8*s, -2*s);
        neck.cubicTo(-12*s, -8*s, -12*s, -16*s, -8*s, -18*s);
        neck.lineTo(-2*s, -14*s);
        neck.cubicTo(-4*s, -10*s, -4*s, -4*s, -4*s, 0);
        neck.closeSubpath();
        p.drawPath(neck);
        
        // 头部
        p.drawEllipse(QPointF(-8*s, -22*s), 8*s, 7*s);
        
        // 嘴巴
        p.drawEllipse(QPointF(-14*s, -20*s), 3*s, 2.5*s);
        
        // 耳朵
        QPainterPath ear;
        ear.moveTo(-12*s, -28*s);
        ear.quadTo(-14*s, -35*s, -10*s, -30*s);
        ear.closeSubpath();
        p.drawPath(ear);
        
        QPainterPath ear2;
        ear2.moveTo(-4*s, -28*s);
        ear2.quadTo(-2*s, -35*s, -6*s, -30*s);
        ear2.closeSubpath();
        p.drawPath(ear2);
        
        // 角
        p.setBrush(hornColor);
        QPainterPath horn;
        horn.moveTo(-8*s, -29*s);
        horn.lineTo(-6*s, -40*s);
        horn.lineTo(-4*s, -29*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // 鬃毛
        p.setBrush(maneColor);
        QPainterPath mane;
        mane.moveTo(-2*s, -28*s);
        mane.cubicTo(4*s, -26*s, 6*s, -18*s, 4*s, -10*s);
        mane.cubicTo(2*s, -4*s, 4*s, 0, 6*s, 4*s);
        mane.lineTo(2*s, 2*s);
        mane.cubicTo(0, -2*s, -2*s, -8*s, 0, -16*s);
        mane.cubicTo(2*s, -22*s, 0, -26*s, -2*s, -28*s);
        mane.closeSubpath();
        p.drawPath(mane);
        
        // 眼睛
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(50, 50, 70));
        p.drawEllipse(QPointF(-10*s, -22*s), 2*s, 2.5*s);
        
        // 眼睛高光
        p.setBrush(Qt::white);
        p.drawEllipse(QPointF(-11*s, -23*s), 0.8*s, 0.8*s);
        
        // 腮红
        p.setBrush(QColor(255, 180, 180, 120));
        p.drawEllipse(QPointF(-14*s, -18*s), 2.5*s, 1.5*s);
        
        // 大蝴蝶结
        drawBow(p, -6*s, -10*s, 1.8*s, bowColor, outlineColor);
        drawBell(p, -6*s, -4*s, 1*s, QColor(255, 220, 100));
    }
    
    // 类型4：躺在云上的飞马独角兽
    void drawCloudUnicorn(QPainter &p, double s, QColor bodyColor, QColor maneColor, QColor hornColor, QColor outlineColor) {
        // 云朵
        p.setPen(QPen(QColor(200, 210, 220), 0.4*s));
        p.setBrush(QColor(240, 248, 255, 200));
        
        p.drawEllipse(QPointF(-8*s, 14*s), 10*s, 6*s);
        p.drawEllipse(QPointF(6*s, 12*s), 8*s, 5*s);
        p.drawEllipse(QPointF(-2*s, 10*s), 7*s, 5*s);
        p.drawEllipse(QPointF(12*s, 14*s), 6*s, 4*s);
        
        p.setPen(QPen(outlineColor, 0.6*s));
        
        // 尾巴 - 彩虹色
        QLinearGradient tailGrad(8*s, 4*s, 18*s, 10*s);
        tailGrad.setColorAt(0, maneColor);
        tailGrad.setColorAt(0.5, QColor(180, 220, 210));
        tailGrad.setColorAt(1, QColor(200, 180, 220));
        p.setBrush(tailGrad);
        
        QPainterPath tail;
        tail.moveTo(8*s, 4*s);
        tail.cubicTo(12*s, 2*s, 18*s, 4*s, 16*s, 10*s);
        tail.cubicTo(14*s, 14*s, 10*s, 12*s, 8*s, 8*s);
        tail.closeSubpath();
        p.drawPath(tail);
        
        // 后腿（躺姿）
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(4*s, 6*s), 5*s, 4*s);
        
        // 身体
        p.drawEllipse(QPointF(-2*s, 2*s), 12*s, 8*s);
        
        // 翅膀
        p.setBrush(QColor(255, 255, 255, 220));
        QPainterPath wing;
        wing.moveTo(-4*s, -2*s);
        wing.cubicTo(-8*s, -10*s, 0, -16*s, 6*s, -12*s);
        wing.cubicTo(10*s, -10*s, 8*s, -6*s, 4*s, -4*s);
        wing.cubicTo(2*s, -6*s, -2*s, -4*s, -4*s, -2*s);
        wing.closeSubpath();
        p.drawPath(wing);
        
        // 翅膀羽毛纹路
        p.setPen(QPen(QColor(200, 210, 220), 0.3*s));
        p.drawLine(QPointF(-2*s, -4*s), QPointF(2*s, -10*s));
        p.drawLine(QPointF(0, -4*s), QPointF(4*s, -10*s));
        p.drawLine(QPointF(2*s, -4*s), QPointF(6*s, -8*s));
        
        p.setPen(QPen(outlineColor, 0.6*s));
        
        // 前腿
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(-10*s, 4*s), 4*s, 3*s);
        
        // 脖子
        QPainterPath neck;
        neck.moveTo(-10*s, -2*s);
        neck.cubicTo(-14*s, -6*s, -14*s, -14*s, -10*s, -16*s);
        neck.lineTo(-6*s, -12*s);
        neck.cubicTo(-8*s, -8*s, -8*s, -4*s, -8*s, 0);
        neck.closeSubpath();
        p.drawPath(neck);
        
        // 头部
        p.drawEllipse(QPointF(-12*s, -20*s), 8*s, 7*s);
        
        // 嘴巴
        p.drawEllipse(QPointF(-18*s, -18*s), 3*s, 2.5*s);
        
        // 耳朵
        QPainterPath ear;
        ear.moveTo(-16*s, -26*s);
        ear.quadTo(-18*s, -32*s, -14*s, -28*s);
        ear.closeSubpath();
        p.drawPath(ear);
        
        QPainterPath ear2;
        ear2.moveTo(-8*s, -26*s);
        ear2.quadTo(-6*s, -32*s, -10*s, -28*s);
        ear2.closeSubpath();
        p.drawPath(ear2);
        
        // 角
        p.setBrush(hornColor);
        QPainterPath horn;
        horn.moveTo(-12*s, -27*s);
        horn.lineTo(-10*s, -38*s);
        horn.lineTo(-8*s, -27*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // 鬃毛 - 彩虹色
        QLinearGradient maneGrad(-6*s, -26*s, 4*s, 2*s);
        maneGrad.setColorAt(0, maneColor);
        maneGrad.setColorAt(0.3, QColor(180, 220, 210));
        maneGrad.setColorAt(0.6, QColor(200, 180, 220));
        maneGrad.setColorAt(1, maneColor.lighter(110));
        p.setBrush(maneGrad);
        
        QPainterPath mane;
        mane.moveTo(-6*s, -26*s);
        mane.cubicTo(0, -24*s, 4*s, -16*s, 2*s, -8*s);
        mane.cubicTo(0, -2*s, 2*s, 2*s, 4*s, 6*s);
        mane.lineTo(0, 4*s);
        mane.cubicTo(-2*s, 0, -4*s, -6*s, -2*s, -14*s);
        mane.cubicTo(0, -20*s, -4*s, -24*s, -6*s, -26*s);
        mane.closeSubpath();
        p.drawPath(mane);
        
        // 闭着的眼睛（睡觉）
        p.setPen(QPen(QColor(50, 50, 70), 0.6*s));
        p.setBrush(Qt::NoBrush);
        QPainterPath closedEye;
        closedEye.moveTo(-15*s, -20*s);
        closedEye.quadTo(-13*s, -18*s, -11*s, -20*s);
        p.drawPath(closedEye);
        
        // 腮红
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 180, 180, 120));
        p.drawEllipse(QPointF(-18*s, -16*s), 2.5*s, 1.5*s);
        
        // 项链
        p.setBrush(QColor(255, 200, 200));
        p.drawEllipse(QPointF(-14*s, -10*s), 1*s, 1*s);
        p.drawEllipse(QPointF(-12*s, -9*s), 0.8*s, 0.8*s);
        p.drawEllipse(QPointF(-10*s, -10*s), 1*s, 1*s);
        
        // 云上的星星
        p.setBrush(QColor(255, 230, 150, 180));
        drawFourPointStar(p, 14*s, 10*s, 2*s);
        drawFourPointStar(p, -14*s, 12*s, 1.5*s);
    }
    
    // 绘制独角兽图案（自绘制版本）
    void drawUnicorns(QPainter &p) {
        int w = width();
        int h = height();
        
        int area = w * h;
        
        // 根据窗口大小调整密度：大窗口（主界面）密度高，小窗口（设置界面）密度低
        int unicornCount;
        if (area < 200000) {
            // 小窗口（设置界面）：2-4个
            unicornCount = qBound(2, area / 50000, 4);
        } else {
            // 大窗口（主界面）：5-15个
            unicornCount = qBound(5, area / 40000, 15);
        }
        
        qsrand(static_cast<uint>(QDateTime::currentDateTime().toSecsSinceEpoch() / 60));
        
        QList<QRect> occupiedRects;
        
        // 颜色方案
        struct ColorScheme {
            QColor body, mane, horn, outline, accent;
        };
        
        ColorScheme schemes[4] = {
            // 粉色系
            {QColor(255, 245, 248), QColor(255, 182, 193), QColor(255, 220, 120), QColor(200, 150, 160), QColor(180, 220, 210)},
            // 紫色系
            {QColor(250, 245, 255), QColor(200, 170, 210), QColor(255, 210, 100), QColor(160, 140, 180), QColor(200, 180, 220)},
            // 蓝绿系
            {QColor(248, 255, 255), QColor(180, 220, 210), QColor(255, 215, 100), QColor(150, 180, 180), QColor(180, 200, 220)},
            // 米白系
            {QColor(255, 252, 245), QColor(230, 210, 190), QColor(255, 200, 80), QColor(180, 160, 140), QColor(220, 200, 180)}
        };
        
        for (int i = 0; i < unicornCount; ++i) {
            // 大小变化：1.0 到 2.0
            double scale = 1.0 + (qrand() % 11) / 10.0;
            int size = static_cast<int>(50 * scale);
            
            int x, y;
            bool found = false;
            
            for (int attempt = 0; attempt < unicornCount * 20; ++attempt) {
                x = size + qrand() % qMax(1, w - size * 2);
                y = size + qrand() % qMax(1, h - size * 2);
                
                if (!isOverlapping(x, y, size, occupiedRects)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) continue;
            
            occupiedRects.append(QRect(x - size, y - size, size * 2, size * 2));
            
            p.setOpacity(0.18 + (qrand() % 12) / 100.0);
            
            int colorIndex = qrand() % 4;
            ColorScheme &cs = schemes[colorIndex];
            
            p.save();
            p.translate(x, y);
            if (qrand() % 2 == 0) p.scale(-1, 1);
            
            // 随机选择4种独角兽类型
            int type = qrand() % 4;
            switch (type) {
                case 0:
                    drawRockingUnicorn(p, scale, cs.body, cs.mane, cs.horn, cs.outline, cs.accent);
                    break;
                case 1:
                    drawStandingUnicorn(p, scale, cs.body, cs.mane, cs.horn, cs.outline);
                    break;
                case 2:
                    drawSittingUnicorn(p, scale, cs.body, cs.mane, cs.horn, cs.outline, cs.accent);
                    break;
                case 3:
                    drawCloudUnicorn(p, scale, cs.body, cs.mane, cs.horn, cs.outline);
                    break;
            }
            
            p.restore();
        }
        
        // 添加小星星装饰
        p.setOpacity(0.3);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 230, 150));
        
        int starCount = qBound(4, area / 30000, 25);
        for (int i = 0; i < starCount; ++i) {
            int sx = 15 + qrand() % qMax(1, w - 30);
            int sy = 15 + qrand() % qMax(1, h - 30);
            double sr = 3 + qrand() % 8;
            
            if (!isOverlapping(sx, sy, static_cast<int>(sr * 2), occupiedRects)) {
                drawFourPointStar(p, sx, sy, sr);
            }
        }
        
        p.setOpacity(1.0);
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
