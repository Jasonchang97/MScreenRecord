#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPoint>
#include <QKeySequence>

class HotkeyEdit;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    
    static QString getSavePath();
    static bool getMinimizeToTray();
    static QString getTheme();
    
    // 获取快捷键设置
    static QKeySequence getShowWindowHotkey();
    static QKeySequence getStartRecordHotkey();

signals:
    void hotkeyChanged();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onBrowseClicked();
    void onSaveClicked();
    void onShowWindowHotkeyChanged(const QKeySequence &seq);
    void onStartRecordHotkeyChanged(const QKeySequence &seq);

private:
    void loadSettings();
    void saveSettings();
    bool checkHotkeyConflict(const QKeySequence &seq, HotkeyEdit *sourceEdit);

    QWidget *m_titleBar;
    QPoint m_dragPosition;
    bool m_isDragging;

    QLineEdit *m_editPath;
    QSpinBox *m_spinFps;
    QComboBox *m_comboBitrate;
    QCheckBox *m_chkMinimizeToTray;
    QCheckBox *m_chkCountdown;
    QSpinBox *m_spinCountdownSecs;
    QComboBox *m_comboTheme;
    QWidget *m_themeOverlay;
    
    // 快捷键
    HotkeyEdit *m_hotkeyShowWindow;
    HotkeyEdit *m_hotkeyStartRecord;
};
