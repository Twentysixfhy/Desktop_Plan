#include "ecgsimulator.h"
#include <QtMath>
#include <QRandomGenerator>

EcgSimulator::EcgSimulator(int sampleRate, QObject *parent)
    : EcgDataSource(parent)
    , m_sampleRate(sampleRate)
    , m_index(0)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &EcgSimulator::emitNextSample);
    generateEcgTemplate();
}

void EcgSimulator::start() {
    if (m_running) return;
    m_index = 0;
    m_running = true;
    int intervalMs = 1000 / m_sampleRate;
    m_timer->start(intervalMs);
}

void EcgSimulator::stop() {
    m_running = false;
    m_timer->stop();
}

bool EcgSimulator::isRunning() const {
    return m_running;
}

void EcgSimulator::emitNextSample() {
    if (m_template.isEmpty()) return;
    double val = m_template[m_index];

    // 加一点随机噪声，更真实
    val += QRandomGenerator::global()->bounded(-0.02, 0.02);

    emit sampleReady(val);
    m_index = (m_index + 1) % m_template.size();
}

/// 生成一个近似 ECG 心跳周期的模板（约 1 秒，250 采样点）
/// 包含 P 波、QRS 波群、T 波
void EcgSimulator::generateEcgTemplate() {
    const int N = m_sampleRate;          // 1 秒 = sampleRate 个采样点
    m_template.resize(N);

    for (int i = 0; i < N; ++i) {
        double t = static_cast<double>(i) / m_sampleRate; // 时间 (秒)
        double v = 0.0;

        // P 波 — 小正向波 (0.1-0.2s)
        if (t >= 0.10 && t <= 0.22) {
            double x = (t - 0.16) / 0.04;  // 归一化
            v += 0.15 * qExp(-x * x * 4);
        }

        // Q 波 — 小负向波 (0.24-0.27s)
        if (t >= 0.24 && t <= 0.27) {
            double x = (t - 0.255) / 0.015;
            v -= 0.10 * qExp(-x * x * 4);
        }

        // R 波 — 高大正向波 (0.27-0.31s)
        if (t >= 0.27 && t <= 0.31) {
            double x = (t - 0.285) / 0.008;
            v += 1.0 * qExp(-x * x * 4);
        }

        // S 波 — 负向波 (0.31-0.35s)
        if (t >= 0.31 && t <= 0.35) {
            double x = (t - 0.330) / 0.012;
            v -= 0.30 * qExp(-x * x * 4);
        }

        // T 波 — 较宽正向波 (0.36-0.55s)
        if (t >= 0.36 && t <= 0.55) {
            double x = (t - 0.45) / 0.06;
            v += 0.25 * qExp(-x * x * 4);
        }

        // 归一化到 -1 ~ 1 范围
        m_template[i] = v;
    }
}
