#include <QApplication>
#include <QDir>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("DeskDashboard");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Xiaojimao");

    // 深色应用全局风格
    app.setStyle("Fusion");

    // 解析配置路径
    QString configPath;
    QStringList args = app.arguments();
    for (int i = 1; i < args.size(); ++i) {
        if (args[i] == "--config" && i + 1 < args.size()) {
            configPath = args[i + 1];
            break;
        }
    }

    if (configPath.isEmpty()) {
        // 默认：找可执行文件同目录下的 config/config.json
        configPath = QApplication::applicationDirPath() + "/config/config.json";
    }

    qInfo() << "Starting DeskDashboard v1.0.0";
    qInfo() << "Config path:" << configPath;

    MainWindow window;
    window.loadConfig(configPath);
    window.show();

    return app.exec();
}
