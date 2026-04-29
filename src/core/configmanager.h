#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QJsonObject>

/// 全局配置管理（DeepSeek API Key、数据路径、显示偏好等）
class ConfigManager {
public:
    static ConfigManager &instance();

    void load(const QString &path);

    // DeepSeek
    QString deepSeekApiKey() const;
    QString deepSeekApiUrl() const;

    // 数据文件路径
    QString taskFilePath() const;
    QString scheduleFilePath() const;

    // ECG
    bool ecgEnabled() const;
    int  ecgSpiChannel() const;
    int  ecgSampleRate() const;

    // 其他显示配置
    int refreshIntervalSec() const;

private:
    ConfigManager() = default;
    QJsonObject m_root;
};

#endif // CONFIGMANAGER_H
