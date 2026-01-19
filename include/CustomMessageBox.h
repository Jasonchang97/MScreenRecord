#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>

class CustomMessageBox : public QDialog {
    Q_OBJECT
    Q_PROPERTY(qreal popupScale READ popupScale WRITE setPopupScale)

public:
    enum IconType { Info, Warning, Question, Error, Success };
    
    explicit CustomMessageBox(QWidget *parent, const QString &title, const QString &text, IconType icon = Info);
    
    static int question(QWidget *parent, const QString &title, const QString &text);
    static void information(QWidget *parent, const QString &title, const QString &text);
    static void warning(QWidget *parent, const QString &title, const QString &text);

    qreal popupScale() const { return m_popupScale; }
    void setPopupScale(qreal scale);

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void setupUi(const QString &title, const QString &text, IconType icon);
    void loadThemeColors();
    void drawIcon(QPainter &painter, const QRect &rect, IconType type);
    
    // Theme colors
    QColor m_bgColor;
    QColor m_textColor;
    QColor m_borderColor;
    QColor m_btnBgColor;
    QColor m_btnHoverColor;
    QColor m_accentColor;
    QColor m_shadowColor;
    
    // UI Elements
    QWidget *m_titleBar;
    QLabel *m_iconLabel;
    IconType m_iconType;
    QPoint m_dragPosition;
    bool m_isDragging;
    qreal m_popupScale;
    QPropertyAnimation *m_animation;
};
