#ifndef EISENHOWERMATRIX_H
#define EISENHOWERMATRIX_H

#include <QWidget>
#include <QGridLayout>
#include <QListWidget>
#include <QLabel>
#include <QFrame>
#include <QMimeData>
#include <QDropEvent>
#include "core/taskmodel.h"

/// 可拖拽的象限列表（支持接收跨象限拖放）
class DragListWidget : public QListWidget {
    Q_OBJECT
public:
    explicit DragListWidget(EisenhowerQuadrant quadrant, QWidget *parent = nullptr)
        : QListWidget(parent), m_quadrant(quadrant)
    {
        setDragEnabled(true);
        setAcceptDrops(true);
        setDropIndicatorShown(true);
        setDefaultDropAction(Qt::MoveAction);
        setDragDropMode(QAbstractItemView::DragDrop);
        setSelectionMode(QAbstractItemView::SingleSelection);
        setFrameShape(QFrame::NoFrame);
        setStyleSheet(
            "QListWidget { background: transparent; border: 1px dashed #ccc; "
            "  border-radius: 6px; padding: 4px; }"
            "QListWidget::item { padding: 6px 8px; border-radius: 4px; margin: 2px; }"
            "QListWidget::item:hover { background: rgba(255,255,255,0.1); }"
        );
    }

    EisenhowerQuadrant quadrant() const { return m_quadrant; }

signals:
    void taskDropped(const QString &taskId, EisenhowerQuadrant toQuadrant);

protected:
    void dropEvent(QDropEvent *event) override {
        const QMimeData *mime = event->mimeData();
        if (mime->hasFormat("application/x-eisenhower-task")) {
            QByteArray data = mime->data("application/x-eisenhower-task");
            QString taskId = QString::fromUtf8(data);
            emit taskDropped(taskId, m_quadrant);
        }
        QListWidget::dropEvent(event);
    }

private:
    EisenhowerQuadrant m_quadrant;
};

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
