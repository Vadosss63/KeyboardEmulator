#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);

private:
    QGraphicsScene* scene;
    QGraphicsView*  view;

    void setupScene();
};
