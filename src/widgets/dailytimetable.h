#ifndef DAILYTIMETABLE_H
#define DAILYTIMETABLE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QTime>
#include "core/dailyschedule.h"

/// 每日时间表小部件
/// 纵轴显示 00:00-24:00，横排显示每个时间块的任务
/// 已完成条目有删除线标记
class DailyTimetable : public QWidget {
    Q_OBJECT
public:
    explicit DailyTimetable(QWidget *parent = nullptr);

    void setSchedule(DailySchedule *schedule);
    void refresh();

signals:
    void itemToggled(const QString &itemId, bool completed);

private:
    void setupUi();
    void addTimeSlot(const ScheduleItem &item);

    QVBoxLayout *m_layout = nullptr;
    QScrollArea *m_scroll = nullptr;
    QWidget *m_scrollContent = nullptr;
    QVBoxLayout *m_timelineLayout = nullptr;
    DailySchedule *m_schedule = nullptr;
};

#endif // DAILYTIMETABLE_H
