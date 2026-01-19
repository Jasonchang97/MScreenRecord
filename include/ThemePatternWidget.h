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
        QColor outlineColor(160, 145, 155, alpha * 0.6);
        
        // ===== 图层1: 尾巴（大S卷，带线条）=====
        p.setPen(QPen(outlineColor, 0.5*s));
        p.setBrush(maneColor);
        QPainterPath tail;
        tail.moveTo(12*s, 0*s);
        tail.cubicTo(16*s, -4*s, 20*s, -2*s, 22*s, 4*s);
        tail.cubicTo(24*s, 10*s, 22*s, 16*s, 18*s, 18*s);
        tail.cubicTo(14*s, 20*s, 12*s, 16*s, 14*s, 10*s);
        tail.cubicTo(16*s, 4*s, 14*s, 2*s, 12*s, 0*s);
        p.drawPath(tail);
        // 尾巴线条纹理
        p.setPen(QPen(outlineColor.lighter(120), 0.4*s));
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(14*s, 2*s), QPointF(18*s, 8*s));
        p.drawLine(QPointF(16*s, 6*s), QPointF(18*s, 14*s));
        
        // ===== 图层2: 蹄子 =====
        p.setPen(QPen(outlineColor, 0.4*s));
        p.setBrush(maneColor);
        p.drawEllipse(QPointF(9*s, 14*s), 2*s, 1.2*s);
        p.drawEllipse(QPointF(4*s, 13*s), 2*s, 1.2*s);
        p.drawEllipse(QPointF(-6*s, 14*s), 2*s, 1.2*s);
        p.drawEllipse(QPointF(-11*s, 13*s), 2*s, 1.2*s);
        
        // ===== 图层3: 后腿 =====
        p.setBrush(bodyColor);
        p.drawRoundedRect(QRectF(7*s, 4*s, 4*s, 10*s), 1.5*s, 1.5*s);
        p.drawRoundedRect(QRectF(2*s, 4*s, 4*s, 9*s), 1.5*s, 1.5*s);
        
        // ===== 图层4: 身体（扁椭圆）=====
        p.drawEllipse(QPointF(0, 0), 12*s, 7*s);
        
        // ===== 图层5: 前腿 =====
        p.drawRoundedRect(QRectF(-8*s, 3*s, 4*s, 11*s), 1.5*s, 1.5*s);
        p.drawRoundedRect(QRectF(-13*s, 3*s, 4*s, 10*s), 1.5*s, 1.5*s);
        
        // ===== 图层6: 头部（大圆+嘴部圆形突出）=====
        // 头主体 - 大圆形
        p.drawEllipse(QPointF(-10*s, -8*s), 7*s, 6.5*s);
        // 嘴部 - 小圆形突出（参考图是圆形嘴）
        p.drawEllipse(QPointF(-16*s, -6*s), 3*s, 2.5*s);
        
        // ===== 图层7: 鬃毛（大波浪+线条纹理）=====
        p.setBrush(maneColor);
        QPainterPath mane;
        mane.moveTo(-3*s, -12*s);
        mane.cubicTo(3*s, -16*s, 9*s, -12*s, 7*s, -6*s);
        mane.cubicTo(5*s, -2*s, 11*s, 0*s, 15*s, -2*s);
        mane.lineTo(13*s, 2*s);
        mane.cubicTo(9*s, 4*s, 3*s, 2*s, 3*s, -2*s);
        mane.cubicTo(3*s, -6*s, 1*s, -10*s, -3*s, -12*s);
        p.drawPath(mane);
        // 鬃毛线条纹理
        p.setPen(QPen(outlineColor.lighter(115), 0.4*s));
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(-1*s, -11*s), QPointF(5*s, -8*s));
        p.drawLine(QPointF(3*s, -10*s), QPointF(7*s, -4*s));
        p.drawLine(QPointF(5*s, -6*s), QPointF(11*s, -1*s));
        
        // ===== 图层8: 耳朵+独角 =====
        p.setPen(QPen(outlineColor, 0.4*s));
        p.setBrush(bodyColor);
        QPainterPath ear;
        ear.moveTo(-6*s, -13*s);
        ear.lineTo(-4*s, -19*s);
        ear.lineTo(-2*s, -13*s);
        ear.closeSubpath();
        p.drawPath(ear);
        // 耳内粉
        p.setBrush(QColor(255, 210, 220, alpha * 0.6));
        p.setPen(Qt::NoPen);
        QPainterPath earIn;
        earIn.moveTo(-5*s, -13.5*s);
        earIn.lineTo(-4*s, -17*s);
        earIn.lineTo(-3*s, -13.5*s);
        earIn.closeSubpath();
        p.drawPath(earIn);
        // 独角
        p.setBrush(hornColor);
        p.setPen(QPen(outlineColor, 0.3*s));
        QPainterPath horn;
        horn.moveTo(-8*s, -14*s);
        horn.lineTo(-6*s, -24*s);
        horn.lineTo(-4*s, -14*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // ===== 图层9: 装饰物（项链）=====
        p.setPen(QPen(outlineColor.lighter(130), 0.6*s));
        p.setBrush(Qt::NoBrush);
        // 项链线
        QPainterPath neckLine;
        neckLine.moveTo(-14*s, -4*s);
        neckLine.quadTo(-10*s, 0*s, -6*s, -2*s);
        p.drawPath(neckLine);
        // 项链珠子
        p.setPen(QPen(outlineColor, 0.3*s));
        QColor beadColors[] = {
            QColor(180, 220, 210, alpha),
            QColor(255, 220, 200, alpha),
            QColor(220, 200, 230, alpha),
            QColor(180, 220, 210, alpha)
        };
        double beadPos[][2] = {{-13, -3}, {-11, -1.5}, {-9, -1}, {-7, -1.5}};
        for (int i = 0; i < 4; ++i) {
            p.setBrush(beadColors[i]);
            p.drawEllipse(QPointF(beadPos[i][0]*s, beadPos[i][1]*s), 1.2*s, 1.2*s);
        }
        
        // ===== 图层10: 身上星星 =====
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 235, 180, alpha * 0.9));
        drawStar(p, -2*s, 0*s, 2.8*s);
        drawStar(p, 4*s, 2*s, 2*s);
        
        // ===== 图层11: 眼睛和嘴巴 =====
        // 眼睛
        p.setBrush(QColor(50, 45, 50, alpha));
        p.drawEllipse(QPointF(-12*s, -8*s), 1.2*s, 1.5*s);
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPointF(-12.5*s, -8.6*s), 0.5*s, 0.5*s);
        // 嘴巴
        p.setPen(QPen(outlineColor, 0.5*s));
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(-18*s, -5.5*s, 2.5*s, 1.5*s), 200*16, 140*16);
        p.setPen(Qt::NoPen);
    }
    
    // ========== 形态2：坐姿款（参考图左下 - 白身体+粉鬃毛+大蝴蝶结铃铛）==========
    void drawUnicornSitting(QPainter &p, double s, int alpha,
                            QColor bodyColor, QColor maneColor, QColor hornColor, QColor bowColor) {
        QColor outlineColor(160, 145, 155, alpha * 0.6);
        
        // ===== 图层1: 尾巴（S卷+线条）=====
        p.setPen(QPen(outlineColor, 0.5*s));
        p.setBrush(maneColor);
        QPainterPath tail;
        tail.moveTo(8*s, 4*s);
        tail.cubicTo(14*s, 0*s, 18*s, 4*s, 20*s, 10*s);
        tail.cubicTo(22*s, 16*s, 18*s, 20*s, 14*s, 18*s);
        tail.cubicTo(10*s, 16*s, 12*s, 10*s, 16*s, 8*s);
        tail.cubicTo(18*s, 6*s, 16*s, 4*s, 14*s, 6*s);
        tail.cubicTo(10*s, 8*s, 8*s, 6*s, 8*s, 4*s);
        p.drawPath(tail);
        // 尾巴线条
        p.setPen(QPen(outlineColor.lighter(120), 0.4*s));
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(12*s, 4*s), QPointF(16*s, 10*s));
        p.drawLine(QPointF(14*s, 8*s), QPointF(16*s, 14*s));
        
        // ===== 图层2: 后臀（坐姿）=====
        p.setPen(QPen(outlineColor, 0.4*s));
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(4*s, 8*s), 7*s, 5*s);
        
        // ===== 图层3: 身体 =====
        p.drawEllipse(QPointF(-2*s, 2*s), 9*s, 7*s);
        
        // ===== 图层4: 前腿+蹄子 =====
        p.drawRoundedRect(QRectF(-12*s, 4*s, 4*s, 10*s), 1.5*s, 1.5*s);
        p.drawRoundedRect(QRectF(-7*s, 5*s, 4*s, 9*s), 1.5*s, 1.5*s);
        p.setBrush(maneColor);
        p.drawEllipse(QPointF(-10*s, 14*s), 2*s, 1.2*s);
        p.drawEllipse(QPointF(-5*s, 14*s), 2*s, 1.2*s);
        
        // ===== 图层5: 头部（大圆+圆形嘴）=====
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(-4*s, -8*s), 7*s, 6.5*s);
        // 嘴部 - 圆形
        p.drawEllipse(QPointF(-10*s, -6*s), 3*s, 2.5*s);
        
        // ===== 图层6: 鬃毛（波浪+线条）=====
        p.setBrush(maneColor);
        QPainterPath mane;
        mane.moveTo(3*s, -12*s);
        mane.cubicTo(9*s, -16*s, 13*s, -10*s, 11*s, -4*s);
        mane.cubicTo(9*s, 0*s, 13*s, 2*s, 17*s, 0*s);
        mane.lineTo(15*s, 4*s);
        mane.cubicTo(11*s, 6*s, 7*s, 4*s, 7*s, 0*s);
        mane.cubicTo(7*s, -4*s, 5*s, -8*s, 3*s, -12*s);
        p.drawPath(mane);
        // 鬃毛线条
        p.setPen(QPen(outlineColor.lighter(115), 0.4*s));
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(5*s, -11*s), QPointF(9*s, -6*s));
        p.drawLine(QPointF(7*s, -8*s), QPointF(11*s, -2*s));
        p.drawLine(QPointF(9*s, -4*s), QPointF(13*s, 1*s));
        
        // ===== 图层7: 耳朵+独角 =====
        p.setPen(QPen(outlineColor, 0.4*s));
        p.setBrush(bodyColor);
        QPainterPath ear;
        ear.moveTo(0*s, -14*s);
        ear.lineTo(2*s, -20*s);
        ear.lineTo(4*s, -14*s);
        ear.closeSubpath();
        p.drawPath(ear);
        p.setBrush(QColor(255, 210, 220, alpha * 0.6));
        p.setPen(Qt::NoPen);
        QPainterPath earIn;
        earIn.moveTo(1*s, -14.5*s);
        earIn.lineTo(2*s, -18*s);
        earIn.lineTo(3*s, -14.5*s);
        earIn.closeSubpath();
        p.drawPath(earIn);
        // 独角
        p.setBrush(hornColor);
        p.setPen(QPen(outlineColor, 0.3*s));
        QPainterPath horn;
        horn.moveTo(-2*s, -14*s);
        horn.lineTo(0*s, -24*s);
        horn.lineTo(2*s, -14*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // ===== 图层8: 大蝴蝶结（详细绘制）=====
        p.setPen(QPen(outlineColor, 0.4*s));
        p.setBrush(bowColor);
        // 左翼
        QPainterPath bowLeft;
        bowLeft.moveTo(-4*s, -1*s);
        bowLeft.cubicTo(-8*s, -7*s, -14*s, -5*s, -12*s, 0*s);
        bowLeft.cubicTo(-10*s, 5*s, -6*s, 3*s, -4*s, -1*s);
        p.drawPath(bowLeft);
        // 左翼线条
        p.setPen(QPen(outlineColor.lighter(120), 0.3*s));
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(-6*s, -3*s), QPointF(-10*s, -1*s));
        // 右翼
        p.setPen(QPen(outlineColor, 0.4*s));
        p.setBrush(bowColor);
        QPainterPath bowRight;
        bowRight.moveTo(-4*s, -1*s);
        bowRight.cubicTo(0*s, -7*s, 6*s, -5*s, 4*s, 0*s);
        bowRight.cubicTo(2*s, 5*s, -2*s, 3*s, -4*s, -1*s);
        p.drawPath(bowRight);
        // 右翼线条
        p.setPen(QPen(outlineColor.lighter(120), 0.3*s));
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(-2*s, -3*s), QPointF(2*s, -1*s));
        // 飘带
        p.setPen(QPen(outlineColor, 0.4*s));
        p.setBrush(bowColor);
        QPainterPath ribbon1;
        ribbon1.moveTo(-6*s, 2*s);
        ribbon1.quadTo(-8*s, 8*s, -5*s, 10*s);
        ribbon1.quadTo(-4*s, 8*s, -5*s, 2*s);
        ribbon1.closeSubpath();
        p.drawPath(ribbon1);
        QPainterPath ribbon2;
        ribbon2.moveTo(-2*s, 2*s);
        ribbon2.quadTo(0*s, 8*s, -3*s, 10*s);
        ribbon2.quadTo(-4*s, 8*s, -3*s, 2*s);
        ribbon2.closeSubpath();
        p.drawPath(ribbon2);
        // 中心圆+铃铛
        p.setBrush(bowColor.darker(105));
        p.drawEllipse(QPointF(-4*s, -1*s), 2*s, 2*s);
        p.setBrush(QColor(255, 225, 120, alpha));
        p.drawEllipse(QPointF(-4*s, -1*s), 1.5*s, 1.5*s);
        // 铃铛高光
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 220, alpha * 0.7));
        p.drawEllipse(QPointF(-4.5*s, -1.5*s), 0.5*s, 0.5*s);
        // 铃铛底部线
        p.setPen(QPen(QColor(200, 180, 100, alpha), 0.3*s));
        p.drawLine(QPointF(-4*s, 0.3*s), QPointF(-4*s, 1*s));
        
        // ===== 图层9: 眼睛和嘴巴 =====
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(50, 45, 50, alpha));
        p.drawEllipse(QPointF(-6*s, -8*s), 1.2*s, 1.5*s);
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPointF(-6.5*s, -8.6*s), 0.5*s, 0.5*s);
        // 嘴巴
        p.setPen(QPen(outlineColor, 0.5*s));
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(-12*s, -5.5*s, 2.5*s, 1.5*s), 200*16, 140*16);
        p.setPen(Qt::NoPen);
    }
    
    // ========== 形态3：飞马款（参考图右下 - 躺在云上+翅膀+彩虹鬃毛）==========
    void drawUnicornPegasus(QPainter &p, double s, int alpha,
                            QColor bodyColor, QColor maneColor, QColor hornColor) {
        QColor outlineColor(160, 145, 155, alpha * 0.5);
        Q_UNUSED(maneColor);
        
        // ===== 图层1: 云朵 =====
        p.setPen(QPen(outlineColor.lighter(130), 0.3*s));
        p.setBrush(QColor(255, 255, 255, alpha * 0.6));
        p.drawEllipse(QPointF(-10*s, 12*s), 10*s, 5*s);
        p.drawEllipse(QPointF(8*s, 10*s), 12*s, 6*s);
        p.drawEllipse(QPointF(-16*s, 14*s), 7*s, 4*s);
        p.drawEllipse(QPointF(18*s, 12*s), 9*s, 5*s);
        p.setBrush(QColor(255, 255, 255, alpha * 0.8));
        p.drawEllipse(QPointF(0*s, 8*s), 16*s, 7*s);
        
        // ===== 图层2: 尾巴（三色渐变+线条）=====
        p.setPen(QPen(outlineColor, 0.4*s));
        // 粉色层
        p.setBrush(QColor(255, 210, 220, alpha));
        QPainterPath tail1;
        tail1.moveTo(14*s, 0*s);
        tail1.cubicTo(20*s, -4*s, 24*s, 2*s, 22*s, 10*s);
        tail1.cubicTo(20*s, 16*s, 16*s, 14*s, 18*s, 8*s);
        tail1.cubicTo(19*s, 4*s, 17*s, 2*s, 14*s, 0*s);
        p.drawPath(tail1);
        // 青色层
        p.setBrush(QColor(180, 220, 210, alpha));
        QPainterPath tail2;
        tail2.moveTo(16*s, 2*s);
        tail2.cubicTo(22*s, 0*s, 26*s, 6*s, 24*s, 14*s);
        tail2.cubicTo(22*s, 20*s, 18*s, 18*s, 20*s, 12*s);
        tail2.cubicTo(21*s, 8*s, 19*s, 4*s, 16*s, 2*s);
        p.drawPath(tail2);
        // 紫色层
        p.setBrush(QColor(210, 200, 230, alpha));
        QPainterPath tail3;
        tail3.moveTo(18*s, 4*s);
        tail3.cubicTo(24*s, 4*s, 26*s, 10*s, 24*s, 18*s);
        tail3.cubicTo(22*s, 24*s, 18*s, 22*s, 20*s, 16*s);
        tail3.cubicTo(21*s, 12*s, 20*s, 6*s, 18*s, 4*s);
        p.drawPath(tail3);
        // 尾巴线条
        p.setPen(QPen(outlineColor.lighter(120), 0.3*s));
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(16*s, 2*s), QPointF(20*s, 8*s));
        p.drawLine(QPointF(18*s, 6*s), QPointF(20*s, 14*s));
        p.drawLine(QPointF(20*s, 8*s), QPointF(22*s, 16*s));
        
        // ===== 图层3: 身体 =====
        p.setPen(QPen(outlineColor, 0.4*s));
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(0, 0), 12*s, 7*s);
        
        // ===== 图层4: 腿（收起）=====
        p.drawEllipse(QPointF(8*s, 6*s), 3*s, 2*s);
        p.drawEllipse(QPointF(-4*s, 6*s), 3*s, 2*s);
        
        // ===== 图层5: 翅膀（详细羽毛）=====
        p.setBrush(QColor(255, 255, 255, alpha * 0.95));
        QPainterPath wing;
        wing.moveTo(-2*s, -4*s);
        wing.cubicTo(2*s, -10*s, 10*s, -14*s, 14*s, -10*s);
        wing.cubicTo(16*s, -8*s, 14*s, -4*s, 10*s, -2*s);
        wing.cubicTo(6*s, 0*s, 2*s, -2*s, -2*s, -4*s);
        p.drawPath(wing);
        // 羽毛线条
        p.setPen(QPen(QColor(220, 215, 235, alpha * 0.6), 0.4*s));
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(0*s, -6*s), QPointF(6*s, -10*s));
        p.drawLine(QPointF(2*s, -4*s), QPointF(8*s, -8*s));
        p.drawLine(QPointF(4*s, -3*s), QPointF(10*s, -6*s));
        p.drawLine(QPointF(6*s, -2*s), QPointF(12*s, -5*s));
        
        // ===== 图层6: 头部（大圆+圆形嘴）=====
        p.setPen(QPen(outlineColor, 0.4*s));
        p.setBrush(bodyColor);
        p.drawEllipse(QPointF(-10*s, -6*s), 7*s, 6*s);
        // 嘴部 - 圆形
        p.drawEllipse(QPointF(-16*s, -4*s), 3*s, 2.5*s);
        
        // ===== 图层7: 鬃毛（三色渐变+线条）=====
        // 粉色层
        p.setBrush(QColor(255, 210, 220, alpha));
        QPainterPath mane1;
        mane1.moveTo(-4*s, -10*s);
        mane1.cubicTo(2*s, -14*s, 6*s, -10*s, 4*s, -6*s);
        mane1.cubicTo(2*s, -4*s, -2*s, -6*s, -4*s, -10*s);
        p.drawPath(mane1);
        // 青色层
        p.setBrush(QColor(180, 220, 210, alpha));
        QPainterPath mane2;
        mane2.moveTo(-2*s, -8*s);
        mane2.cubicTo(4*s, -12*s, 8*s, -8*s, 6*s, -4*s);
        mane2.cubicTo(4*s, -2*s, 0*s, -4*s, -2*s, -8*s);
        p.drawPath(mane2);
        // 紫色层
        p.setBrush(QColor(210, 200, 230, alpha));
        QPainterPath mane3;
        mane3.moveTo(0*s, -6*s);
        mane3.cubicTo(6*s, -10*s, 10*s, -6*s, 8*s, -2*s);
        mane3.cubicTo(6*s, 0*s, 2*s, -2*s, 0*s, -6*s);
        p.drawPath(mane3);
        // 鬃毛线条
        p.setPen(QPen(outlineColor.lighter(115), 0.3*s));
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(-2*s, -10*s), QPointF(2*s, -8*s));
        p.drawLine(QPointF(0*s, -8*s), QPointF(4*s, -5*s));
        p.drawLine(QPointF(2*s, -6*s), QPointF(6*s, -3*s));
        
        // ===== 图层8: 耳朵+独角 =====
        p.setPen(QPen(outlineColor, 0.4*s));
        p.setBrush(bodyColor);
        QPainterPath ear;
        ear.moveTo(-6*s, -12*s);
        ear.lineTo(-4*s, -18*s);
        ear.lineTo(-2*s, -12*s);
        ear.closeSubpath();
        p.drawPath(ear);
        p.setBrush(QColor(255, 210, 220, alpha * 0.6));
        p.setPen(Qt::NoPen);
        QPainterPath earIn;
        earIn.moveTo(-5*s, -12.5*s);
        earIn.lineTo(-4*s, -16*s);
        earIn.lineTo(-3*s, -12.5*s);
        earIn.closeSubpath();
        p.drawPath(earIn);
        // 独角
        p.setBrush(hornColor);
        p.setPen(QPen(outlineColor, 0.3*s));
        QPainterPath horn;
        horn.moveTo(-8*s, -12*s);
        horn.lineTo(-6*s, -22*s);
        horn.lineTo(-4*s, -12*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // ===== 图层9: 项链（珠子）=====
        p.setPen(QPen(outlineColor.lighter(130), 0.5*s));
        p.setBrush(Qt::NoBrush);
        QPainterPath neckLine;
        neckLine.moveTo(-14*s, -2*s);
        neckLine.quadTo(-10*s, 2*s, -6*s, 0*s);
        p.drawPath(neckLine);
        p.setPen(QPen(outlineColor, 0.3*s));
        QColor beadColors[] = {
            QColor(255, 200, 210, alpha),
            QColor(180, 220, 210, alpha),
            QColor(210, 200, 230, alpha)
        };
        double beadPos[][2] = {{-13, -1}, {-10, 1}, {-7, 0.5}};
        for (int i = 0; i < 3; ++i) {
            p.setBrush(beadColors[i]);
            p.drawEllipse(QPointF(beadPos[i][0]*s, beadPos[i][1]*s), 1*s, 1*s);
        }
        
        // ===== 图层10: 眼睛（闭着）=====
        p.setPen(QPen(QColor(60, 55, 60, alpha), 0.8*s));
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(-14*s, -8*s, 4*s, 3*s), 0, 180*16);
        p.setPen(Qt::NoPen);
        
        // ===== 图层11: 身上星星+云朵星星 =====
        p.setBrush(QColor(255, 235, 180, alpha * 0.8));
        drawStar(p, 0*s, -2*s, 2*s);
        p.setBrush(QColor(255, 235, 180, alpha * 0.7));
        drawStar(p, -10*s, 6*s, 2*s);
        drawStar(p, 12*s, 8*s, 1.8*s);
    }
    
    // ========== 形态4：木马款（参考图左上 - 摇摇椅+蝴蝶结铃铛）==========
    void drawUnicornRocking(QPainter &p, double s, int alpha,
                            QColor bodyColor, QColor maneColor, QColor hornColor, QColor baseColor) {
        QColor outlineColor(160, 145, 155, alpha * 0.6);
        
        // ===== 图层1: 底座星星 =====
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 235, 180, alpha * 0.9));
        drawStar(p, -16*s, 18*s, 3*s);
        drawStar(p, 16*s, 18*s, 3*s);
        
        // ===== 图层2: 摇摇椅底座 =====
        p.setPen(QPen(outlineColor, 0.5*s));
        p.setBrush(baseColor);
        QPainterPath base;
        base.moveTo(-18*s, 18*s);
        base.quadTo(0*s, 24*s, 18*s, 18*s);
        base.lineTo(16*s, 21*s);
        base.quadTo(0*s, 26*s, -16*s, 21*s);
        base.closeSubpath();
        p.drawPath(base);
        // 底座卷曲装饰
        QPainterPath baseEnd1;
        baseEnd1.moveTo(-18*s, 18*s);
        baseEnd1.cubicTo(-22*s, 16*s, -24*s, 22*s, -20*s, 24*s);
        baseEnd1.cubicTo(-18*s, 25*s, -16*s, 22*s, -18*s, 18*s);
        p.drawPath(baseEnd1);
        QPainterPath baseEnd2;
        baseEnd2.moveTo(18*s, 18*s);
        baseEnd2.cubicTo(22*s, 16*s, 24*s, 22*s, 20*s, 24*s);
        baseEnd2.cubicTo(18*s, 25*s, 16*s, 22*s, 18*s, 18*s);
        p.drawPath(baseEnd2);
        
        // ===== 图层3: 支撑柱 =====
        p.drawRoundedRect(QRectF(-5*s, 8*s, 2*s, 12*s), 0.5*s, 0.5*s);
        p.drawRoundedRect(QRectF(4*s, 6*s, 2*s, 14*s), 0.5*s, 0.5*s);
        
        // ===== 图层4: 尾巴（大卷+线条）=====
        p.setBrush(maneColor);
        QPainterPath tail;
        tail.moveTo(10*s, -2*s);
        tail.cubicTo(16*s, -6*s, 20*s, 0*s, 18*s, 8*s);
        tail.cubicTo(16*s, 14*s, 12*s, 12*s, 14*s, 6*s);
        tail.cubicTo(16*s, 2*s, 14*s, 0*s, 12*s, 2*s);
        tail.cubicTo(10*s, 4*s, 10*s, 0*s, 10*s, -2*s);
        p.drawPath(tail);
        // 尾巴线条
        p.setPen(QPen(outlineColor.lighter(120), 0.4*s));
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(12*s, 0*s), QPointF(16*s, 6*s));
        p.drawLine(QPointF(14*s, 4*s), QPointF(16*s, 10*s));
        
        // ===== 图层5: 后腿 =====
        p.setPen(QPen(outlineColor, 0.4*s));
        p.setBrush(bodyColor);
        p.drawRoundedRect(QRectF(4*s, 2*s, 3*s, 8*s), 1*s, 1*s);
        
        // ===== 图层6: 身体 =====
        p.drawEllipse(QPointF(0, 0), 9*s, 6*s);
        
        // ===== 图层7: 前腿 =====
        p.drawRoundedRect(QRectF(-7*s, 0*s, 3*s, 10*s), 1*s, 1*s);
        
        // ===== 图层8: 头部（大圆+圆形嘴）=====
        p.drawEllipse(QPointF(-6*s, -10*s), 7*s, 6*s);
        // 嘴部 - 圆形
        p.drawEllipse(QPointF(-12*s, -8*s), 3*s, 2.5*s);
        
        // ===== 图层9: 鬃毛（大波浪+线条）=====
        p.setBrush(maneColor);
        QPainterPath mane;
        mane.moveTo(1*s, -14*s);
        mane.cubicTo(7*s, -18*s, 11*s, -12*s, 9*s, -6*s);
        mane.cubicTo(7*s, -2*s, 11*s, 0*s, 15*s, -2*s);
        mane.lineTo(13*s, 2*s);
        mane.cubicTo(9*s, 4*s, 5*s, 2*s, 5*s, -2*s);
        mane.cubicTo(5*s, -6*s, 3*s, -10*s, 1*s, -14*s);
        p.drawPath(mane);
        // 鬃毛线条
        p.setPen(QPen(outlineColor.lighter(115), 0.4*s));
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(3*s, -13*s), QPointF(7*s, -8*s));
        p.drawLine(QPointF(5*s, -10*s), QPointF(9*s, -4*s));
        p.drawLine(QPointF(7*s, -6*s), QPointF(11*s, -1*s));
        
        // ===== 图层10: 耳朵+独角 =====
        p.setPen(QPen(outlineColor, 0.4*s));
        p.setBrush(bodyColor);
        QPainterPath ear;
        ear.moveTo(-2*s, -16*s);
        ear.lineTo(0*s, -22*s);
        ear.lineTo(2*s, -16*s);
        ear.closeSubpath();
        p.drawPath(ear);
        p.setBrush(QColor(255, 210, 220, alpha * 0.6));
        p.setPen(Qt::NoPen);
        QPainterPath earIn;
        earIn.moveTo(-1*s, -16.5*s);
        earIn.lineTo(0*s, -20*s);
        earIn.lineTo(1*s, -16.5*s);
        earIn.closeSubpath();
        p.drawPath(earIn);
        // 独角
        p.setBrush(hornColor);
        p.setPen(QPen(outlineColor, 0.3*s));
        QPainterPath horn;
        horn.moveTo(-4*s, -16*s);
        horn.lineTo(-2*s, -26*s);
        horn.lineTo(0*s, -16*s);
        horn.closeSubpath();
        p.drawPath(horn);
        
        // ===== 图层11: 蝴蝶结（详细）=====
        p.setPen(QPen(outlineColor, 0.4*s));
        p.setBrush(baseColor);
        // 左翼
        QPainterPath bowLeft;
        bowLeft.moveTo(-4*s, -4*s);
        bowLeft.cubicTo(-8*s, -10*s, -14*s, -6*s, -10*s, -2*s);
        bowLeft.cubicTo(-8*s, 2*s, -6*s, 0*s, -4*s, -4*s);
        p.drawPath(bowLeft);
        // 左翼线条
        p.setPen(QPen(outlineColor.lighter(120), 0.3*s));
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(-6*s, -6*s), QPointF(-10*s, -4*s));
        // 右翼
        p.setPen(QPen(outlineColor, 0.4*s));
        p.setBrush(baseColor);
        QPainterPath bowRight;
        bowRight.moveTo(-4*s, -4*s);
        bowRight.cubicTo(0*s, -10*s, 6*s, -6*s, 2*s, -2*s);
        bowRight.cubicTo(0*s, 2*s, -2*s, 0*s, -4*s, -4*s);
        p.drawPath(bowRight);
        // 右翼线条
        p.setPen(QPen(outlineColor.lighter(120), 0.3*s));
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(-2*s, -6*s), QPointF(2*s, -4*s));
        // 中心+铃铛
        p.setPen(QPen(outlineColor, 0.3*s));
        p.setBrush(baseColor.darker(105));
        p.drawEllipse(QPointF(-4*s, -4*s), 2*s, 2*s);
        p.setBrush(QColor(255, 225, 120, alpha));
        p.drawEllipse(QPointF(-4*s, -4*s), 1.5*s, 1.5*s);
        // 铃铛高光
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 220, alpha * 0.7));
        p.drawEllipse(QPointF(-4.5*s, -4.5*s), 0.5*s, 0.5*s);
        // 铃铛底部线
        p.setPen(QPen(QColor(200, 180, 100, alpha), 0.3*s));
        p.drawLine(QPointF(-4*s, -2.7*s), QPointF(-4*s, -1.5*s));
        
        // ===== 图层12: 眼睛和嘴巴 =====
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(50, 45, 50, alpha));
        p.drawEllipse(QPointF(-8*s, -10*s), 1.2*s, 1.5*s);
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPointF(-8.5*s, -10.6*s), 0.5*s, 0.5*s);
        // 嘴巴
        p.setPen(QPen(outlineColor, 0.5*s));
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(-14*s, -7.5*s, 2.5*s, 1.5*s), 200*16, 140*16);
        p.setPen(Qt::NoPen);
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
    
    // 绘制彩虹
    void drawRainbow(QPainter &p, int x, int y, int size, int alpha) {
        p.save();
        p.translate(x, y);
        p.setPen(Qt::NoPen);
        
        // 彩虹颜色从外到内
        QColor colors[] = {
            QColor(255, 120, 120, alpha),  // 红
            QColor(255, 200, 120, alpha),  // 橙
            QColor(255, 255, 150, alpha),  // 黄
            QColor(150, 255, 150, alpha),  // 绿
            QColor(150, 200, 255, alpha),  // 蓝
            QColor(200, 150, 255, alpha)   // 紫
        };
        
        double baseR = size;
        for (int i = 0; i < 6; ++i) {
            p.setBrush(colors[i]);
            double r = baseR - i * (size / 8.0);
            QPainterPath arc;
            arc.moveTo(-r, 0);
            arc.arcTo(-r, -r, r * 2, r * 2, 180, -180);
            arc.lineTo(r - size / 8.0, 0);
            double r2 = r - size / 8.0;
            arc.arcTo(-r2, -r2, r2 * 2, r2 * 2, 0, 180);
            arc.closeSubpath();
            p.drawPath(arc);
        }
        
        // 彩虹两端的云朵
        p.setBrush(QColor(255, 255, 255, alpha * 0.8));
        p.drawEllipse(QPointF(-baseR, 0), size * 0.3, size * 0.2);
        p.drawEllipse(QPointF(-baseR + size * 0.15, -size * 0.1), size * 0.25, size * 0.18);
        p.drawEllipse(QPointF(baseR, 0), size * 0.3, size * 0.2);
        p.drawEllipse(QPointF(baseR - size * 0.15, -size * 0.1), size * 0.25, size * 0.18);
        
        p.restore();
    }
    
    // 绘制气球
    void drawBalloon(QPainter &p, int x, int y, int size, int alpha, QColor color) {
        p.save();
        p.translate(x, y);
        p.setPen(Qt::NoPen);
        
        // 气球线
        p.setPen(QPen(QColor(180, 180, 180, alpha * 0.6), 1));
        QPainterPath string;
        string.moveTo(0, size * 0.5);
        string.cubicTo(size * 0.1, size * 0.8, -size * 0.1, size * 1.0, 0, size * 1.2);
        p.drawPath(string);
        p.setPen(Qt::NoPen);
        
        // 气球
        color.setAlpha(alpha);
        p.setBrush(color);
        p.drawEllipse(QPointF(0, 0), size * 0.4, size * 0.5);
        
        // 气球底部小三角
        QPainterPath knot;
        knot.moveTo(-size * 0.08, size * 0.45);
        knot.lineTo(0, size * 0.55);
        knot.lineTo(size * 0.08, size * 0.45);
        knot.closeSubpath();
        p.drawPath(knot);
        
        // 高光
        p.setBrush(QColor(255, 255, 255, alpha * 0.4));
        p.drawEllipse(QPointF(-size * 0.12, -size * 0.15), size * 0.1, size * 0.15);
        
        p.restore();
    }
    
    // 绘制糖果
    void drawCandy(QPainter &p, int x, int y, int size, int alpha) {
        p.save();
        p.translate(x, y);
        p.rotate(qrand() % 60 - 30);
        p.setPen(Qt::NoPen);
        
        // 糖果主体
        p.setBrush(QColor(255, 200, 210, alpha));
        p.drawRoundedRect(QRectF(-size * 0.3, -size * 0.15, size * 0.6, size * 0.3), size * 0.1, size * 0.1);
        
        // 条纹
        p.setBrush(QColor(255, 150, 170, alpha));
        for (int i = 0; i < 3; ++i) {
            p.drawRect(QRectF(-size * 0.2 + i * size * 0.2, -size * 0.15, size * 0.08, size * 0.3));
        }
        
        // 两端包装纸
        p.setBrush(QColor(255, 220, 230, alpha));
        QPainterPath wrap1, wrap2;
        wrap1.moveTo(-size * 0.3, -size * 0.1);
        wrap1.lineTo(-size * 0.5, -size * 0.2);
        wrap1.lineTo(-size * 0.5, size * 0.2);
        wrap1.lineTo(-size * 0.3, size * 0.1);
        wrap1.closeSubpath();
        wrap2.moveTo(size * 0.3, -size * 0.1);
        wrap2.lineTo(size * 0.5, -size * 0.2);
        wrap2.lineTo(size * 0.5, size * 0.2);
        wrap2.lineTo(size * 0.3, size * 0.1);
        wrap2.closeSubpath();
        p.drawPath(wrap1);
        p.drawPath(wrap2);
        
        p.restore();
    }
    
    // 绘制爱心
    void drawHeart(QPainter &p, double x, double y, double s, int alpha, QColor color) {
        p.save();
        p.translate(x, y);
        color.setAlpha(alpha);
        p.setBrush(color);
        p.setPen(Qt::NoPen);
        
        QPainterPath heartPath;
        heartPath.moveTo(0, s * 0.4);
        heartPath.cubicTo(-s * 0.8, -s * 0.25, -s * 0.4, -s * 0.8, 0, -s * 0.4);
        heartPath.cubicTo(s * 0.4, -s * 0.8, s * 0.8, -s * 0.25, 0, s * 0.4);
        p.drawPath(heartPath);
        
        p.restore();
    }
    
    // 绘制棒棒糖
    void drawLollipop(QPainter &p, int x, int y, int size, int alpha) {
        p.save();
        p.translate(x, y);
        p.rotate(qrand() % 40 - 20);
        p.setPen(Qt::NoPen);
        
        // 棒子
        p.setBrush(QColor(200, 180, 160, alpha));
        p.drawRoundedRect(QRectF(-size * 0.04, size * 0.25, size * 0.08, size * 0.5), 2, 2);
        
        // 糖果圆形
        p.setBrush(QColor(255, 200, 220, alpha));
        p.drawEllipse(QPointF(0, 0), size * 0.3, size * 0.3);
        
        // 螺旋纹
        p.setPen(QPen(QColor(255, 150, 180, alpha), size * 0.05));
        for (int i = 0; i < 3; ++i) {
            double r = size * 0.1 * (i + 1);
            p.drawArc(QRectF(-r, -r, r * 2, r * 2), 45 * 16, 180 * 16);
        }
        p.setPen(Qt::NoPen);
        
        p.restore();
    }
    
    // 检查位置是否与已有位置重叠
    bool isOverlapping(int x, int y, int size, const QVector<QRect> &occupied) {
        QRect newRect(x - size, y - size, size * 2, size * 2);
        for (const QRect &rect : occupied) {
            if (newRect.intersects(rect.adjusted(-20, -20, 20, 20))) {
                return true;
            }
        }
        return false;
    }
    
    // 绘制项链装饰（参考图左上角）
    void drawNecklace(QPainter &p, int x, int y, int size, int alpha) {
        p.save();
        p.translate(x, y);
        
        QColor outlineColor(180, 170, 175, alpha * 0.5);
        p.setPen(QPen(outlineColor, 0.8));
        p.setBrush(Qt::NoBrush);
        
        // 项链弧线
        QPainterPath chain;
        chain.moveTo(0, 0);
        chain.cubicTo(size * 0.3, size * 0.5, size * 0.3, size * 1.2, 0, size * 1.5);
        p.drawPath(chain);
        p.setPen(Qt::NoPen);
        
        // 彩色珠子
        QColor beadColors[] = {
            QColor(180, 220, 210, alpha),
            QColor(255, 220, 200, alpha),
            QColor(220, 200, 230, alpha)
        };
        for (int i = 0; i < 5; ++i) {
            double t = i / 4.0;
            double bx = size * 0.3 * qSin(t * M_PI);
            double by = t * size * 1.5;
            p.setBrush(beadColors[i % 3]);
            p.drawEllipse(QPointF(bx, by), size * 0.12, size * 0.12);
        }
        
        // 星星吊坠
        p.setBrush(QColor(255, 235, 180, alpha));
        drawStar(p, 0, size * 1.6, size * 0.25);
        
        p.restore();
    }
    
    // 绘制蝴蝶结装饰（参考图中间）
    void drawBowDecor(QPainter &p, int x, int y, int size, int alpha) {
        p.save();
        p.translate(x, y);
        p.setPen(Qt::NoPen);
        
        QColor bowColor(210, 190, 220, alpha);
        p.setBrush(bowColor);
        
        // 左翼
        QPainterPath left;
        left.moveTo(0, 0);
        left.cubicTo(-size * 0.6, -size * 0.4, -size * 0.6, size * 0.4, 0, 0);
        p.drawPath(left);
        // 右翼
        QPainterPath right;
        right.moveTo(0, 0);
        right.cubicTo(size * 0.6, -size * 0.4, size * 0.6, size * 0.4, 0, 0);
        p.drawPath(right);
        // 中心铃铛
        p.setBrush(QColor(255, 225, 120, alpha));
        p.drawEllipse(QPointF(0, 0), size * 0.15, size * 0.15);
        
        p.restore();
    }
    
    // 绘制多个独角兽和装饰
    void drawUnicorns(QPainter &p, int w, int h, QColor baseColor) {
        Q_UNUSED(baseColor);
        
        // 根据面积计算合适的数量
        int area = w * h;
        int unicornCount = qMax(3, qMin(12, area / 50000));
        int starCount = qMax(4, qMin(15, area / 35000));
        int decorCount = qMax(2, qMin(6, area / 80000));
        
        QVector<QRect> occupied;
        
        // 绘制独角兽（防重叠）
        int attempts = 0;
        int placed = 0;
        while (placed < unicornCount && attempts < unicornCount * 10) {
            int x = 60 + qrand() % (w - 120);
            int y = 60 + qrand() % (h - 120);
            double depth = (qrand() % 100) / 100.0;
            int size = 45 + static_cast<int>(depth * 30); // 45-75
            
            if (!isOverlapping(x, y, size, occupied)) {
                int alpha = 70 + static_cast<int>(depth * 50); // 70-120
                
                p.save();
                if (qrand() % 2 == 0) {
                    p.translate(x, y);
                    p.scale(-1, 1);
                    p.translate(-x, -y);
                }
                
                drawUnicorn(p, x, y, size, alpha);
                p.restore();
                
                occupied.append(QRect(x - size, y - size, size * 2, size * 2));
                placed++;
            }
            attempts++;
        }
        
        // 绘制装饰物（项链、蝴蝶结）
        p.setPen(Qt::NoPen);
        for (int i = 0; i < decorCount; ++i) {
            int x = 30 + qrand() % (w - 60);
            int y = 30 + qrand() % (h - 60);
            int alpha = 60 + qrand() % 40;
            int size = 20 + qrand() % 15;
            
            if (!isOverlapping(x, y, size, occupied)) {
                if (i % 2 == 0) {
                    drawNecklace(p, x, y, size, alpha);
                } else {
                    drawBowDecor(p, x, y, size, alpha);
                }
                occupied.append(QRect(x - size, y - size, size * 2, size * 2));
            }
        }
        
        // 绘制小星星点缀（散落在空白处）
        for (int i = 0; i < starCount; ++i) {
            int x = qrand() % w;
            int y = qrand() % h;
            int alpha = 50 + qrand() % 50;
            p.setBrush(QColor(255, 235, 180, alpha));
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
            // 子君白 - 爱心（根据面积调整数量）
            int area = w * h;
            int heartCount = qMax(8, qMin(35, area / 20000));
            int starCount = qMax(5, qMin(20, area / 30000));
            
            for (int i = 0; i < heartCount; ++i) {
                int x = qrand() % w;
                int y = qrand() % h;
                double depth = (qrand() % 100) / 100.0;
                int s = 10 + static_cast<int>(depth * 20); // 10-30
                int alpha = 35 + static_cast<int>(depth * 55);
                
                QColor c = patternColor;
                c.setAlpha(alpha);
                p.setBrush(c);
                p.setPen(Qt::NoPen);
                
                p.save();
                p.translate(x, y);
                p.rotate((qrand() % 40) - 20);
                
                // 绘制爱心
                QPainterPath heartPath;
                heartPath.moveTo(0, s/2.0);
                heartPath.cubicTo(-s, -s/3.0, -s/2.0, -s, 0, -s/2.0);
                heartPath.cubicTo(s/2.0, -s, s, -s/3.0, 0, s/2.0);
                p.drawPath(heartPath);
                
                p.restore();
            }
            
            // 添加小星星点缀
            for (int i = 0; i < starCount; ++i) {
                int x = qrand() % w;
                int y = qrand() % h;
                int alpha = 35 + qrand() % 45;
                p.setBrush(QColor(230, 199, 192, alpha));
                drawStar(p, x, y, 3 + qrand() % 4);
            }
        }
    }
};
