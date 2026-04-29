#ifndef ECG_WIDGET_H
#define ECG_WIDGET_H

#include <QWidget>
#include <QVector>
#include <QPushButton>
#include <QLabel>
#include <QElapsedTimer>
#include "ecg/ecgdatasource.h"

/// 心电波形显示小部件
/// 实时滚动显示 ECG 数据流，支持开始/暂停
class EcgWidget : public QWidget {
    Q_OBJECT
public:
    explicit EcgWidget(QWidget *parent = nullptr);
    ~EcgWidget() override;

    /// 绑定数据源（模拟器或 ADC）
    void setDataSource(EcgDataSource *source);

    /// 启动 ECG 显示
    void start();

    /// 停止 ECG 显示
    void stop();

    bool isRunning() const { return m_running; }

signals:
    void runningChanged(bool running);

public slots:
    void toggleRunning();
    void clearScreen();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onSampleReceived(double value);

private:
    void drawGrid(QPainter &p, const QRect &rect);
    void drawWaveform(QPainter &p, const QRect &rect);

    EcgDataSource *m_source = nullptr;
    bool m_running = false;

    // 环形缓冲区
    static constexpr int MAX_SAMPLES = 2000;
    QVector<double> m_buffer;
    int m_head = 0;
    int m_count = 0;

    // 显示参数
    double m_minValue = -1.5;
    double m_maxValue = 1.5;
    int m_samplesPerScreen = 500; // 屏幕可见采样点数

    // UI
    QPushButton *m_toggleBtn = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_bpmLabel = nullptr;

    // BPM 计算
    int m_bpm = 0;
    QElapsedTimer m_bpmTimer;
    int m_rPeakCount = 0;
    double m_lastRPeakTime = 0;
    double m_peakThreshold = 0.6;
};

#endif // ECG_WIDGET_H
