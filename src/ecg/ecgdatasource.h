#ifndef ECG_DATASOURCE_H
#define ECG_DATASOURCE_H

#include <QObject>

/// ECG 数据源抽象接口
/// 提供统一的采样值流，不关心底层是模拟器还是 ADC
class EcgDataSource : public QObject {
    Q_OBJECT
public:
    explicit EcgDataSource(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~EcgDataSource() = default;

    /// 启动数据采集（子类应开始发射 sampleReady 信号）
    virtual void start() = 0;

    /// 停止采集
    virtual void stop()  = 0;

    /// 是否正在运行
    virtual bool isRunning() const = 0;

    /// 采样率（Hz）
    virtual int sampleRate() const = 0;

signals:
    /// 每采集到一个样本点发射一次
    /// @param value 原始 ADC 值（0-4095 或归一化到 mV）
    void sampleReady(double value);

    /// 发生错误
    void errorOccurred(const QString &message);
};

#endif // ECG_DATASOURCE_H
