#include "configmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDebug>

ConfigManager &ConfigManager::instance() {
    static ConfigManager inst;
    return inst;
}

void ConfigManager::load(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Config not found at" << path << "— using defaults.";
        m_root = QJsonObject();
        return;
    }
    m_root = QJsonDocument::fromJson(file.readAll()).object();
    file.close();
    qInfo() << "Config loaded from" << path;
}

QString ConfigManager::deepSeekApiKey() const {
    return m_root["deepseek"].toObject()["apiKey"].toString();
}

QString ConfigManager::deepSeekApiUrl() const {
    return m_root["deepseek"].toObject()["apiUrl"].toString(
        "https://api.deepseek.com/user/balance");
}

QString ConfigManager::taskFilePath() const {
    return m_root["data"].toObject()["tasks"].toString(
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + "/tasks.json");
}

QString ConfigManager::scheduleFilePath() const {
    return m_root["data"].toObject()["schedule"].toString(
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + "/schedule.json");
}

bool ConfigManager::ecgEnabled() const {
    return m_root["ecg"].toObject()["enabled"].toBool(true);
}

int ConfigManager::ecgSpiChannel() const {
    return m_root["ecg"].toObject()["spiChannel"].toInt(0);
}

int ConfigManager::ecgSampleRate() const {
    return m_root["ecg"].toObject()["sampleRate"].toInt(250);
}

int ConfigManager::refreshIntervalSec() const {
    return m_root["display"].toObject()["refreshIntervalSec"].toInt(60);
}
