#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTabWidget>

#include "widgets/eisenhowermatrix.h"
#include "widgets/dailytimetable.h"
#include "widgets/apiquotawidget.h"
#include "widgets/ecgwidget.h"
#include "core/taskmodel.h"
#include "core/dailyschedule.h"

/// 主窗口 — 左侧周计划/API/ECG，右侧每日时间表
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void loadConfig(const QString &configPath);

private:
    void setupUi();
    void setupLeftPanel(QWidget *panel);
    void setupRightPanel(QWidget *panel);
    void connectSignals();
    void loadDataFiles();

    // Data models
    TaskModel     *m_taskModel     = nullptr;
    DailySchedule *m_scheduleModel = nullptr;

    // Data sources (ECG)
    EcgDataSource *m_ecgSource     = nullptr;

    // Widgets
    EisenhowerMatrix *m_eisenhowerMatrix = nullptr;
    DailyTimetable   *m_dailyTimetable   = nullptr;
    ApiQuotaWidget   *m_apiQuota         = nullptr;
    EcgWidget        *m_ecgWidget        = nullptr;

    QSplitter *m_splitter = nullptr;
    QString m_configDir;
};

#endif // MAINWINDOW_H
