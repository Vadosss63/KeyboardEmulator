#include "StartScreenWidget.h"

#include <QFileInfo>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QSize>
#include <QVBoxLayout>

StartScreenWidget::StartScreenWidget(QWidget* parent) : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    auto* column = new QWidget(this);
    auto* colLay = new QVBoxLayout(column);
    colLay->setContentsMargins(0, 0, 0, 0);
    colLay->setSpacing(8);
    root->addWidget(column, 0, Qt::AlignHCenter | Qt::AlignTop);

    auto* label = new QLabel(tr("Выберите изображение или загрузите проект:"), column);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    colLay->addWidget(label);

    const int SIDE_PAD    = 24;
    const int columnWidth = QFontMetrics(label->font()).horizontalAdvance(label->text()) + SIDE_PAD;

    auto* btnLoadImage   = new QPushButton(tr("Загрузить изображение"), column);
    auto* btnLoadProject = new QPushButton(tr("Загрузить проект"), column);
    btnLoadImage->setFixedWidth(columnWidth);
    btnLoadProject->setFixedWidth(columnWidth);

    colLay->addWidget(btnLoadImage, 0, Qt::AlignLeft);
    colLay->addWidget(btnLoadProject, 0, Qt::AlignLeft);

    connect(btnLoadImage, &QPushButton::clicked, this, &StartScreenWidget::loadImageRequested);
    connect(btnLoadProject, &QPushButton::clicked, this, &StartScreenWidget::loadProjectRequested);

    auto* sep = new QFrame(column);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    colLay->addWidget(sep);

    auto* recentLabel = new QLabel(tr("Последние проекты:"), column);
    recentLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    colLay->addWidget(recentLabel);

    recentList = new QListWidget(column);
    recentList->setFixedWidth(columnWidth);
    recentList->setUniformItemSizes(true);
    recentList->setAlternatingRowColors(false);
    recentList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    colLay->addWidget(recentList, 0, Qt::AlignLeft);

    connect(recentList,
            &QListWidget::itemActivated,
            this,
            [this](QListWidgetItem* item)
            {
                if (!item)
                {
                    return;
                }
                const QString path = item->data(Qt::UserRole).toString();
                if (!path.isEmpty())
                {
                    emit recentItemActivated(path);
                }
            });

    auto* row = new QHBoxLayout;
    row->addStretch();
    clearButton = new QPushButton(tr("Очистить список"), column);
    row->addWidget(clearButton);
    colLay->addLayout(row);

    connect(clearButton, &QPushButton::clicked, this, &StartScreenWidget::clearRecentRequested);

    resizeRecentListToContents();
    setMinimumSize(column->sizeHint().expandedTo(QSize(columnWidth + 24, 0)));
}

void StartScreenWidget::setRecentEntries(const QStringList& entries)
{
    if (!recentList)
    {
        return;
    }

    recentList->clear();
    for (const QString& path : entries)
    {
        QFileInfo fi(path);
        auto*     item = new QListWidgetItem(fi.fileName(), recentList);
        item->setToolTip(path);
        item->setData(Qt::UserRole, path);
        recentList->addItem(item);
    }

    recentList->setEnabled(!entries.isEmpty());
    if (clearButton)
    {
        clearButton->setEnabled(!entries.isEmpty());
    }
    resizeRecentListToContents();
}

void StartScreenWidget::resizeRecentListToContents()
{
    if (!recentList)
    {
        return;
    }

    int rows = recentList->count();
    int rowH = recentList->sizeHintForRow(0);
    if (rowH <= 0)
    {
        rowH = QFontMetrics(recentList->font()).height() + 8;
    }

    const int maxRows = 5;
    const int visible = qMax(1, qMin(rows, maxRows));
    const int frame   = 2 * recentList->frameWidth();

    const int h = (visible + 1) * rowH + frame + 4;
    recentList->setFixedHeight(h);
}
