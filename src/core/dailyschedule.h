#ifndef DAILYSCHEDULE_H
#define DAILYSCHEDULE_H

#include <QString>
#include <QTime>
#include <QColor>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>

/// 每日时间表的一个条目
struct ScheduleItem {
    QString id;
    QTime   startTime;   // 开始时间
    QTime   endTime;     // 结束时间
    QString title;
    QString description;
    QColor  color;       // 显示颜色
    bool    completed = false;

    QJsonObject toJson() const;
    static ScheduleItem fromJson(const QJsonObject &obj);
};

/// 每日计划模型
class DailySchedule {
public:
    DailySchedule();

    void addItem(const ScheduleItem &item);
    void removeItem(const QString &id);
    void setCompleted(const QString &id, bool completed);

    QVector<ScheduleItem> items() const { return m_items; }
    QVector<ScheduleItem> itemsForDate(const QDate &date) const;

    // 持久化
    void loadFromFile(const QString &path);
    void saveToFile(const QString &path) const;

private:
    QVector<ScheduleItem> m_items;
    QString m_filePath;
};

#endif // DAILYSCHEDULE_H
