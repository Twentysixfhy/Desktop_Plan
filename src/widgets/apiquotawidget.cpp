#include "apiquotawidget.h"
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

ApiQuotaWidget::ApiQuotaWidget(QWidget *parent)
    : QWidget(parent)
{
    m_network = new QNetworkAccessManager(this);
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &ApiQuotaWidget::refresh);
    connect(m_network, &QNetworkAccessManager::finished, this, &ApiQuotaWidget::onReplyFinished);
    setupUi();
}

void ApiQuotaWidget::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    m_titleLabel = new QLabel(QStringLiteral("<b>🤖 DeepSeek API 额度</b>"));
    m_titleLabel->setStyleSheet("font-size: 13px;");
    layout->addWidget(m_titleLabel);

    m_balanceLabel = new QLabel(QStringLiteral("加载中…"));
    m_balanceLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #4CAF50;");
    m_balanceLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_balanceLabel);

    m_progressBar = new QProgressBar;
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFixedHeight(16);
    m_progressBar->setStyleSheet(
        "QProgressBar { background: #333; border: none; border-radius: 8px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "  stop:0 #3498DB, stop:1 #2ECC71); border-radius: 8px; }"
    );
    layout->addWidget(m_progressBar);

    m_statusLabel = new QLabel(QStringLiteral("下次刷新: --"));
    m_statusLabel->setStyleSheet("font-size: 11px; color: #888;");
    m_statusLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(m_statusLabel);

    layout->addStretch();
}

void ApiQuotaWidget::setApiKey(const QString &key) {
    m_apiKey = key;
}

void ApiQuotaWidget::setApiUrl(const QString &url) {
    m_apiUrl = url;
}

void ApiQuotaWidget::setRefreshInterval(int seconds) {
    m_refreshTimer->start(seconds * 1000);
    m_statusLabel->setText(QString("每 %1 秒刷新一次").arg(seconds));
}

void ApiQuotaWidget::refresh() {
    if (m_apiKey.isEmpty()) {
        m_balanceLabel->setText("未配置 API Key");
        m_balanceLabel->setStyleSheet("font-size: 16px; color: #E74C3C;");
        return;
    }

    QNetworkRequest request(QUrl(m_apiUrl.isEmpty()
        ? "https://api.deepseek.com/user/balance" : m_apiUrl));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    request.setRawHeader("Accept", "application/json");

    m_network->get(request);
    m_statusLabel->setText("查询中…");
}

void ApiQuotaWidget::onReplyFinished(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        m_balanceLabel->setText("查询失败 ❌");
        m_balanceLabel->setStyleSheet("font-size: 16px; color: #E74C3C;");
        m_statusLabel->setText(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        m_balanceLabel->setText("解析失败");
        return;
    }

    QJsonObject obj = doc.object();

    // DeepSeek balance API 返回格式:
    // { "balance": 12.34, "currency": "CNY", ... }
    double balance = obj["balance"].toDouble();
    QString currency = obj["currency"].toString("¥");
    double total = obj["total_balance"].toDouble();

    m_balanceLabel->setText(QString("%1%2").arg(currency).arg(balance, 0, 'f', 2));
    m_balanceLabel->setStyleSheet(
        balance < 10
            ? "font-size: 24px; font-weight: bold; color: #E74C3C;"
            : "font-size: 24px; font-weight: bold; color: #4CAF50;"
    );

    // 进度条：假设总余额满额是 100 元
    if (total > 0) {
        m_progressBar->setValue(static_cast<int>((balance / total) * 100));
        m_progressBar->setFormat(QStringLiteral("剩余 %1%").arg(balance / total * 100, 0, 'f', 1));
    }

    m_statusLabel->setText(QStringLiteral("上次更新: %1")
        .arg(QDateTime::currentDateTime().toString("HH:mm:ss")));

    qInfo() << "DeepSeek balance:" << balance << currency;
}
