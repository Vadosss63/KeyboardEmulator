#include "MainWindow.h"

#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QMenuBar>
#include <QSerialPortInfo>
#include <QToolBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), scene(new CustomScene(this)), view(new QGraphicsView(scene, this))
{
    setCentralWidget(view);
    setupScene();
    setupToolbar();
    setupMenus();

    QPixmap defaultPix(":/resources/keyboard.png");
    if (!defaultPix.isNull())
    {
        auto* defaultItem = new QGraphicsPixmapItem(defaultPix);
        defaultItem->setZValue(-1);
        scene->addItem(defaultItem);
    }

    connect(scene, &CustomScene::diodeAdded, this, &MainWindow::addDiodeItem);
    connect(scene, &CustomScene::buttonAdded, this, &MainWindow::addButtonItem);
}

void MainWindow::setupScene()
{
    scene->setBackgroundBrush(Qt::lightGray);
    view->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::setupToolbar()
{
    auto* tb = addToolBar("File");

    loadImgAction = tb->addAction("Загрузка изображения");
    connect(loadImgAction,
            &QAction::triggered,
            [this]()
            {
                QString path = QFileDialog::getOpenFileName(this, "Select Keyboard Image", {}, "Images (*.png *.jpg)");
                if (!path.isEmpty())
                {
                    scene->clear();
                    auto* pix = new QGraphicsPixmapItem(QPixmap(path));
                    pix->setZValue(-1);
                    scene->addItem(pix);
                }
            });

    modeCheckAction = tb->addAction("Проверка клавиатуры");
    connect(modeCheckAction, &QAction::triggered, this, &MainWindow::appEnterModeCheck);

    modeRunAction = tb->addAction("Режим работы");
    connect(modeRunAction, &QAction::triggered, this, &MainWindow::appEnterModeRun);
}

void MainWindow::setupMenus()
{
    comMenu = menuBar()->addMenu(tr("COM Port"));
    connect(comMenu, &QMenu::aboutToShow, this, &MainWindow::refreshComPorts);
}

void MainWindow::refreshComPorts()
{
    comMenu->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& info : ports)
    {
        const QString name = info.portName();
        QAction*      act  = comMenu->addAction(name);
        connect(act,
                &QAction::triggered,
                this,
                [this, name]()
                {
                    qDebug() << "Selected COM port:" << name;
                    emit comPortSelected(name);
                });
    }
    if (ports.isEmpty())
    {
        comMenu->addAction(tr("No ports found"))->setEnabled(false);
    }
}

void MainWindow::markButtonPressed(uint8_t pin1, uint8_t pin2)
{
    qDebug() << "View: button pressed on pins" << pin1 << pin2;
    // TODO: найти ButtonItem по pin1/pin2 и вызвать updateAppearance(true)
}

void MainWindow::markButtonReleased(uint8_t pin1, uint8_t pin2)
{
    qDebug() << "View: button released on pins" << pin1 << pin2;
    // TODO: найти ButtonItem и снять выделение
}

void MainWindow::enterCheckMode()
{
    qDebug() << "View: entered CHECK mode";
    // TODO: визуально отметить режим проверки
}

void MainWindow::enterRunMode()
{
    qDebug() << "View: entered RUN mode";
    // TODO: сбросить визуальные подсказки
}

void MainWindow::updateStatus(uint8_t status, uint8_t pin1, uint8_t pin2, const QVector<uint8_t>& leds)
{
    qDebug() << "Status=" << status << "pins:" << pin1 << pin2 << "LEDs:" << leds;

    for (int i = 0; i < leds.size(); ++i)
    {
        emit updateDiodeStatus(i, leds[i]);
    }
}

void MainWindow::addDiodeItem(DiodeItem* diode)
{
    connect(this, &MainWindow::updateDiodeStatus, diode, &DiodeItem::onStatusUpdate);
}

void MainWindow::addButtonItem(ButtonItem* button) {}
