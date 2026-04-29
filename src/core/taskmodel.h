#ifndef TASKMODEL_H
#define TASKMODEL_H

#include <QString>
#include <QColor>
#include <QJsonObject>
#include <QJsonArray>
#include <QDate>

/// 艾森豪威尔矩阵四象限
enum class EisenhowerQuadrant {
    UrgentImportant,      // 重要且紧急  — 红色
    NotUrgentImportant,   // 重要不紧急  — 蓝色
    UrgentNotImportant,   // 紧急不重要  — 橙色
    NotUrgentNotImportant // 不重要不紧急 — 灰色
};

/// 单个任务项
struct TaskItem {
    QString id;
    QString title;
    QString note;
    EisenhowerQuadrant quadrant = EisenhowerQuadrant::NotUrgentNotImportant;
    QDate dueDate;
    bool completed = false;

    QJsonObject toJson() const;
    static TaskItem fromJson(const QJsonObject &obj);
    static QColor quadrantColor(EisenhowerQuadrant q);
    static QString quadrantName(EisenhowerQuadrant q);
};

/// 周计划模型（一组任务，按象限归类）
class TaskModel {
public:
    TaskModel();

    void addTask(const TaskItem &task);
    void removeTask(const QString &id);
    void moveTask(const QString &id, EisenhowerQuadrant newQuadrant);
    void setCompleted(const QString &id, bool completed);

    QVector<TaskItem> tasksInQuadrant(EisenhowerQuadrant q) const;
    QVector<TaskItem> allTasks() const { return m_tasks; }

    // 持久化
    void loadFromFile(const QString &path);
    void saveToFile(const QString &path) const;

    // 本周日期范围
    static QPair<QDate, QDate> currentWeekRange();

private:
    QVector<TaskItem> m_tasks;
};

#endif // TASKMODEL_H
