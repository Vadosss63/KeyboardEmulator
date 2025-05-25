#pragma once

#include <QGraphicsView>
#include <QMainWindow>

#include "CustomScene.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);

private:
    void setupScene();

    CustomScene*   scene;
    QGraphicsView* view;
};
