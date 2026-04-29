#ifndef ECGSIMULATOR_H
#define ECGSIMULATOR_H

#include "ecg/ecgdatasource.h"
#include <QTimer>
#include <QVector>

/// 模拟 ECG 信号发生器
/// 在 Windows 开发阶段使用，生成仿真心电波形
class EcgSimulator : public EcgDataSource {
    Q_OBJECT
public:
    explicit EcgSimulator(int sampleRate = 250, QObject *parent = nullptr);

    void start() override;
    void stop() override;
    bool isRunning() const override;
    int  sampleRate() const override { return m_sampleRate; }

private slots:
    void emitNextSample();

private:
    void generateEcgTemplate();

    QTimer *m_timer = nullptr;
    int m_sampleRate;
    int m_index = 0;
    QVector<double> m_template;   // 一个完整心跳周期的样本
    bool m_running = false;
};

#endif // ECGSIMULATOR_H
