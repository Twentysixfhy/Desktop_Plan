#ifndef ECGADC_H
#define ECGADC_H

#include "ecg/ecgdatasource.h"
#include <QTimer>
#include <QFile>

/// 树莓派 ADC 数据源（SPI）
/// 读取外接 ADC（如 MCP3008）的原始值
/// 需要在树莓派上编译（PLATFORM_RASPBERRY_PI）
class EcgAdc : public EcgDataSource {
    Q_OBJECT
public:
    explicit EcgAdc(int spiChannel = 0, int sampleRate = 250, QObject *parent = nullptr);

    void start() override;
    void stop() override;
    bool isRunning() const override;
    int  sampleRate() const override { return m_sampleRate; }

private slots:
    void readSample();

private:
    bool openSpiDevice();

    QTimer *m_timer = nullptr;
    int m_spiChannel;
    int m_sampleRate;
    bool m_running = false;

    // SPI 设备相关
    int m_spiFd = -1;
    QString m_spiDevice;
};

#endif // ECGADC_H
