#include "MainWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QDebug>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QMenuBar>
#include <QSerialPortInfo>
#include <QToolBar>
#include <QToolButton>

#include "ProjectIO.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), scene(new CustomScene(this)), view(new QGraphicsView(scene, this))
{
    setCentralWidget(view);
    setupScene();
    setupToolbar();
    setupMenus();

    backgroundImage.load(":/resources/keyboard.png");
    if (!backgroundImage.isNull())
    {
        auto* defaultItem = new QGraphicsPixmapItem(backgroundImage);
        defaultItem->setZValue(-1);
        scene->addItem(defaultItem);
    }

    connect(scene, &CustomScene::diodeAdded, this, &MainWindow::addDiodeItem);
    connect(scene, &CustomScene::buttonAdded, this, &MainWindow::addButtonItem);
    connect(this, &MainWindow::workModeChanged, this, &MainWindow::handleNewWorkMode);
}

void MainWindow::setupScene()
{
    scene->setBackgroundBrush(Qt::lightGray);
    view->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::handleNewWorkMode(WorkMode mode)
{
    currentWorkMode = mode;
    emit modifyModStatusChanged(currentWorkMode == WorkMode::Modify);
}

void MainWindow::setupToolbar()
{
    auto* tb = addToolBar("WorkMode");

    auto* modeMenu = new QMenu(this);
    auto* group    = new QActionGroup(modeMenu);
    group->setExclusive(true);

    auto* actCheck = modeMenu->addAction(tr("Проверка клавиатуры"));
    actCheck->setCheckable(true);
    actCheck->setData(static_cast<int>(WorkMode::Check));
    actCheck->setActionGroup(group);

    auto* actRun = modeMenu->addAction(tr("Режим работы"));
    actRun->setCheckable(true);
    actRun->setData(static_cast<int>(WorkMode::Work));
    actRun->setActionGroup(group);

    auto* actModify = modeMenu->addAction(tr("Режим модификации"));
    actModify->setCheckable(true);
    actModify->setData(static_cast<int>(WorkMode::Modify));
    actModify->setActionGroup(group);

    auto* modeBtn = new QToolButton(this);
    modeBtn->setPopupMode(QToolButton::InstantPopup);
    modeBtn->setMenu(modeMenu);
    modeBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    modeBtn->setDefaultAction(actRun);
    actRun->setChecked(true);

    tb->addWidget(modeBtn);

    connect(group,
            &QActionGroup::triggered,
            this,
            [this, modeBtn](QAction* act)
            {
                modeBtn->setDefaultAction(act);
                const auto mode = static_cast<WorkMode>(act->data().toInt());
                switch (mode)
                {
                    case WorkMode::Check:
                    {
                        appEnterModeCheck();
                        enterCheckMode();
                        break;
                    }
                    case WorkMode::Work:
                    {
                        appEnterModeRun();
                        enterRunMode();
                        break;
                    }
                    case WorkMode::Modify:
                    {
                        break;
                    }
                    default:
                    {
                        qDebug() << "Unrecognize mode";
                        return;
                    }
                }
                emit workModeChanged(mode);
            });

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

    connect(this, &MainWindow::modifyModStatusChanged, loadImgAction, &QAction::setVisible);

    saveProjectAction = tb->addAction("Сохранить проект");
    connect(saveProjectAction, &QAction::triggered, this, &MainWindow::saveProject);
    connect(this, &MainWindow::modifyModStatusChanged, saveProjectAction, &QAction::setVisible);

    loadProjectAction = tb->addAction("Загрузить проект");
    connect(loadProjectAction, &QAction::triggered, this, &MainWindow::loadProject);
    connect(this, &MainWindow::modifyModStatusChanged, loadProjectAction, &QAction::setVisible);
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

        QAction* act = comMenu->addAction(name);

        connect(act,
                &QAction::triggered,
                [this, name]()
                {
                    qDebug() << "Selected COM port:" << name;
                    emit comPortSelected(name);
                });
    }

    {
        // For testing purposes, add a dummy action
        // This should be removed in production code
        // It simulates a port selection for testing without actual hardware

        QString testPortName = "/tmp/ttyV1";

        QAction* actTest = comMenu->addAction(testPortName);

        connect(actTest,
                &QAction::triggered,
                [this, testPortName]()
                {
                    qDebug() << "Selected COM port:" << testPortName;
                    emit comPortSelected(testPortName);
                });
    }

    if (ports.isEmpty())
    {
        comMenu->addAction(tr("No ports found"))->setEnabled(false);
    }
}

void MainWindow::enterCheckMode()
{
    scene->showStatus(true);
}

void MainWindow::enterRunMode()
{
    scene->showStatus(false);
}

void MainWindow::updateStatus(uint8_t pin1, uint8_t pin2, const QVector<uint8_t>& leds)
{
    scene->updateStatus(pin1, pin2, leds);

    for (int i = 0; i < leds.size(); ++i)
    {
        emit updateDiodeStatus(i + 1, leds[i]);
    }
}

void MainWindow::addDiodeItem(DiodeItem* diode)
{
    connect(this, &MainWindow::updateDiodeStatus, diode, &DiodeItem::onStatusUpdate);
    diodeItems.append(diode);
    addResizableItem(diode);
}

void MainWindow::addResizableItem(ResizableRectItem* item)
{
    connect(this, &MainWindow::modifyModStatusChanged, item, &ResizableRectItem::setResizable);
}

void MainWindow::addButtonItem(ButtonItem* button)
{
    connect(button, &ButtonItem::buttonPressed, this, &MainWindow::appButtonPressed);
    connect(button, &ButtonItem::buttonReleased, this, &MainWindow::appButtonReleased);
    connect(this, &MainWindow::updateButtonStatus, button, &ButtonItem::onStatusUpdate);
    addResizableItem(button);
    buttonItems.append(button);
}

void MainWindow::saveProject()
{
    QString path = QFileDialog::getSaveFileName(this, "Save Project", {}, "Keyboard Project (*.kbk)");

    if (path.isEmpty())
    {
        return;
    }

    if (!path.endsWith(".kbk"))
    {
        path += ".kbk";
    }

    Project project;
    for (const DiodeItem* diode : diodeItems)
    {
        project.leds.append(diode->getDefinition());
    }

    for (const ButtonItem* button : buttonItems)
    {
        project.buttons.append(button->getDefinition());
    }

    project.background = backgroundImage.toImage();

    if (!ProjectIO::save(path, project))
    {
        qDebug() << "Failed to save project";
        return;
    }

    qDebug() << "Project saved successfully";
}

void MainWindow::loadProject()
{
    QString path = QFileDialog::getOpenFileName(this, "Load Project", {}, "Keyboard Project (*.kbk)");
    if (path.isEmpty())
    {
        return;
    }

    Project project;
    if (!ProjectIO::load(path, project))
    {
        qDebug() << "Failed to load project";
        return;
    }

    // Clear current items
    scene->clear();

    diodeItems.clear();
    buttonItems.clear();

    // Restore background
    backgroundImage = QPixmap::fromImage(project.background);
    auto* pix       = new QGraphicsPixmapItem(backgroundImage);
    pix->setZValue(-1);
    scene->addItem(pix);
    scene->addStatusItem();

    // Restore LEDs
    for (const auto& ledDef : project.leds)
    {
        auto* diode = new DiodeItem(ledDef);
        scene->addItem(diode);
        addDiodeItem(diode);
    }

    // Restore buttons
    for (const auto& buttonDef : project.buttons)
    {
        auto* button = new ButtonItem(buttonDef);
        scene->addItem(button);
        addButtonItem(button);
    }

    qDebug() << "Project loaded successfully";
}
