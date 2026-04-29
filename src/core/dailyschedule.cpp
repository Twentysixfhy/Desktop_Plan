#include "dailyschedule.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUuid>
#include <QDate>
#include <QDebug>

// ── ScheduleItem ─────────────────────────────────────────

QJsonObject ScheduleItem::toJson() const {
    QJsonObject obj;
    obj["id"]          = id;
    obj["startTime"]   = startTime.toString("HH:mm");
    obj["endTime"]     = endTime.toString("HH:mm");
    obj["title"]       = title;
    obj["description"] = description;
    obj["color"]       = color.name();
    obj["completed"]   = completed;
    return obj;
}

ScheduleItem ScheduleItem::fromJson(const QJsonObject &obj) {
    ScheduleItem item;
    item.id          = obj["id"].toString();
    item.startTime   = QTime::fromString(obj["startTime"].toString(), "HH:mm");
    item.endTime     = QTime::fromString(obj["endTime"].toString(), "HH:mm");
    item.title       = obj["title"].toString();
    item.description = obj["description"].toString();
    item.color       = QColor(obj["color"].toString());
    item.completed   = obj["completed"].toBool();
    if (item.id.isEmpty()) item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return item;
}

// ── DailySchedule ────────────────────────────────────────

DailySchedule::DailySchedule() = default;

void DailySchedule::addItem(const ScheduleItem &item) {
    m_items.append(item);
}

void DailySchedule::removeItem(const QString &id) {
    m_items.erase(std::remove_if(m_items.begin(), m_items.end(),
        [&](const ScheduleItem &s) { return s.id == id; }), m_items.end());
}

void DailySchedule::setCompleted(const QString &id, bool completed) {
    for (auto &s : m_items) {
        if (s.id == id) {
            s.completed = completed;
            return;
        }
    }
}

QVector<ScheduleItem> DailySchedule::itemsForDate(const QDate & /*date*/) const {
    // 目前简单返回全部；后续可扩展为按日期过滤
    return m_items;
}

void DailySchedule::loadFromFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open schedule file:" << path;
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    m_items.clear();
    const QJsonArray arr = doc.array();
    for (const auto &val : arr)
        m_items.append(ScheduleItem::fromJson(val.toObject()));
    qInfo() << "Loaded" << m_items.size() << "schedule items from" << path;
}

void DailySchedule::saveToFile(const QString &path) const {
    QJsonArray arr;
    for (const auto &s : m_items)
        arr.append(s.toJson());
    QJsonDocument doc(arr);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly))
        file.write(doc.toJson(QJsonDocument::Indented));
}
