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
    // ========== 形态1：站立款（参考图右上 - 淡粉身体+紫色鬃毛+项链+星星）==========
    void drawUnicornStanding(QPainter &p, double s, int alpha, 
                             QColor bodyColor, QColor maneColor, QColor hornColor) {
        p.setPen(Qt::NoPen);
        
        // 尾巴 - 大波浪卷曲向上
        p.setBrush(maneColor);
        QPainterPath tail;
        tail.moveTo(18*s, 2*s);
        tail.cubicTo(22*s, -4*s, 28*s, -2*s, 30*s, 6*s);
        tail.cubicTo(32*s, 14*s, 28*s, 20*s, 24*s, 18*s);
        tail.cubicTo(20*s, 16*s, 22*s, 10*s, 26*s, 8*s);
        tail.cubicTo(28*s, 6*s, 26*s, 2*s, 22*s, 4*s);
        tail.cubicTo(20*s, 6*s, 18*s, 4*s, 18*s, 2*s);
        p.drawPath(tail);
        
        // 蹄子（4个小椭圆在腿底）
        p.setBrush(QColor(maneColor.red(), maneColor.green(), maneColor.blue(), alpha * 0.7));
        p.drawEllipse(QPointF(12*s, 22*s), 3*s, 2*s);
        p.drawEllipse(QPointF(6*s, 20*s), 3*s, 2*s);
        p.drawEllipse(QPointF(-8*s, 22*s), 3*s, 2*s);
        p.drawEllipse(QPointF(-14*s, 20*s), 3*s, 2*s);
        
        // 后腿
        p.setBrush(bodyColor);
        QPainterPath backLeg1, backLeg2;
        backLeg1.moveTo(14*s, 6*s);
        backLeg1.cubicTo(16*s, 10*s, 15*s, 16*s, 12*s, 20*s);
        backLeg1.cubicTo(10*s, 20*s, 10*s, 16*s, 11*s, 6*s);
        backLeg1.closeSubpath();
        backLeg2.moveTo(8*s, 8*s);
        backLeg2.cubicTo(10*s, 12*s, 9*s, 16*s, 6*s, 18*s);
        backLeg2.cubicTo(4*s, 18*s, 4*s, 14*s, 5*s, 8*s);
        backLeg2.closeSubpath();
        p.drawPath(backLeg1);
        p.drawPath(backLeg2);
        
        // 身体 - 圆润椭圆
        p.drawEllipse(QPointF(0, 0), 16*s, 10*s);
        
        // 前腿
        QPainterPath frontLeg1, frontLeg2;
        frontLeg1.moveTo(-6*s, 6*s);
        frontLeg1.cubicTo(-5*s, 12*s, -6*s, 18*s, -8*s, 20*s);
        frontLeg1.cubicTo(-10*s, 20*s, -10*s, 14*s, -9*s, 6*s);
        frontLeg1.closeSubpath();
        frontLeg2.moveTo(-12*s, 4*s);
        frontLeg2.cubicTo(-11*s, 10*s, -12*s, 16*s, -14*s, 18*s);
        frontLeg2.cubicTo(-16*s, 18*s, -16*s, 12*s, -15*s, 4*s);
        frontLeg2.closeSubpath();
        p.drawPath(frontLeg1);
        p.drawPath(frontLeg2);
        
        // 脖子 - 优雅弯曲
        QPainterPath neck;
        neck.moveTo(-12*s, -4*s);
        neck.cubicTo(-14*s, -10*s, -16*s, -18*s, -12*s, -24*s);
        neck.cubicTo(-10*s, -20*s, -8*s, -14*s, -8*s, -6*s);
        neck.closeSubpath();
        p.drawPath(neck);
        
        // 头 - 圆形
        p.drawEllipse(QPointF(-14*s, -28*s), 9*s, 8*s);
        // 嘴部突出
        p.drawEllipse(QPointF(-22*s, -26*s), 4*s, 3*s);
        
        // 耳朵 - 尖三角
        QPainterPath ear;
        ear.moveTo(-10*s, -34*s);
        ear.quadTo(-8*s, -42*s, -6*s, -34*s);
        ear.closeSubpath();
        p.drawPath(ear);
        p.setBrush(QColor(255, 220, 220, alpha * 0.6));
        QPainterPath earIn;
        earIn.moveTo(-9*s, -35*s);
        earIn.quadTo(-8*s, -40*s, -7*s, -35*s);
        earIn.closeSubpath();
        p.drawPath(earIn);
        
        // 独角 - 金色螺旋
        p.setBrush(hornColor);
        QPainterPath horn;
        horn.moveTo(-12*s, -35*s);
        horn.lineTo(-10*s, -50*s);
        horn.lineTo(-8*s, -35*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // 鬃毛 - 大波浪从头顶到背部
        p.setBrush(maneColor);
        QPainterPath mane;
        mane.moveTo(-6*s, -32*s);
        mane.cubicTo(0*s, -38*s, 6*s, -32*s, 4*s, -24*s);
        mane.cubicTo(2*s, -18*s, 8*s, -16*s, 12*s, -20*s);
        mane.cubicTo(16*s, -24*s, 18*s, -16*s, 14*s, -8*s);
        mane.cubicTo(10*s, -2*s, 6*s, -6*s, 2*s, -10*s);
        mane.cubicTo(-2*s, -14*s, -4*s, -18*s, -2*s, -24*s);
        mane.cubicTo(0*s, -28*s, -4*s, -30*s, -6*s, -32*s);
        p.drawPath(mane);
        
        // 项链
        p.setPen(QPen(QColor(180, 210, 200, alpha), 1*s));
        p.setBrush(Qt::NoBrush);
        QPainterPath necklace;
        necklace.moveTo(-18*s, -22*s);
        necklace.quadTo(-12*s, -16*s, -6*s, -18*s);
        p.drawPath(necklace);
        p.setPen(Qt::NoPen);
        
        // 身上星星装饰
        p.setBrush(QColor(255, 235, 180, alpha * 0.8));
        drawStar(p, -2*s, -2*s, 3*s);
        drawStar(p, 6*s, 2*s, 2.5*s);
        
        // 眼睛
        p.setBrush(QColor(40, 40, 40, alpha));
        p.drawEllipse(QPointF(-16*s, -28*s), 2*s, 2.5*s);
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPointF(-17*s, -29*s), 0.8*s, 0.8*s);
    }
    
    // ========== 形态2：坐姿款（参考图左下 - 白身体+粉鬃毛+蝴蝶结铃铛）==========
    void drawUnicornSitting(QPainter &p, double s, int alpha,
                            QColor bodyColor, QColor maneColor, QColor hornColor, QColor bowColor) {
        p.setPen(Qt::NoPen);
        
        // 尾巴 - 向右下卷曲
        p.setBrush(maneColor);
        QPainterPath tail;
        tail.moveTo(10*s, 8*s);
        tail.cubicTo(16*s, 4*s, 22*s, 8*s, 24*s, 16*s);
        tail.cubicTo(26*s, 24*s, 22*s, 30*s, 18*s, 28*s);
        tail.cubicTo(14*s, 26*s, 16*s, 20*s, 20*s, 18*s);
        tail.cubicTo(22*s, 16*s, 20*s, 12*s, 16*s, 14*s);
        tail.cubicTo(12*s, 16*s, 10*s, 12*s, 10*s, 8*s);
        p.drawPath(tail);
        
        // 后腿/臀部 - 坐着的圆形
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(6*s, 12*s), 10*s, 8*s);
        
        // 身体 - 坐姿更圆
        p.drawEllipse(QPointF(-2*s, 4*s), 12*s, 12*s);
        
        // 前腿蹄子
        p.setBrush(QColor(maneColor.red(), maneColor.green(), maneColor.blue(), alpha * 0.7));
        p.drawEllipse(QPointF(-18*s, 22*s), 3*s, 2*s);
        p.drawEllipse(QPointF(-12*s, 20*s), 3*s, 2*s);
        
        // 前腿 - 向前伸出
        p.setBrush(bodyColor);
        QPainterPath frontLeg1, frontLeg2;
        frontLeg1.moveTo(-10*s, 8*s);
        frontLeg1.cubicTo(-12*s, 14*s, -16*s, 18*s, -18*s, 20*s);
        frontLeg1.cubicTo(-20*s, 20*s, -18*s, 14*s, -14*s, 8*s);
        frontLeg1.closeSubpath();
        frontLeg2.moveTo(-6*s, 10*s);
        frontLeg2.cubicTo(-8*s, 14*s, -10*s, 18*s, -12*s, 18*s);
        frontLeg2.cubicTo(-14*s, 18*s, -12*s, 12*s, -8*s, 10*s);
        frontLeg2.closeSubpath();
        p.drawPath(frontLeg1);
        p.drawPath(frontLeg2);
        
        // 头 - 圆形，稍大
        p.drawEllipse(QPointF(-8*s, -16*s), 10*s, 9*s);
        // 嘴部
        p.drawEllipse(QPointF(-16*s, -14*s), 4*s, 3*s);
        
        // 耳朵
        QPainterPath ear;
        ear.moveTo(-4*s, -24*s);
        ear.quadTo(-2*s, -34*s, 0*s, -24*s);
        ear.closeSubpath();
        p.drawPath(ear);
        p.setBrush(QColor(255, 220, 220, alpha * 0.6));
        QPainterPath earIn;
        earIn.moveTo(-3*s, -25*s);
        earIn.quadTo(-2*s, -32*s, -1*s, -25*s);
        earIn.closeSubpath();
        p.drawPath(earIn);
        
        // 独角
        p.setBrush(hornColor);
        QPainterPath horn;
        horn.moveTo(-6*s, -25*s);
        horn.lineTo(-4*s, -42*s);
        horn.lineTo(-2*s, -25*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // 鬃毛 - 从头顶飘向背后
        p.setBrush(maneColor);
        QPainterPath mane;
        mane.moveTo(2*s, -22*s);
        mane.cubicTo(8*s, -28*s, 14*s, -22*s, 12*s, -14*s);
        mane.cubicTo(10*s, -8*s, 14*s, -4*s, 18*s, -8*s);
        mane.cubicTo(14*s, 0*s, 8*s, -2*s, 6*s, -8*s);
        mane.cubicTo(4*s, -14*s, 6*s, -18*s, 2*s, -22*s);
        p.drawPath(mane);
        
        // 大蝴蝶结（颈部）
        p.setBrush(bowColor);
        QPainterPath bow;
        // 左翼
        bow.moveTo(-8*s, -2*s);
        bow.cubicTo(-18*s, -10*s, -20*s, 4*s, -8*s, -2*s);
        // 右翼
        bow.cubicTo(4*s, -10*s, 6*s, 4*s, -8*s, -2*s);
        p.drawPath(bow);
        // 蝴蝶结尾带
        QPainterPath bowTail;
        bowTail.moveTo(-10*s, 0*s);
        bowTail.cubicTo(-14*s, 8*s, -12*s, 14*s, -10*s, 12*s);
        bowTail.cubicTo(-8*s, 10*s, -10*s, 4*s, -8*s, 0*s);
        bowTail.moveTo(-6*s, 0*s);
        bowTail.cubicTo(-2*s, 6*s, -4*s, 12*s, -6*s, 10*s);
        bowTail.cubicTo(-8*s, 8*s, -6*s, 4*s, -8*s, 0*s);
        p.drawPath(bowTail);
        // 铃铛
        p.setBrush(QColor(255, 220, 100, alpha));
        p.drawEllipse(QPointF(-8*s, -2*s), 2.5*s, 2.5*s);
        
        // 眼睛
        p.setBrush(QColor(40, 40, 40, alpha));
        p.drawEllipse(QPointF(-10*s, -16*s), 2*s, 2.5*s);
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPointF(-11*s, -17*s), 0.8*s, 0.8*s);
        
        // 腮红
        p.setBrush(QColor(255, 200, 210, alpha * 0.35));
        p.drawEllipse(QPointF(-14*s, -12*s), 2*s, 1.5*s);
    }
    
    // ========== 形态3：飞马款（参考图右下 - 躺在云上+翅膀+彩虹鬃毛）==========
    void drawUnicornPegasus(QPainter &p, double s, int alpha,
                            QColor bodyColor, QColor maneColor, QColor hornColor) {
        p.setPen(Qt::NoPen);
        
        // 云朵（分层绘制更蓬松）
        p.setBrush(QColor(255, 255, 255, alpha * 0.5));
        p.drawEllipse(QPointF(-8*s, 22*s), 16*s, 8*s);
        p.drawEllipse(QPointF(12*s, 20*s), 14*s, 7*s);
        p.drawEllipse(QPointF(-20*s, 24*s), 10*s, 5*s);
        p.drawEllipse(QPointF(24*s, 22*s), 12*s, 6*s);
        p.setBrush(QColor(255, 255, 255, alpha * 0.7));
        p.drawEllipse(QPointF(0*s, 18*s), 20*s, 9*s);
        
        // 尾巴 - 彩色渐变波浪（紫+青）
        p.setBrush(maneColor);
        QPainterPath tail;
        tail.moveTo(22*s, 4*s);
        tail.cubicTo(30*s, 0*s, 36*s, 8*s, 34*s, 18*s);
        tail.cubicTo(32*s, 26*s, 26*s, 24*s, 28*s, 16*s);
        tail.cubicTo(30*s, 10*s, 26*s, 6*s, 22*s, 8*s);
        tail.cubicTo(20*s, 10*s, 20*s, 6*s, 22*s, 4*s);
        p.drawPath(tail);
        // 尾巴第二层色
        p.setBrush(QColor(180, 210, 200, alpha));
        QPainterPath tail2;
        tail2.moveTo(24*s, 6*s);
        tail2.cubicTo(30*s, 4*s, 34*s, 12*s, 32*s, 20*s);
        tail2.cubicTo(30*s, 26*s, 26*s, 22*s, 28*s, 14*s);
        tail2.cubicTo(30*s, 8*s, 26*s, 8*s, 24*s, 6*s);
        p.drawPath(tail2);
        
        // 身体 - 横躺
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(2*s, 6*s), 18*s, 10*s);
        
        // 腿 - 收起
        p.drawEllipse(QPointF(14*s, 12*s), 5*s, 4*s);
        p.drawEllipse(QPointF(-10*s, 12*s), 5*s, 4*s);
        
        // 翅膀 - 展开
        p.setBrush(QColor(255, 255, 255, alpha * 0.9));
        QPainterPath wing;
        wing.moveTo(-2*s, 0*s);
        wing.cubicTo(-6*s, -12*s, 4*s, -22*s, 14*s, -16*s);
        wing.cubicTo(18*s, -12*s, 14*s, -6*s, 8*s, -4*s);
        wing.cubicTo(4*s, -2*s, 0*s, 0*s, -2*s, 0*s);
        p.drawPath(wing);
        // 翅膀羽毛纹路
        p.setPen(QPen(QColor(230, 220, 240, alpha * 0.5), 0.8*s));
        p.drawLine(QPointF(0*s, -4*s), QPointF(6*s, -12*s));
        p.drawLine(QPointF(2*s, -2*s), QPointF(10*s, -10*s));
        p.drawLine(QPointF(4*s, -2*s), QPointF(12*s, -8*s));
        p.setPen(Qt::NoPen);
        
        // 头
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(-18*s, 0*s), 10*s, 8*s);
        // 嘴
        p.drawEllipse(QPointF(-26*s, 2*s), 4*s, 3*s);
        
        // 耳朵
        QPainterPath ear;
        ear.moveTo(-14*s, -6*s);
        ear.quadTo(-12*s, -14*s, -10*s, -6*s);
        ear.closeSubpath();
        p.drawPath(ear);
        
        // 独角
        p.setBrush(hornColor);
        QPainterPath horn;
        horn.moveTo(-16*s, -6*s);
        horn.lineTo(-14*s, -20*s);
        horn.lineTo(-12*s, -6*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // 鬃毛 - 彩色渐变（紫粉青）
        p.setBrush(QColor(200, 180, 210, alpha)); // 紫
        QPainterPath mane1;
        mane1.moveTo(-8*s, -4*s);
        mane1.cubicTo(-2*s, -10*s, 4*s, -6*s, 2*s, 0*s);
        mane1.cubicTo(0*s, 4*s, -6*s, 2*s, -8*s, -4*s);
        p.drawPath(mane1);
        p.setBrush(QColor(180, 200, 200, alpha)); // 青
        QPainterPath mane2;
        mane2.moveTo(-6*s, -2*s);
        mane2.cubicTo(0*s, -8*s, 6*s, -4*s, 4*s, 2*s);
        mane2.cubicTo(2*s, 6*s, -4*s, 4*s, -6*s, -2*s);
        p.drawPath(mane2);
        
        // 项链
        p.setPen(QPen(QColor(255, 200, 200, alpha), 1*s));
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(-24*s, -2*s, 12*s, 8*s), 30*16, 120*16);
        p.setPen(Qt::NoPen);
        
        // 身上星星
        p.setBrush(QColor(255, 235, 180, alpha * 0.7));
        drawStar(p, 6*s, 4*s, 2.5*s);
        
        // 眼睛 - 闭着睡觉
        p.setPen(QPen(QColor(40, 40, 40, alpha), 1.2*s));
        p.drawArc(QRectF(-22*s, -2*s, 5*s, 4*s), 0, 180*16);
        p.setPen(Qt::NoPen);
        
        // 腮红
        p.setBrush(QColor(255, 200, 210, alpha * 0.3));
        p.drawEllipse(QPointF(-24*s, 4*s), 2*s, 1.5*s);
    }
    
    // ========== 形态4：木马款（参考图左上 - 摇摇椅+蝴蝶结铃铛）==========
    void drawUnicornRocking(QPainter &p, double s, int alpha,
                            QColor bodyColor, QColor maneColor, QColor hornColor, QColor baseColor) {
        p.setPen(Qt::NoPen);
        
        // 底座装饰星星
        p.setBrush(QColor(255, 235, 180, alpha * 0.8));
        drawStar(p, -22*s, 24*s, 3*s);
        drawStar(p, 22*s, 24*s, 3*s);
        
        // 摇摇椅底座 - 弧形
        p.setBrush(baseColor);
        QPainterPath base;
        base.moveTo(-26*s, 28*s);
        base.quadTo(0*s, 36*s, 26*s, 28*s);
        base.lineTo(24*s, 32*s);
        base.quadTo(0*s, 38*s, -24*s, 32*s);
        base.closeSubpath();
        p.drawPath(base);
        
        // 底座两端卷曲装饰
        QPainterPath baseEnd1, baseEnd2;
        baseEnd1.moveTo(-26*s, 28*s);
        baseEnd1.cubicTo(-30*s, 26*s, -30*s, 32*s, -26*s, 30*s);
        baseEnd2.moveTo(26*s, 28*s);
        baseEnd2.cubicTo(30*s, 26*s, 30*s, 32*s, 26*s, 30*s);
        p.drawPath(baseEnd1);
        p.drawPath(baseEnd2);
        
        // 支撑柱
        p.drawRoundedRect(QRectF(-8*s, 16*s, 4*s, 14*s), 1*s, 1*s);
        p.drawRoundedRect(QRectF(6*s, 14*s, 4*s, 16*s), 1*s, 1*s);
        
        // 尾巴 - 大卷曲
        p.setBrush(maneColor);
        QPainterPath tail;
        tail.moveTo(14*s, 2*s);
        tail.cubicTo(20*s, -2*s, 26*s, 4*s, 24*s, 14*s);
        tail.cubicTo(22*s, 22*s, 16*s, 20*s, 18*s, 12*s);
        tail.cubicTo(20*s, 6*s, 16*s, 4*s, 14*s, 6*s);
        tail.cubicTo(12*s, 8*s, 12*s, 4*s, 14*s, 2*s);
        p.drawPath(tail);
        
        // 后腿
        p.setBrush(bodyColor);
        QPainterPath backLeg;
        backLeg.moveTo(10*s, 6*s);
        backLeg.cubicTo(12*s, 10*s, 10*s, 14*s, 8*s, 16*s);
        backLeg.cubicTo(6*s, 16*s, 6*s, 12*s, 7*s, 6*s);
        backLeg.closeSubpath();
        p.drawPath(backLeg);
        
        // 身体
        p.drawEllipse(QPointF(0, 0), 14*s, 9*s);
        
        // 前腿
        QPainterPath frontLeg;
        frontLeg.moveTo(-8*s, 4*s);
        frontLeg.cubicTo(-7*s, 10*s, -8*s, 14*s, -10*s, 16*s);
        frontLeg.cubicTo(-12*s, 16*s, -12*s, 10*s, -11*s, 4*s);
        frontLeg.closeSubpath();
        p.drawPath(frontLeg);
        
        // 脖子 - 向上弯曲
        QPainterPath neck;
        neck.moveTo(-10*s, -4*s);
        neck.cubicTo(-14*s, -12*s, -12*s, -22*s, -8*s, -28*s);
        neck.cubicTo(-6*s, -24*s, -6*s, -16*s, -6*s, -6*s);
        neck.closeSubpath();
        p.drawPath(neck);
        
        // 头 - 向上抬
        p.drawEllipse(QPointF(-10*s, -32*s), 8*s, 7*s);
        // 嘴
        p.drawEllipse(QPointF(-16*s, -30*s), 4*s, 3*s);
        
        // 耳朵
        QPainterPath ear;
        ear.moveTo(-6*s, -38*s);
        ear.quadTo(-4*s, -46*s, -2*s, -38*s);
        ear.closeSubpath();
        p.drawPath(ear);
        p.setBrush(QColor(255, 220, 220, alpha * 0.6));
        QPainterPath earIn;
        earIn.moveTo(-5*s, -39*s);
        earIn.quadTo(-4*s, -44*s, -3*s, -39*s);
        earIn.closeSubpath();
        p.drawPath(earIn);
        
        // 独角
        p.setBrush(hornColor);
        QPainterPath horn;
        horn.moveTo(-8*s, -38*s);
        horn.lineTo(-6*s, -54*s);
        horn.lineTo(-4*s, -38*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // 鬃毛
        p.setBrush(maneColor);
        QPainterPath mane;
        mane.moveTo(0*s, -36*s);
        mane.cubicTo(6*s, -42*s, 12*s, -34*s, 8*s, -26*s);
        mane.cubicTo(4*s, -20*s, 10*s, -14*s, 14*s, -18*s);
        mane.cubicTo(10*s, -10*s, 4*s, -14*s, 2*s, -20*s);
        mane.cubicTo(0*s, -26*s, 4*s, -32*s, 0*s, -36*s);
        p.drawPath(mane);
        
        // 蝴蝶结（脖子上）
        p.setBrush(baseColor);
        QPainterPath bow;
        bow.moveTo(-8*s, -18*s);
        bow.cubicTo(-14*s, -24*s, -16*s, -14*s, -8*s, -18*s);
        bow.cubicTo(0*s, -24*s, 2*s, -14*s, -8*s, -18*s);
        p.drawPath(bow);
        // 铃铛
        p.setBrush(QColor(255, 220, 100, alpha));
        p.drawEllipse(QPointF(-8*s, -18*s), 2*s, 2*s);
        
        // 眼睛
        p.setBrush(QColor(40, 40, 40, alpha));
        p.drawEllipse(QPointF(-12*s, -32*s), 1.8*s, 2.2*s);
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPointF(-13*s, -33*s), 0.6*s, 0.6*s);
    }
    
    // ========== 主绘制函数：随机选择形态 ==========
    void drawUnicorn(QPainter &p, int x, int y, int size, int alpha) {
        p.save();
        p.translate(x, y);
        
        double s = size / 60.0;
        int poseType = qrand() % 4;
        
        // 颜色组合 - 更精确匹配参考图
        QColor bodyColor, maneColor, hornColor, accentColor;
        
        switch (poseType) {
            case 0: // 站立款 - 淡粉身体+紫色鬃毛
                bodyColor = QColor(255, 240, 245, alpha);
                maneColor = QColor(195, 175, 205, alpha);
                hornColor = QColor(255, 235, 180, alpha);
                accentColor = QColor(180, 210, 200, alpha);
                break;
            case 1: // 坐姿款 - 白身体+粉色鬃毛+青绿蝴蝶结
                bodyColor = QColor(255, 255, 255, alpha);
                maneColor = QColor(255, 205, 215, alpha);
                hornColor = QColor(255, 235, 180, alpha);
                accentColor = QColor(170, 210, 205, alpha);
                break;
            case 2: // 飞马款 - 白身体+彩虹鬃毛（紫粉青）
                bodyColor = QColor(255, 255, 255, alpha);
                maneColor = QColor(200, 180, 210, alpha);
                hornColor = QColor(255, 235, 180, alpha);
                accentColor = QColor(180, 200, 200, alpha);
                break;
            default: // 木马款 - 白身体+粉色鬃毛+青绿底座
                bodyColor = QColor(255, 255, 255, alpha);
                maneColor = QColor(255, 200, 210, alpha);
                hornColor = QColor(255, 235, 180, alpha);
                accentColor = QColor(175, 210, 205, alpha);
                break;
        }
        
        switch (poseType) {
            case 0:
                drawUnicornStanding(p, s, alpha, bodyColor, maneColor, hornColor);
                break;
            case 1:
                drawUnicornSitting(p, s, alpha, bodyColor, maneColor, hornColor, accentColor);
                break;
            case 2:
                drawUnicornPegasus(p, s, alpha, bodyColor, maneColor, hornColor);
                break;
            case 3:
                drawUnicornRocking(p, s, alpha, bodyColor, maneColor, hornColor, accentColor);
                break;
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
