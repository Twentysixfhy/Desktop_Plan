#include "taskmodel.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUuid>
#include <QDebug>

// ── TaskItem 序列化 ──────────────────────────────────────

QJsonObject TaskItem::toJson() const {
    QJsonObject obj;
    obj["id"]        = id;
    obj["title"]     = title;
    obj["note"]      = note;
    obj["quadrant"]  = static_cast<int>(quadrant);
    obj["dueDate"]   = dueDate.toString(Qt::ISODate);
    obj["completed"] = completed;
    return obj;
}

TaskItem TaskItem::fromJson(const QJsonObject &obj) {
    TaskItem t;
    t.id        = obj["id"].toString();
    t.title     = obj["title"].toString();
    t.note      = obj["note"].toString();
    t.quadrant  = static_cast<EisenhowerQuadrant>(obj["quadrant"].toInt());
    t.dueDate   = QDate::fromString(obj["dueDate"].toString(), Qt::ISODate);
    t.completed = obj["completed"].toBool();
    if (t.id.isEmpty()) t.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return t;
}

QColor TaskItem::quadrantColor(EisenhowerQuadrant q) {
    switch (q) {
        case EisenhowerQuadrant::UrgentImportant:      return QColor("#E74C3C"); // 红
        case EisenhowerQuadrant::NotUrgentImportant:   return QColor("#3498DB"); // 蓝
        case EisenhowerQuadrant::UrgentNotImportant:   return QColor("#F39C12"); // 橙
        case EisenhowerQuadrant::NotUrgentNotImportant:return QColor("#95A5A6"); // 灰
    }
    return QColor("#95A5A6");
}

QString TaskItem::quadrantName(EisenhowerQuadrant q) {
    switch (q) {
        case EisenhowerQuadrant::UrgentImportant:      return QStringLiteral("重要且紧急");
        case EisenhowerQuadrant::NotUrgentImportant:   return QStringLiteral("重要不紧急");
        case EisenhowerQuadrant::UrgentNotImportant:   return QStringLiteral("紧急不重要");
        case EisenhowerQuadrant::NotUrgentNotImportant:return QStringLiteral("不重要不紧急");
    }
    return {};
}

// ── TaskModel ────────────────────────────────────────────

TaskModel::TaskModel() = default;

void TaskModel::addTask(const TaskItem &task) {
    m_tasks.append(task);
}

void TaskModel::removeTask(const QString &id) {
    m_tasks.erase(std::remove_if(m_tasks.begin(), m_tasks.end(),
        [&](const TaskItem &t) { return t.id == id; }), m_tasks.end());
}

void TaskModel::moveTask(const QString &id, EisenhowerQuadrant newQuadrant) {
    for (auto &t : m_tasks) {
        if (t.id == id) {
            t.quadrant = newQuadrant;
            return;
        }
    }
}

void TaskModel::setCompleted(const QString &id, bool completed) {
    for (auto &t : m_tasks) {
        if (t.id == id) {
            t.completed = completed;
            return;
        }
    }
}

QVector<TaskItem> TaskModel::tasksInQuadrant(EisenhowerQuadrant q) const {
    QVector<TaskItem> result;
    for (const auto &t : m_tasks) {
        if (t.quadrant == q)
            result.append(t);
    }
    return result;
}

void TaskModel::loadFromFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open task file:" << path;
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    m_tasks.clear();
    const QJsonArray arr = doc.array();
    for (const auto &val : arr)
        m_tasks.append(TaskItem::fromJson(val.toObject()));
    qInfo() << "Loaded" << m_tasks.size() << "tasks from" << path;
}

void TaskModel::saveToFile(const QString &path) const {
    QJsonArray arr;
    for (const auto &t : m_tasks)
        arr.append(t.toJson());
    QJsonDocument doc(arr);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly))
        file.write(doc.toJson(QJsonDocument::Indented));
}

QPair<QDate, QDate> TaskModel::currentWeekRange() {
    QDate today = QDate::currentDate();
    int dayOfWeek = today.dayOfWeek(); // 1=Mon ... 7=Sun
    QDate monday = today.addDays(-(dayOfWeek - 1));
    QDate sunday = monday.addDays(6);
    return {monday, sunday};
}
