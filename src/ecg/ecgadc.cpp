#include "ecgadc.h"
#include <QDebug>

#ifdef PLATFORM_RASPBERRY_PI
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#endif

EcgAdc::EcgAdc(int spiChannel, int sampleRate, QObject *parent)
    : EcgDataSource(parent)
    , m_spiChannel(spiChannel)
    , m_sampleRate(sampleRate)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &EcgAdc::readSample);
    m_spiDevice = QString("/dev/spidev0.%1").arg(spiChannel);
}

void EcgAdc::start() {
    if (m_running) return;

#ifndef PLATFORM_RASPBERRY_PI
    qWarning() << "EcgAdc: not on Raspberry Pi — cannot open SPI. Use EcgSimulator instead.";
    emit errorOccurred(QStringLiteral("SPI 仅在树莓派上可用，当前平台不支持"));
    return;
#endif

    if (!openSpiDevice()) {
        emit errorOccurred(QStringLiteral("无法打开 SPI 设备: ") + m_spiDevice);
        return;
    }

    m_running = true;
    int intervalMs = 1000 / m_sampleRate;
    m_timer->start(intervalMs);
    qInfo() << "ECG ADC started on" << m_spiDevice;
}

void EcgAdc::stop() {
    m_running = false;
    m_timer->stop();

#ifdef PLATFORM_RASPBERRY_PI
    if (m_spiFd >= 0) {
        ::close(m_spiFd);
        m_spiFd = -1;
    }
#endif
}

bool EcgAdc::isRunning() const {
    return m_running;
}

void EcgAdc::readSample() {
#ifdef PLATFORM_RASPBERRY_PI
    if (m_spiFd < 0) return;

    // MCP3008 单端读取：发送 3 字节，返回 2 字节有效数据 + 1 占位
    // 起始位(1) + 模式(1=单端) + 通道号(3) + ××××
    unsigned char tx[3] = {0x01, static_cast<unsigned char>(0x80 | (m_spiChannel << 4)), 0x00};
    unsigned char rx[3] = {0, 0, 0};

    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));
    tr.tx_buf = (unsigned long)tx;
    tr.rx_buf = (unsigned long)rx;
    tr.len = 3;
    tr.speed_hz = 1000000;
    tr.delay_usecs = 0;
    tr.bits_per_word = 8;

    if (ioctl(m_spiFd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        qWarning() << "SPI read error";
        return;
    }

    // MCP3008 返回值：rx[1] 低 4 位 + rx[2] 高 8 位 = 10-bit
    int value = ((rx[1] & 0x0F) << 8) | rx[2];

    // 归一化到 mV（假设 Vref = 3.3V, ADS8232 增益 ~1000）
    double voltage = (value / 1023.0) * 3300.0;
    double ecgMv  = voltage / 1000.0; // 除以 ADS8232 增益

    emit sampleReady(ecgMv);
#else
    // 非树莓派平台不应该走到这里
    emit errorOccurred("SPI not available on this platform");
#endif
}

bool EcgAdc::openSpiDevice() {
#ifdef PLATFORM_RASPBERRY_PI
    m_spiFd = ::open(m_spiDevice.toLocal8Bit().constData(), O_RDWR);
    if (m_spiFd < 0) {
        qWarning() << "Failed to open SPI device:" << m_spiDevice;
        return false;
    }

    // 配置 SPI
    unsigned char mode  = SPI_MODE_0;
    unsigned char bits  = 8;
    unsigned int  speed = 1000000;

    ioctl(m_spiFd, SPI_IOC_WR_MODE, &mode);
    ioctl(m_spiFd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(m_spiFd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    qInfo() << "SPI device opened:" << m_spiDevice;
    return true;
#else
    return false;
#endif
}
