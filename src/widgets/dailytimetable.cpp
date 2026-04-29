#include "dailytimetable.h"
#include <QHBoxLayout>
#include <QCheckBox>
#include <QDateTime>
#include <QFrame>

DailyTimetable::DailyTimetable(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void DailyTimetable::setupUi() {
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    // 标题
    auto *header = new QLabel(QStringLiteral("<b>📅 今日计划</b>"));
    header->setStyleSheet("font-size: 14px; padding: 4px 0;");
    outerLayout->addWidget(header);

    // 滚动区域
    m_scroll = new QScrollArea;
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setStyleSheet("QScrollArea { border: none; }");

    m_scrollContent = new QWidget;
    m_timelineLayout = new QVBoxLayout(m_scrollContent);
    m_timelineLayout->setSpacing(2);
    m_timelineLayout->setContentsMargins(0, 0, 0, 0);
    m_timelineLayout->addStretch();

    m_scroll->setWidget(m_scrollContent);
    outerLayout->addWidget(m_scroll);
}

void DailyTimetable::setSchedule(DailySchedule *schedule) {
    m_schedule = schedule;
    refresh();
}

void DailyTimetable::refresh() {
    if (!m_schedule) return;

    // 清除旧条目（保留最后的 stretch）
    while (m_timelineLayout->count() > 1) {
        QLayoutItem *item = m_timelineLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }

    auto items = m_schedule->items();
    std::sort(items.begin(), items.end(),
        [](const ScheduleItem &a, const ScheduleItem &b) {
            return a.startTime < b.startTime;
        });

    // 添加当前时间指示
    QTime now = QTime::currentTime();

    for (const auto &item : items) {
        // 如果当前时间落在此时间块内，添加高亮标记
        bool isCurrent = (item.startTime <= now && now <= item.endTime);

        auto *frame = new QFrame;
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setStyleSheet(
            QString("QFrame { background: %1; border-radius: 6px; padding: 8px; %2 }")
                .arg(item.color.isValid() ? item.color.name() : "#2d2d2d")
                .arg(isCurrent ? "border: 2px solid #4CAF50;" : "border: 1px solid #444;")
        );

        auto *hLayout = new QHBoxLayout(frame);
        hLayout->setContentsMargins(8, 6, 8, 6);

        // 复选框
        auto *cb = new QCheckBox;
        cb->setChecked(item.completed);
        cb->setStyleSheet("QCheckBox::indicator { width: 18px; height: 18px; }");
        connect(cb, &QCheckBox::toggled, this, [this, id = item.id](bool checked) {
            emit itemToggled(id, checked);
        });
        hLayout->addWidget(cb);

        // 时间标签
        auto *timeLabel = new QLabel(
            QString("<b>%1</b> – <b>%2</b>")
                .arg(item.startTime.toString("HH:mm"), item.endTime.toString("HH:mm")));
        timeLabel->setStyleSheet("font-size: 12px; min-width: 80px;");
        hLayout->addWidget(timeLabel);

        // 标题
        auto *titleLabel = new QLabel(item.title);
        titleLabel->setStyleSheet(
            QString("font-size: 13px; %1")
                .arg(item.completed ? "text-decoration: line-through; color: #888;" : ""));
        hLayout->addWidget(titleLabel, 1);

        // 描述
        if (!item.description.isEmpty()) {
            auto *descLabel = new QLabel(
                QString("<small style='color: #999;'>%1</small>").arg(item.description));
            hLayout->addWidget(descLabel);
        }

        m_timelineLayout->insertWidget(m_timelineLayout->count() - 1, frame);
    }
}
