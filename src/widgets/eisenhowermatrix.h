#ifndef EISENHOWERMATRIX_H
#define EISENHOWERMATRIX_H

#include <QWidget>
#include <QGridLayout>
#include <QListWidget>
#include <QLabel>
#include "core/taskmodel.h"

/// 艾森豪威尔矩阵小部件
/// 2×2 网格，每个象限显示对应的任务列表
/// 支持拖拽移动任务到不同象限
class EisenhowerMatrix : public QWidget {
    Q_OBJECT
public:
    explicit EisenhowerMatrix(QWidget *parent = nullptr);

    void setModel(TaskModel *model);
    void refresh();

signals:
    void taskAdded(const TaskItem &task);
    void taskMoved(const QString &taskId, EisenhowerQuadrant to);
    void taskDoubleClicked(const TaskItem &task);

private:
    void setupUi();
    QListWidget *listForQuadrant(EisenhowerQuadrant q) const;
    QGridLayout *m_layout = nullptr;
    QLabel *m_titleLabels[4]  = {};
    QListWidget *m_lists[4]   = {};
    TaskModel *m_model = nullptr;
};

#endif // EISENHOWERMATRIX_H
