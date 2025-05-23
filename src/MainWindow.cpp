#include "MainWindow.h"

#include <QAction>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QToolBar>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    scene = new QGraphicsScene(this);
    view  = new QGraphicsView(scene, this);
    setCentralWidget(view);

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
                    auto* btn = scene->addRect(50, 50, 80, 30, QPen(Qt::blue), QBrush(Qt::transparent));
                    btn->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
                }
            });

    setupScene();
}

void MainWindow::setupScene()
{
    scene->setBackgroundBrush(Qt::lightGray);
    view->setRenderHint(QPainter::Antialiasing);
    view->setDragMode(QGraphicsView::RubberBandDrag);
}
