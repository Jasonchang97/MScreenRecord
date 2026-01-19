#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>

class CustomMessageBox : public QDialog {
    Q_OBJECT

public:
    enum IconType { Info, Warning, Question, Error };
    
    explicit CustomMessageBox(QWidget *parent, const QString &title, const QString &text, IconType icon = Info);
    
    static int question(QWidget *parent, const QString &title, const QString &text);
    static void information(QWidget *parent, const QString &title, const QString &text);

private:
    void setupUi(const QString &title, const QString &text, IconType icon);
    
    // UI Elements
    QWidget *m_titleBar;
    QPoint m_dragPosition;
    bool m_isDragging;
    
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};
