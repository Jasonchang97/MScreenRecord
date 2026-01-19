#pragma once

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QSlider>
#include <QListWidget>
#include <QCloseEvent>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QTimeEdit>
#include <QVBoxLayout>
#include <QStackedLayout> 
#include <QSettings>
#include <QMouseEvent>
#include <QProcess>
#include <QTimer> 
#include <QPainter> 

#include "RecorderController.h"
#include "HistoryManager.h"
#include "SelectionOverlay.h"
#include "CountdownOverlay.h"
#include "VideoUtils.h"
#include "FloatingBall.h"
#include "RangeSlider.h" 
#include "NativePlayerWidget.h"
#include "GlobalHotkey.h"

#include <QProgressBar> 

// Custom container to enforce 16:9 aspect ratio
class VideoContainer : public QWidget {
    Q_OBJECT
public:
    explicit VideoContainer(QWidget *parent = nullptr) : QWidget(parent) {
        QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        policy.setHeightForWidth(true);
        setSizePolicy(policy);
        setStyleSheet("background-color: black;");
    }
    // Enforce 16:9 Height based on Width
    int heightForWidth(int width) const override {
        return width * 9 / 16;
    }
    bool hasHeightForWidth() const override { return true; }
    QSize sizeHint() const override {
        return QSize(720, 405); // 16:9 default
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void changeEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override; 

private slots:
    void onSelectAreaClicked();
    void onFullScreenClicked();
    void onStartStopClicked();
    void onAreaSelected(const QRect &rect);
    void onSelectionCancelled();
    void saveAndAddToHistory(const QString &path); // Renamed from onRecordingFinished
    void onRecorderStateChanged(RecorderController::State state);
    void onTrimClicked();
    void onOpenFileClicked(); // Open external video for preview/trim
    
    // Playback Slots
    void onPlayPauseClicked();
    
    void onHistoryItemDoubleClicked(QListWidgetItem *item); // Kept for list compatibility
    void onDeleteHistoryClicked();
    void refreshHistoryList();
    void logMessage(const QString &msg);
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    
    void onSettingsClicked();
    void onMinimizeClicked();
    void onMaximizeClicked(); 
    void onCloseClicked();
    void updateAudioLevels(double sys, double mic);
    void onHotkeyTriggered(int id);
    void registerHotkeys(); 

public slots:
    void loadVideo(const QString &path, qint64 durationMs = 0);

private slots:
    // Player Slots
    void onPlayerFinished();
    void onPlayerPositionChanged(qint64 ms);
    void onSeekSliderPressed();
    void onSeekSliderReleased();
    void onSeekSliderMoved(int val);

private:
    void setupUi();
    void setupStyle(); 
    void applyTheme(QString themeName); 
    QColor getThemeIconColor() const;
    void setupTrayIcon();
    void setupFloatingBall(); 
    QString formatTime(qint64 ms) const; 
    void loadSettings();
    void saveSettings();
    void startRecordingInternal(); // Internal function to start recording after countdown
    void updateMaximizeButton(); 
    
    // Helpers
    void updateTimeLabel(qint64 current, qint64 total);

    QWidget *m_titleBar;
    QPoint m_dragPosition;
    bool m_isDragging;

    QPushButton *m_btnStartStop;
    QPushButton *m_btnOpenFile;
    QPushButton *m_btnSettings; 
    QLabel *m_lblAreaStatus; // Kept but maybe hidden or used for logic
    
    QCheckBox *m_chkSysAudio;
    QSlider *m_sliderSysVol;
    QLabel *m_lblSysVolValue; 
    QCheckBox *m_chkMicAudio;
    QSlider *m_sliderMicVol;
    QLabel *m_lblMicVolValue; 
    // QProgressBar *m_levelSys; // Removed to match static image style if needed, or kept hidden
    // QProgressBar *m_levelMic; 
    
    // Video Area
    VideoContainer *m_videoContainer; 
    QVBoxLayout *m_videoLayout; 
    NativePlayerWidget *m_player; 
    QLabel *m_previewLabel; 
    
    RangeSlider *m_rangeSlider; // Re-added
    QPushButton *m_btnPlayPause;
    QLabel *m_lblTimeDisplay; 
    QLabel *m_lblTrimInfo; // Added

    QTimeEdit *m_editTrimStart;
    QTimeEdit *m_editTrimEnd;
    QPushButton *m_btnTrim;
    
    QListWidget *m_listHistory;
    QPushButton *m_btnDeleteHistory;
    
    QPushButton *m_btnMin;
    QPushButton *m_btnMax; 
    QPushButton *m_btnClose;

    QTextEdit *m_logConsole;
    
    QWidget *m_themeOverlay; // Theme pattern widget

    RecorderController *m_recorder;
    HistoryManager *m_historyMgr;
    VideoUtils *m_videoUtils;
    SelectionOverlay *m_overlay;
    CountdownOverlay *m_countdownOverlay;
    bool m_isSeeking;
    bool m_pendingRecording; 
    
    QTimer *m_previewTimer;
    QTimer *m_recTimer; 

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    FloatingBall *m_floatingBall;
    QSettings *m_settings;

    QRect m_currentSelection;
    QString m_lastRecordedFile;
    qint64 m_currentDuration; 
    qint64 m_currentPosition; 
    qint64 m_totalDuration;
    
    bool m_isPlaying;
};
