#include "mainwindow.h"
#include "core/configmanager.h"
#include "ecg/ecgsimulator.h"
#include "ecg/ecgadc.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_taskModel     = new TaskModel;
    m_scheduleModel = new DailySchedule;
    setupUi();
}

MainWindow::~MainWindow() {
    // 持久化保存
    if (!m_configDir.isEmpty()) {
        m_taskModel->saveToFile(m_configDir + "/tasks.json");
        m_scheduleModel->saveToFile(m_configDir + "/schedule.json");
    }
    delete m_taskModel;
    delete m_scheduleModel;
}

void MainWindow::loadConfig(const QString &configPath) {
    auto &cfg = ConfigManager::instance();
    cfg.load(configPath);

    m_configDir = QFileInfo(configPath).absolutePath();

    // 加载数据
    loadDataFiles();

    // API 额度
    m_apiQuota->setApiKey(cfg.deepSeekApiKey());
    m_apiQuota->setApiUrl(cfg.deepSeekApiUrl());
    m_apiQuota->setRefreshInterval(cfg.refreshIntervalSec());
    m_apiQuota->refresh();

    // ECG 数据源
    if (cfg.ecgEnabled()) {
#ifdef PLATFORM_RASPBERRY_PI
        m_ecgSource = new EcgAdc(cfg.ecgSpiChannel(), cfg.ecgSampleRate(), this);
        qInfo() << "Using ADC-based ECG source";
#else
        m_ecgSource = new EcgSimulator(cfg.ecgSampleRate(), this);
        qInfo() << "Using simulated ECG source";
#endif
        m_ecgWidget->setDataSource(m_ecgSource);
    }
}

void MainWindow::setupUi() {
    setWindowTitle(QStringLiteral("桌面小屏 Dashboard"));
    setMinimumSize(800, 600);

    // 深色主题
    setStyleSheet(
        "QMainWindow { background: #1a1a2e; }"
        "QWidget { color: #e0e0e0; }"
        "QGroupBox { border: 1px solid #333; border-radius: 8px; "
        "  margin-top: 12px; padding: 16px 8px 8px 8px; }"
        "QGroupBox::title { subcontrol-origin: margin; "
        "  subcontrol-position: top center; padding: 0 8px; }"
    );

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setHandleWidth(2);
    m_splitter->setStyleSheet("QSplitter::handle { background: #333; }");

    // 左侧面板
    auto *leftPanel = new QWidget;
    setupLeftPanel(leftPanel);
    m_splitter->addWidget(leftPanel);

    // 右侧面板
    auto *rightPanel = new QWidget;
    setupRightPanel(rightPanel);
    m_splitter->addWidget(rightPanel);

    // 比例 7:3
    m_splitter->setStretchFactor(0, 7);
    m_splitter->setStretchFactor(1, 3);

    setCentralWidget(m_splitter);
}

void MainWindow::setupLeftPanel(QWidget *panel) {
    auto *layout = new QVBoxLayout(panel);
    layout->setSpacing(8);

    // ── 顶部：周计划 + API 额度 ──
    auto *topLayout = new QHBoxLayout;

    // 艾森豪威尔矩阵
    auto *planGroup = new QGroupBox(QStringLiteral("📋 周计划 (艾森豪威尔矩阵)"));
    auto *planLayout = new QVBoxLayout(planGroup);
    m_eisenhowerMatrix = new EisenhowerMatrix;
    m_eisenhowerMatrix->setModel(m_taskModel);
    planLayout->addWidget(m_eisenhowerMatrix);
    topLayout->addWidget(planGroup, 3);

    // API 额度
    auto *apiGroup = new QGroupBox(QStringLiteral("💳 DeepSeek"));
    auto *apiLayout = new QVBoxLayout(apiGroup);
    m_apiQuota = new ApiQuotaWidget;
    apiLayout->addWidget(m_apiQuota);
    topLayout->addWidget(apiGroup, 1);

    layout->addLayout(topLayout);

    // ── 底部：心电监测 ──
    auto *ecgGroup = new QGroupBox(QStringLiteral("🫀 心电"));
    auto *ecgLayout = new QVBoxLayout(ecgGroup);
    m_ecgWidget = new EcgWidget;
    m_ecgWidget->setMinimumHeight(200);
    ecgLayout->addWidget(m_ecgWidget);
    layout->addWidget(ecgGroup, 2);
}

void MainWindow::setupRightPanel(QWidget *panel) {
    auto *layout = new QVBoxLayout(panel);

    auto *scheduleGroup = new QGroupBox(QStringLiteral("⏰ 每日时间表"));
    auto *scheduleLayout = new QVBoxLayout(scheduleGroup);
    m_dailyTimetable = new DailyTimetable;
    m_dailyTimetable->setSchedule(m_scheduleModel);
    scheduleLayout->addWidget(m_dailyTimetable);

    layout->addWidget(scheduleGroup);

    // 底部日期和时间
    auto *timeLabel = new QLabel;
    timeLabel->setAlignment(Qt::AlignCenter);
    timeLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #4CAF50; padding: 16px;");
    auto *dateLabel = new QLabel;
    dateLabel->setAlignment(Qt::AlignCenter);
    dateLabel->setStyleSheet("font-size: 14px; color: #888; padding: 4px;");

    // 定时刷新时钟
    auto *clockTimer = new QTimer(this);
    connect(clockTimer, &QTimer::timeout, this, [timeLabel, dateLabel]() {
        timeLabel->setText(QDateTime::currentDateTime().toString("HH:mm:ss"));
        dateLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd dddd"));
    });
    clockTimer->start(1000);

    layout->addWidget(timeLabel);
    layout->addWidget(dateLabel);
    layout->addStretch();
}

void MainWindow::connectSignals() {
    // 艾森豪威尔矩阵拖拽事件 → 更新模型
    connect(m_eisenhowerMatrix, &EisenhowerMatrix::taskMoved,
        this, [this](const QString &taskId, EisenhowerQuadrant to) {
            m_taskModel->moveTask(taskId, to);
            m_eisenhowerMatrix->refresh();
        });

    // 每日计划勾选事件
    connect(m_dailyTimetable, &DailyTimetable::itemToggled,
        this, [this](const QString &itemId, bool completed) {
            m_scheduleModel->setCompleted(itemId, completed);
            m_dailyTimetable->refresh();
        });
}

void MainWindow::loadDataFiles() {
    QString taskFile = ConfigManager::instance().taskFilePath();
    QString scheduleFile = ConfigManager::instance().scheduleFilePath();

    // 如果文件不存在，创建示例数据
    if (!QFileInfo::exists(taskFile)) {
        qInfo() << "No task file found, creating sample data...";
        TaskItem sample;
        sample.title = QStringLiteral("学习 Qt6 开发");
        sample.quadrant = EisenhowerQuadrant::NotUrgentImportant;
        m_taskModel->addTask(sample);

        sample.id.clear();
        sample.title = QStringLiteral("提交周报");
        sample.quadrant = EisenhowerQuadrant::UrgentImportant;
        m_taskModel->addTask(sample);

        sample.id.clear();
        sample.title = QStringLiteral("回复邮件");
        sample.quadrant = EisenhowerQuadrant::UrgentNotImportant;
        m_taskModel->addTask(sample);

        sample.id.clear();
        sample.title = QStringLiteral("刷短视频");
        sample.quadrant = EisenhowerQuadrant::NotUrgentNotImportant;
        m_taskModel->addTask(sample);

        m_taskModel->saveToFile(taskFile);
    } else {
        m_taskModel->loadFromFile(taskFile);
    }

    if (!QFileInfo::exists(scheduleFile)) {
        qInfo() << "No schedule file found, creating sample data...";
        ScheduleItem s;
        s.startTime = QTime(8, 0);
        s.endTime   = QTime(9, 0);
        s.title     = QStringLiteral("起床 & 早餐");
        s.color     = QColor("#3498DB");
        m_scheduleModel->addItem(s);

        s.id.clear();
        s.startTime = QTime(9, 0);
        s.endTime   = QTime(12, 0);
        s.title     = QStringLiteral("看文献 / 学习");
        s.color     = QColor("#2ECC71");
        m_scheduleModel->addItem(s);

        s.id.clear();
        s.startTime = QTime(12, 0);
        s.endTime   = QTime(14, 0);
        s.title     = QStringLiteral("午餐 & 休息");
        s.color     = QColor("#F39C12");
        m_scheduleModel->addItem(s);

        s.id.clear();
        s.startTime = QTime(14, 0);
        s.endTime   = QTime(17, 0);
        s.title     = QStringLiteral("项目开发");
        s.color     = QColor("#9B59B6");
        m_scheduleModel->addItem(s);

        s.id.clear();
        s.startTime = QTime(17, 0);
        s.endTime   = QTime(18, 0);
        s.title     = QStringLiteral("晚餐");
        s.color     = QColor("#E67E22");
        m_scheduleModel->addItem(s);

        s.id.clear();
        s.startTime = QTime(19, 0);
        s.endTime   = QTime(22, 0);
        s.title     = QStringLiteral("自由时间 / 业余项目");
        s.color     = QColor("#1ABC9C");
        m_scheduleModel->addItem(s);

        m_scheduleModel->saveToFile(scheduleFile);
    } else {
        m_scheduleModel->loadFromFile(scheduleFile);
    }

    // 刷新显示
    m_eisenhowerMatrix->refresh();
    m_dailyTimetable->refresh();

    connectSignals();
}
