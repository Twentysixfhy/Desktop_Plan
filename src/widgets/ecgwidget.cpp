#include "ecgwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPen>
#include <QDateTime>
#include <QtMath>
#include <QDebug>

EcgWidget::EcgWidget(QWidget *parent)
    : QWidget(parent)
{
    m_buffer.resize(MAX_SAMPLES, 0);
    m_bpmTimer.start();

    // ── 布局 ──
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 标题栏
    auto *headerLayout = new QHBoxLayout;
    auto *titleLabel = new QLabel(QStringLiteral("<b>🫀 心电监测</b>"));
    titleLabel->setStyleSheet("font-size: 14px;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    m_bpmLabel = new QLabel("-- BPM");
    m_bpmLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #4CAF50;");
    headerLayout->addWidget(m_bpmLabel);

    m_statusLabel = new QLabel(QStringLiteral("⏹ 已停止"));
    m_statusLabel->setStyleSheet("font-size: 12px; color: #888;");
    headerLayout->addWidget(m_statusLabel);

    m_toggleBtn = new QPushButton(QStringLiteral("▶ 开始"));
    m_toggleBtn->setCheckable(true);
    m_toggleBtn->setFixedWidth(80);
    m_toggleBtn->setStyleSheet(
        "QPushButton { background: #2ECC71; color: white; border: none; "
        "  border-radius: 4px; padding: 4px 12px; font-size: 12px; }"
        "QPushButton:checked { background: #E74C3C; }"
    );
    connect(m_toggleBtn, &QPushButton::toggled, this, [this](bool /*checked*/) {
        toggleRunning();
    });
    headerLayout->addWidget(m_toggleBtn);

    layout->addLayout(headerLayout);

    // 波形绘制区域占满剩余空间（setContentsMargins 确保画布干净）
    layout->addSpacing(4);
}

EcgWidget::~EcgWidget() {
    if (m_running) stop();
}

void EcgWidget::setDataSource(EcgDataSource *source) {
    m_source = source;
    if (m_source) {
        disconnect(m_source, nullptr, this, nullptr);
        connect(m_source, &EcgDataSource::sampleReady, this, &EcgWidget::onSampleReceived);
        connect(m_source, &EcgDataSource::errorOccurred, this, [this](const QString &msg) {
            m_statusLabel->setText("⚠️ " + msg);
        });
    }
}

void EcgWidget::start() {
    if (m_running || !m_source) return;
    m_source->start();
    m_running = true;
    m_toggleBtn->setChecked(true);
    m_statusLabel->setText(QStringLiteral("▶ 运行中"));
    m_statusLabel->setStyleSheet("font-size: 12px; color: #4CAF50;");
    qInfo() << "ECG widget started";
    emit runningChanged(true);
}

void EcgWidget::stop() {
    if (!m_running) return;
    if (m_source) m_source->stop();
    m_running = false;
    m_toggleBtn->setChecked(false);
    m_statusLabel->setText(QStringLiteral("⏹ 已停止"));
    m_statusLabel->setStyleSheet("font-size: 12px; color: #888;");
    qInfo() << "ECG widget stopped";
    emit runningChanged(false);
}

void EcgWidget::toggleRunning() {
    if (m_running) stop();
    else start();
}

void EcgWidget::clearScreen() {
    m_head = 0;
    m_count = 0;
    m_buffer.fill(0);
    update();
}

void EcgWidget::onSampleReceived(double value) {
    // 写入环形缓冲区
    m_buffer[m_head] = value;
    m_head = (m_head + 1) % MAX_SAMPLES;
    if (m_count < MAX_SAMPLES) ++m_count;

    // R 波峰值检测（简单阈值法）
    if (value > m_peakThreshold && value > 0) {
        double now = m_bpmTimer.elapsed() / 1000.0;
        if (now - m_lastRPeakTime > 0.3) { // 防抖动：相距至少 300ms
            m_rPeakCount++;
            m_lastRPeakTime = now;

            if (m_rPeakCount >= 2) {
                // 用最近 5 个 R-R 间隔估算 BPM
                m_bpm = static_cast<int>(60.0 / (now / m_rPeakCount));
                if (m_bpm > 30 && m_bpm < 220) {
                    m_bpmLabel->setText(QString("%1 BPM").arg(m_bpm));
                    m_bpmLabel->setStyleSheet(
                        m_bpm > 100
                            ? "font-size: 16px; font-weight: bold; color: #E74C3C;"
                            : "font-size: 16px; font-weight: bold; color: #4CAF50;");
                }
            }
        }
    }
    // 超时重置计数（3 秒无 R 波）
    if (m_bpmTimer.elapsed() / 1000.0 - m_lastRPeakTime > 3.0) {
        m_rPeakCount = 0;
    }

    update(); // 触发重绘
}

void EcgWidget::paintEvent(QPaintEvent * /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // 布局已处理标题栏区域，contentsRect() 就是绘图区域
    QRect plotRect = contentsRect();

    // 背景
    p.fillRect(plotRect, QColor("#1a1a2e"));

    // 网格
    drawGrid(p, plotRect);

    // 波形
    drawWaveform(p, plotRect);
}

void EcgWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    m_samplesPerScreen = width() / 3; // 每个像素约 3 个采样点
}

void EcgWidget::drawGrid(QPainter &p, const QRect &rect) {
    p.setPen(QPen(QColor("#2a2a3e"), 0.5));

    // 水平网格线
    const int hLines = 5;
    for (int i = 0; i <= hLines; ++i) {
        int y = rect.top() + (rect.height() * i) / hLines;
        p.drawLine(rect.left(), y, rect.right(), y);
    }

    // 垂直网格线
    const int vLines = 10;
    for (int i = 0; i <= vLines; ++i) {
        int x = rect.left() + (rect.width() * i) / vLines;
        p.drawLine(x, rect.top(), x, rect.bottom());
    }

    // 基线（Y 轴中点）加粗
    int midY = rect.top() + rect.height() / 2;
    p.setPen(QPen(QColor("#3a3a5e"), 1));
    p.drawLine(rect.left(), midY, rect.right(), midY);

    // 刻度标签
    p.setPen(QColor("#666"));
    QFont smallFont = p.font();
    smallFont.setPixelSize(9);
    p.setFont(smallFont);
    p.drawText(rect.left() + 2, rect.top() + 10,
               QString("+%1 mV").arg(m_maxValue, 0, 'f', 1));
    p.drawText(rect.left() + 2, rect.bottom() - 2,
               QString("%1 mV").arg(m_minValue, 0, 'f', 1));
}

void EcgWidget::drawWaveform(QPainter &p, const QRect &rect) {
    if (m_count < 2) {
        p.setPen(QColor("#555"));
        p.drawText(rect, Qt::AlignCenter, "等待数据…");
        return;
    }

    double midY = rect.top() + rect.height() / 2.0;
    double range = m_maxValue - m_minValue;
    double scaleY = rect.height() / range;

    // 从缓冲区尾部取最近 samplesPerScreen 个点
    int n = qMin(m_count, m_samplesPerScreen);
    int startIdx = (m_head - n + MAX_SAMPLES) % MAX_SAMPLES;

    QPainterPath path;
    bool first = true;

    for (int i = 0; i < n; ++i) {
        int idx = (startIdx + i) % MAX_SAMPLES;
        double val = m_buffer[idx];
        double x = rect.left() + (static_cast<double>(i) / n) * rect.width();
        double y = midY - val * scaleY;

        // 钳位到绘图区域
        y = qBound(static_cast<double>(rect.top()), y, static_cast<double>(rect.bottom()));

        if (first) {
            path.moveTo(x, y);
            first = false;
        } else {
            path.lineTo(x, y);
        }
    }

    // 绘制波形线
    p.setPen(QPen(QColor("#2ECC71"), 2));
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);

    // 波形下方填充（半透明渐变）
    QPainterPath fillPath = path;
    fillPath.lineTo(rect.right(), midY);
    fillPath.lineTo(rect.left(), midY);
    fillPath.closeSubpath();
    QLinearGradient gradient(0, rect.top(), 0, rect.bottom());
    gradient.setColorAt(0.0, QColor("#2ECC71"));
    gradient.setColorAt(1.0, QColor("#2ECC7100"));
    p.fillPath(fillPath, QBrush(gradient));
}
