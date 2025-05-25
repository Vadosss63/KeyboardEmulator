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
    CustomScene*   scene;
    QGraphicsView* view;

    void setupScene();
};
