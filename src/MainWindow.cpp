#include "MainWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QDebug>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QLabel>
#include <QMenuBar>
#include <QPushButton>
#include <QSerialPortInfo>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include "ProjectIO.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , startWidget(new QWidget(this))
    , scene(new CustomScene(this))
    , view(new QGraphicsView(scene, this))
    , stackedWidget(new QStackedWidget(this))
{
    setWindowTitle("Эмулятор клавиатуры");

    setupScene();
    setupToolbar();
    setupMenus();

    connect(scene, &CustomScene::diodeAdded, this, &MainWindow::addDiodeItem);
    connect(scene, &CustomScene::buttonAdded, this, &MainWindow::addButtonItem);
    connect(this, &MainWindow::workModeChanged, this, &MainWindow::handleNewWorkMode);
    connect(this, &MainWindow::modifyModStatusChanged, scene, &CustomScene::setModifiable);

    emit workModeChanged(WorkMode::Modify);
    createStartWidget();

    stackedWidget->addWidget(startWidget);
    stackedWidget->addWidget(view);
    stackedWidget->setCurrentWidget(startWidget);
    setCentralWidget(stackedWidget);

    connect(this, &MainWindow::projectReady, &MainWindow::enableSceneMode);
}

void MainWindow::createStartWidget()
{
    auto* layout = new QVBoxLayout(startWidget);
    layout->setAlignment(Qt::AlignCenter);
    auto* label = new QLabel("Выберите изображение или загрузите проект:", startWidget);
    layout->addWidget(label);
    auto* btnLoadImage = new QPushButton("Загрузить изображение", startWidget);
    layout->addWidget(btnLoadImage);
    connect(btnLoadImage, &QPushButton::clicked, this, &MainWindow::loadImage);
    auto* btnLoadProject = new QPushButton("Загрузить проект", startWidget);
    layout->addWidget(btnLoadProject);
    connect(btnLoadProject, &QPushButton::clicked, this, &MainWindow::loadProject);
    startWidget->setFixedWidth(800);
    startWidget->setFixedHeight(600);
}

void MainWindow::setupScene()
{
    scene->setBackgroundBrush(Qt::lightGray);
    view->setRenderHint(QPainter::Antialiasing);
}

bool MainWindow::isWorkingMode() const
{
    return currentWorkMode == WorkMode::Work;
}

bool MainWindow::isModifyMode() const
{
    return currentWorkMode == WorkMode::Modify;
}

bool MainWindow::isCheckMode() const
{
    return currentWorkMode == WorkMode::Check;
}

void MainWindow::handleNewWorkMode(WorkMode mode)
{
    currentWorkMode = mode;
    emit modifyModStatusChanged(isModifyMode());
    emit workingModStatusChanged(isWorkingMode());
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

    modeBtn->setDefaultAction(actModify);
    actModify->setChecked(true);

    tb->addWidget(modeBtn);

    connect(this, &MainWindow::projectReady, modeBtn, &QToolButton::setEnabled);
    modeBtn->setEnabled(false);

    connect(group,
            &QActionGroup::triggered,
            this,
            [this, modeBtn](QAction* act)
            {
                modeBtn->setDefaultAction(act);
                const auto mode = static_cast<WorkMode>(act->data().toInt());
                /// TODO: Should be reworked
                switch (mode)
                {
                    case WorkMode::Check:
                    {
                        emit appEnterModeCheck();
                        break;
                    }
                    case WorkMode::Work:
                    {
                        emit appEnterModeRun();
                        break;
                    }
                    case WorkMode::Modify:
                    {
                        emit appEnterModeConfigure();
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
    connect(loadImgAction, &QAction::triggered, this, &MainWindow::loadImage);

    connect(this, &MainWindow::modifyModStatusChanged, loadImgAction, &QAction::setVisible);

    saveProjectAction = tb->addAction("Сохранить проект");
    connect(saveProjectAction, &QAction::triggered, this, &MainWindow::saveProject);
    connect(this, &MainWindow::modifyModStatusChanged, saveProjectAction, &QAction::setVisible);
    connect(this, &MainWindow::projectReady, saveProjectAction, &QAction::setEnabled);
    saveProjectAction->setEnabled(false);

    loadProjectAction = tb->addAction("Загрузить проект");
    connect(loadProjectAction, &QAction::triggered, this, &MainWindow::loadProject);
    connect(this, &MainWindow::modifyModStatusChanged, loadProjectAction, &QAction::setVisible);

    statusAction = tb->addAction("Pins: P1: , P2: , LEDs: ");
    statusAction->setCheckable(false);
    statusAction->setVisible(false);
    connect(this,
            &MainWindow::workModeChanged,
            [this](WorkMode mode) { statusAction->setVisible(mode == WorkMode::Check); });
}

void MainWindow::loadImage()
{
    QString path = QFileDialog::getOpenFileName(this, "Выбор изображения клавиатуры", {}, "Images (*.png *.jpg)");
    if (path.isEmpty())
    {
        return;
    }

    setBackgroundImage(QPixmap(path));
}

void MainWindow::enableSceneMode(bool enable)
{
    stackedWidget->setCurrentWidget(enable ? view : startWidget);
}

void MainWindow::setupMenus()
{
    comMenu = new ComPortMenu(this);
    comMenu->createMenu(QStringLiteral("COM Порт"));
    comMenu->addToMenuBar(menuBar());
    comMenu->setTestPortVisible(true, QStringLiteral("/tmp/ttyV1"));

    comMenu->setCurrentPort(QString{});

    connect(comMenu,
            &ComPortMenu::portSelected,
            this,
            [this](const QString& name)
            {
                qDebug() << "Selected COM port:" << (name.isEmpty() ? "None" : name);
                emit comPortSelected(name);
            });
    QMenu* versionMenu = menuBar()->addMenu("Версия ПО");
    versionMenu->addAction(QStringLiteral(APP_VERSION));
}

void MainWindow::updateStatus(uint8_t pin1, uint8_t pin2, const QVector<uint8_t>& leds)
{
    QString statusText      = QString("Pins: P1:%1, P2:%2, LEDs: ").arg(pin1).arg(pin2);
    int     countActiveLeds = 0;
    for (int i = 0; i < leds.size(); ++i)
    {
        if (!leds[i])
        {
            continue;
        }

        if (countActiveLeds > 0)
        {
            statusText.append(", ");
        }

        statusText.append(QString("L%1").arg(i + 1));
        countActiveLeds++;
    }
    statusAction->setText(statusText);

    emit updateButtonStatus(0, 0, false);

    emit updateButtonStatus(pin1, pin2, true);

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
    connect(diode, &DiodeItem::removeDiode, this, &MainWindow::deleteDiode);
}

void MainWindow::addResizableItem(ResizableRectItem* item)
{
    connect(this, &MainWindow::modifyModStatusChanged, item, &ResizableRectItem::setResizable);
    item->setResizable(isModifyMode());
}

void MainWindow::addButtonItem(ButtonItem* button)
{
    connect(button, &ButtonItem::buttonPressed, this, &MainWindow::appButtonPressed);
    connect(button, &ButtonItem::buttonReleased, this, &MainWindow::appButtonReleased);
    connect(this, &MainWindow::updateButtonStatus, button, &ButtonItem::onStatusUpdate);
    connect(this, &MainWindow::workingModStatusChanged, button, &ButtonItem::setClickable);
    addResizableItem(button);
    buttonItems.append(button);
    connect(button, &ButtonItem::removeButton, this, &MainWindow::deleteButton);
}

void MainWindow::deleteButton(ButtonItem* button)
{
    if (!button)
    {
        return;
    }

    scene->removeItem(button);
    buttonItems.removeOne(button);
    delete button;
}

void MainWindow::deleteDiode(DiodeItem* diode)
{
    if (!diode)
    {
        return;
    }

    scene->removeItem(diode);
    diodeItems.removeOne(diode);
    delete diode;
}

void MainWindow::saveProject()
{
    QString path = QFileDialog::getSaveFileName(this, "Сохранить проект", {}, "Keyboard Project (*.kbk)");

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
    QString path = QFileDialog::getOpenFileName(this, "Загрузить проект", {}, "Keyboard Project (*.kbk)");
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

    // Restore background
    setBackgroundImage(QPixmap::fromImage(project.background));

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

void MainWindow::clearItems()
{
    scene->clear();

    diodeItems.clear();
    buttonItems.clear();
}

void MainWindow::setBackgroundImage(const QPixmap& pixmap)
{
    clearItems();
    backgroundImage = pixmap;
    auto* pix       = new QGraphicsPixmapItem(backgroundImage);
    pix->setZValue(-1);
    scene->addItem(pix);

    emit projectReady(true);
}
