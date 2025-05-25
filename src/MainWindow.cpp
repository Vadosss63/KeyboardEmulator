#include "MainWindow.h"

#include <QAction>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QToolBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), scene(new CustomScene(this)), view(new QGraphicsView(scene, this))
{
    setCentralWidget(view);
    setupScene();

    QPixmap defaultPix(":/resources/keyboard.png");
    if (!defaultPix.isNull())
    {
        auto* defaultItem = new QGraphicsPixmapItem(defaultPix);
        defaultItem->setZValue(-1);
        scene->addItem(defaultItem);
    }

    auto*    tb      = addToolBar("File");
    QAction* loadImg = tb->addAction("Load Keyboard...");
    connect(loadImg,
            &QAction::triggered,
            [this]()
            {
                QString path = QFileDialog::getOpenFileName(this, "Select Keyboard Image", {}, "Images (*.png *.jpg)");
                if (!path.isEmpty())
                {
                    scene->clear();
                    auto* pix = new QGraphicsPixmapItem(QPixmap(path));
                    scene->addItem(pix);
                    pix->setZValue(-1);
                }
            });
}

void MainWindow::setupScene()
{
    scene->setBackgroundBrush(Qt::lightGray);
    view->setRenderHint(QPainter::Antialiasing);
    view->setDragMode(QGraphicsView::RubberBandDrag);
}
