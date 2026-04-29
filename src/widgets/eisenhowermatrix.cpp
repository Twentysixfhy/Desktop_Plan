#include "eisenhowermatrix.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QApplication>

// ── 可拖拽的 ListWidget ─────────────────────────────────

class DragListWidget : public QListWidget {
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
            "QListWidget { background: transparent; border: 1px dashed #ccc; border-radius: 6px; padding: 4px; }"
            "QListWidget::item { padding: 6px 8px; border-radius: 4px; margin: 2px; }"
            "QListWidget::item:hover { background: rgba(255,255,255,0.1); }"
        );
    }

    EisenhowerQuadrant quadrant() const { return m_quadrant; }

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

signals:
    void taskDropped(const QString &taskId, EisenhowerQuadrant toQuadrant);

private:
    EisenhowerQuadrant m_quadrant;
};

// ── EisenhowerMatrix ─────────────────────────────────────

EisenhowerMatrix::EisenhowerMatrix(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void EisenhowerMatrix::setModel(TaskModel *model) {
    m_model = model;
    refresh();
}

void EisenhowerMatrix::refresh() {
    if (!m_model) return;

    for (int i = 0; i < 4; ++i) {
        auto q = static_cast<EisenhowerQuadrant>(i);
        auto *list = m_lists[i];
        list->clear();

        auto tasks = m_model->tasksInQuadrant(q);
        for (const auto &t : tasks) {
            auto *item = new QListWidgetItem(t.title);
            item->setData(Qt::UserRole, t.id);
            item->setData(Qt::UserRole + 1, static_cast<int>(q));

            if (t.completed) {
                QFont f = item->font();
                f.setStrikeOut(true);
                item->setFont(f);
                item->setForeground(QColor("#888"));
            } else {
                QColor bg = TaskItem::quadrantColor(q);
                bg.setAlpha(30);
                item->setBackground(bg);
                item->setForeground(TaskItem::quadrantColor(q));
            }
            list->addItem(item);
        }
    }
}

void EisenhowerMatrix::setupUi() {
    m_layout = new QGridLayout(this);
    m_layout->setSpacing(6);

    // 四象限标签和列表
    struct QuadInfo {
        QString name;
        QString desc;
        EisenhowerQuadrant q;
        int row, col;
    };

    QuadInfo quads[] = {
        {QStringLiteral("重要且紧急 ⚠️"),   QStringLiteral("立即去做"),  EisenhowerQuadrant::UrgentImportant,      0, 1},
        {QStringLiteral("重要不紧急 🎯"),   QStringLiteral("规划去做"),  EisenhowerQuadrant::NotUrgentImportant,   0, 0},
        {QStringLiteral("紧急不重要 📞"),   QStringLiteral("委托/简化"), EisenhowerQuadrant::UrgentNotImportant,   1, 1},
        {QStringLiteral("不重要不紧急 🗑️"), QStringLiteral("尽量不做"),  EisenhowerQuadrant::NotUrgentNotImportant, 1, 0},
    };

    for (int i = 0; i < 4; ++i) {
        auto &q = quads[i];

        auto *titleLabel = new QLabel(QString("<b>%1</b><br><small>%2</small>").arg(q.name, q.desc));
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet(
            QString("padding: 8px; border-radius: 6px; background: %1; color: white;")
                .arg(TaskItem::quadrantColor(q.q).name())
        );
        m_titleLabels[i] = titleLabel;

        auto *list = new DragListWidget(q.q);
        list->setMinimumHeight(120);
        m_lists[i] = list;

        connect(list, &DragListWidget::taskDropped, this, &EisenhowerMatrix::taskMoved);
        connect(list, &QListWidget::itemDoubleClicked, this, [this, q](QListWidgetItem *lwi) {
            if (!lwi) return;
            TaskItem t;
            t.id = lwi->data(Qt::UserRole).toString();
            t.title = lwi->text();
            t.quadrant = static_cast<EisenhowerQuadrant>(lwi->data(Qt::UserRole + 1).toInt());
            emit taskDoubleClicked(t);
        });

        m_layout->addWidget(titleLabel, q.row * 2, q.col);
        m_layout->addWidget(list, q.row * 2 + 1, q.col);
    }
}

QListWidget *EisenhowerMatrix::listForQuadrant(EisenhowerQuadrant q) const {
    return m_lists[static_cast<int>(q)];
}
