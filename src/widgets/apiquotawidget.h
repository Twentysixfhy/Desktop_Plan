#ifndef APIQUOTAWIDGET_H
#define APIQUOTAWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

/// DeepSeek API 额度显示小部件
class ApiQuotaWidget : public QWidget {
    Q_OBJECT
public:
    explicit ApiQuotaWidget(QWidget *parent = nullptr);

    void setApiKey(const QString &key);
    void setApiUrl(const QString &url);
    void setRefreshInterval(int seconds);

public slots:
    void refresh();

private:
    void setupUi();
    void onReplyFinished(QNetworkReply *reply);

    QLabel *m_titleLabel  = nullptr;
    QLabel *m_balanceLabel = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QLabel *m_statusLabel = nullptr;

    QString m_apiKey;
    QString m_apiUrl;
    QNetworkAccessManager *m_network = nullptr;
    QTimer *m_refreshTimer = nullptr;
};

#endif // APIQUOTAWIDGET_H
