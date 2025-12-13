#include "MainWindow.h"

#include <QAction>
#include <QCursor>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QGuiApplication>
#include <QMenu>
#include <QMenuBar>
#include <QScreen>
#include <QSerialPortInfo>
#include <QWindow>

#include "ComPortMenu.h"
#include "ImageZoomWidget.h"
#include "ProjectIO.h"
#include "QtFileDialogService.h"
#include "QtMessageService.h"
#include "StartScreenWidget.h"
#include "WorkModeState.h"
#include "WorkModeToolbar.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , scene(new CustomScene(this))
    , view(new QGraphicsView(scene, this))
    , stackedWidget(new QStackedWidget(this))
    , startScreen(new StartScreenWidget(this))
{
    setWindowTitle("Эмулятор клавиатуры");

    setupScene();
    setupMenus();

    workModeUi     = new WorkModeToolbar(this, this);
    workModeState  = new WorkModeState(this);
    fileDialogs    = std::make_unique<QtFileDialogService>(this);
    messageService = std::make_unique<QtMessageService>(this);

    connect(scene, &CustomScene::diodeAdded, this, &MainWindow::addDiodeItem);
    connect(scene, &CustomScene::buttonAdded, this, &MainWindow::addButtonItem);
    connect(scene, &CustomScene::pasteItem, this, &MainWindow::pasteItem);

    connect(this, &MainWindow::modifyModStatusChanged, scene, &CustomScene::setModifiable);

    createImageViewer();

    stackedWidget->addWidget(startScreen);
    stackedWidget->addWidget(imageViewer);
    stackedWidget->addWidget(view);
    stackedWidget->setCurrentWidget(startScreen);
    setCentralWidget(stackedWidget);

    connect(startScreen, &StartScreenWidget::loadImageRequested, this, &MainWindow::loadImage);
    connect(startScreen, &StartScreenWidget::loadProjectRequested, this, &MainWindow::loadProject);
    connect(startScreen, &StartScreenWidget::recentItemActivated, this, &MainWindow::handleRecentProjectRequested);
    connect(startScreen, &StartScreenWidget::clearRecentRequested, this, &MainWindow::handleClearRecentRequested);

    connect(workModeUi,
            &WorkModeToolbar::workModeSelected,
            this,
            [this](WorkMode mode)
            {
                if (workModeState)
                {
                    workModeState->setMode(mode);
                }
            });
    connect(workModeUi, &WorkModeToolbar::loadImageRequested, this, &MainWindow::loadImage);
    connect(workModeUi, &WorkModeToolbar::saveProjectRequested, this, &MainWindow::saveProject);
    connect(workModeUi, &WorkModeToolbar::loadProjectRequested, this, &MainWindow::loadProject);
    connect(this, &MainWindow::projectReady, workModeUi, &WorkModeToolbar::setProjectReady);

    if (workModeState)
    {
        connect(workModeState,
                &WorkModeState::modeChanged,
                this,
                [this](WorkMode mode)
                {
                    if (workModeUi)
                    {
                        workModeUi->setCurrentMode(mode);
                    }
                    emit workModeChanged(mode);
                });
        connect(workModeState, &WorkModeState::modifyModeChanged, this, &MainWindow::modifyModStatusChanged);
        connect(workModeState, &WorkModeState::workModeChanged, this, &MainWindow::workingModStatusChanged);
        connect(workModeState, &WorkModeState::checkModeChanged, this, &MainWindow::checkModStatusChanged);

        workModeState->setMode(WorkMode::Modify);
    }

    refreshRecentProjects();

    connect(this, &MainWindow::projectReady, &MainWindow::enableSceneMode);
}

void MainWindow::refreshRecentProjects()
{
    if (!startScreen)
    {
        return;
    }

    startScreen->setRecentEntries(m_recent.list());
}

void MainWindow::handleRecentProjectRequested(const QString& path)
{
    if (path.isEmpty())
    {
        return;
    }

    if (!QFileInfo::exists(path))
    {
        if (messageService)
        {
            messageService->showWarning(this, tr("Файл не найден"), tr("Файл не существует:\n%1").arg(path));
        }
        m_recent.remove(path);
        refreshRecentProjects();
        return;
    }

    const bool ok = loadProjectFromPath(path);
    if (ok)
    {
        m_recent.add(path);
        refreshRecentProjects();
        return;
    }

    if (messageService)
    {
        messageService->showWarning(this, tr("Ошибка открытия"), tr("Не удалось открыть проект:\n%1").arg(path));
    }
    m_recent.remove(path);
    refreshRecentProjects();
}

void MainWindow::handleClearRecentRequested()
{
    if (m_recent.list().isEmpty())
    {
        return;
    }

    if (messageService)
    {
        const bool confirmed =
            messageService->confirmQuestion(this, tr("Очистить список"), tr("Очистить список последних проектов?"));
        if (!confirmed)
        {
            return;
        }
    }

    m_recent.clear();
    refreshRecentProjects();
}
void MainWindow::createImageViewer()
{
    if (!imageViewer)
    {
        imageViewer = new ImageZoomWidget;
        imageViewer->setZoomLimits(0.1, 10.0);
        imageViewer->setZoomStep(1.15);
    }

    connect(imageViewer,
            &ImageZoomWidget::zoomReady,
            this,
            [this]() { setBackgroundImage(imageViewer->getResultPixmap()); });
}

void MainWindow::setupScene()
{
    scene->setBackgroundBrush(Qt::lightGray);
    view->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::loadImage()
{
    if (!fileDialogs)
    {
        return;
    }

    const QString path =
        fileDialogs->openFile(this, tr("Выбор изображения клавиатуры"), QString(), tr("Images (*.png *.jpg)"));
    if (path.isEmpty())
    {
        return;
    }

    if (!imageViewer)
    {
        return;
    }

    imageViewer->setImage(QPixmap(path));
    stackedWidget->setCurrentWidget(imageViewer);
}

void MainWindow::enableSceneMode(bool enable)
{
    QWidget* target = enable ? static_cast<QWidget*>(view) : static_cast<QWidget*>(startScreen);
    stackedWidget->setCurrentWidget(target);
}

void MainWindow::copyItem(ResizableRectItem* item)
{
    copiedItem = item ? item->clone() : nullptr;
    scene->setPasteEnabled(copiedItem != nullptr);
}

void MainWindow::pasteItem(QPointF pos)
{
    if (!copiedItem)
    {
        return;
    }

    auto* newItem = copiedItem->clone();
    // TODO: Improve position
    newItem->setPos(newItem->pos() + QPointF(10, 10));

    if (DiodeItem* diode = dynamic_cast<DiodeItem*>(newItem))
    {
        addDiodeItem(diode);
        scene->addItem(diode);
        return;
    }

    if (ButtonItem* button = dynamic_cast<ButtonItem*>(newItem))
    {
        addButtonItem(button);
        scene->addItem(button);
        return;
    }

    delete newItem;
    return;
}

void MainWindow::setupMenus()
{
    comPortMenu = new ComPortMenu(this, this);
    connect(comPortMenu, &ComPortMenu::refreshRequested, this, &MainWindow::refreshComPortList);

    QMenu* versionMenu = menuBar()->addMenu("Версия ПО");
    versionMenu->addAction(QStringLiteral(APP_VERSION));
}

void MainWindow::updateStatus(Pins pins, const QVector<Pins>& leds)
{
    const WorkMode mode = currentMode();
    if (mode == WorkMode::Modify || mode == WorkMode::DiodeConf)
    {
        return;
    }

    if (mode == WorkMode::Work)
    {
        emit clearDiodeStatus();
        for (const auto& led : leds)
        {
            emit updateDiodeStatus(led);
        }

        return;
    }

    if (mode == WorkMode::Check)
    {
        emit    clearButtonStatus();
        QString statusText      = QString("Pins: P1:%1, P2:%2, LEDs: ").arg(pins.pin1).arg(pins.pin2);
        int     countActiveLeds = 0;
        for (const auto& led : leds)
        {
            if (countActiveLeds > 0)
            {
                statusText.append(", ");
            }

            statusText.append(QString("A%1:K%2").arg(led.pin1).arg(led.pin2));
            countActiveLeds++;
        }
        if (workModeUi)
        {
            workModeUi->setStatusText(statusText);
        }

        emit updateButtonStatus(pins, true);
    }
}

void MainWindow::addDiodeItem(DiodeItem* diode)
{
    connect(diode,
            &DiodeItem::buttonPressed,
            this,
            [this](Pins pins) { emit appExecuteCommand(Command::DiodePressed, pins); });
    connect(diode,
            &DiodeItem::buttonReleased,
            this,
            [this](Pins pins) { emit appExecuteCommand(Command::DiodeReleased, pins); });

    connect(this, &MainWindow::updateDiodeStatus, diode, &DiodeItem::onStatusUpdate);
    connect(this, &MainWindow::clearDiodeStatus, diode, &DiodeItem::clearStatus);
    connect(this, &MainWindow::checkModStatusChanged, diode, &DiodeItem::setClickable);

    connect(this,
            &MainWindow::workModeChanged,
            diode,
            [diode](WorkMode mode)
            {
                const bool show = mode == WorkMode::Modify || mode == WorkMode::Check;
                diode->setShowExtendedMenu(show);
            });

    connect(diode, &DiodeItem::pinsChanged, this, &MainWindow::updatePinStatus);

    diodeItems.append(diode);
    addResizableItem(diode);
    connect(diode, &DiodeItem::removeItem, this, &MainWindow::deleteItem);
}

void MainWindow::addResizableItem(ResizableRectItem* item)
{
    connect(this, &MainWindow::modifyModStatusChanged, item, &ResizableRectItem::setResizable);
    connect(item, &ResizableRectItem::itemCopied, this, &MainWindow::copyItem);
    item->setResizable(currentMode() == WorkMode::Modify);
}

void MainWindow::updatePinStatus(AbstractItem* item)
{
    if (!item)
    {
        return;
    }

    emit appExecuteCommand(Command::ModeDiodeConfig, {item->getPin1(), item->getPin2()});
}

void MainWindow::updateComPort(const QString& portName)
{
    if (comPortMenu)
    {
        comPortMenu->setStatusText(tr("Соединение: %1").arg(portName));
    }
}

void MainWindow::addButtonItem(ButtonItem* button)
{
    connect(button,
            &ButtonItem::buttonPressed,
            this,
            [this](Pins pins) { emit appExecuteCommand(Command::ButtonPressed, pins); });
    connect(button,
            &ButtonItem::buttonReleased,
            this,
            [this](Pins pins) { emit appExecuteCommand(Command::ButtonReleased, pins); });
    connect(this, &MainWindow::updateButtonStatus, button, &ButtonItem::onStatusUpdate);
    connect(this, &MainWindow::clearButtonStatus, button, &ButtonItem::clearStatus);
    connect(this, &MainWindow::workingModStatusChanged, button, &ButtonItem::setClickable);

    connect(this,
            &MainWindow::workModeChanged,
            button,
            [button](WorkMode mode)
            {
                const bool show = mode == WorkMode::Modify || mode == WorkMode::Check;
                button->setShowExtendedMenu(show);
            });

    addResizableItem(button);
    buttonItems.append(button);
    connect(button, &ButtonItem::removeItem, this, &MainWindow::deleteItem);
}

void MainWindow::deleteItem(AbstractItem* item)
{
    if (!item)
    {
        return;
    }

    if (DiodeItem* diode = dynamic_cast<DiodeItem*>(item))
    {
        deleteDiode(diode);
        return;
    }

    if (ButtonItem* button = dynamic_cast<ButtonItem*>(item))
    {
        deleteButton(button);
        return;
    }
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

    emit appExecuteCommand(Command::ModeDiodeConfigDel, {diode->getPin1(), diode->getPin2()});

    scene->removeItem(diode);
    diodeItems.removeOne(diode);
    delete diode;
}

void MainWindow::saveProject()
{
    QString dir = m_recent.lastOpenDir();

    const QString startDir = dir.isEmpty() ? QDir::homePath() : dir;
    QString       path     = fileDialogs
                                 ? fileDialogs->saveFile(this, tr("Сохранить проект"), startDir, tr("Keyboard Project (*.kbk)"))
                                 : QString();

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
        if (messageService)
        {
            messageService->showWarning(
                this, tr("Ошибка сохранения"), tr("Не удалось сохранить проект:\n%1").arg(path));
        }
        return;
    }

    qDebug() << "Project saved successfully";
    m_recent.add(path);
    refreshRecentProjects();
}

void MainWindow::loadProject()
{
    QString dir = m_recent.lastOpenDir();

    const QString startDir = dir.isEmpty() ? QDir::homePath() : dir;
    QString       path     = fileDialogs
                                 ? fileDialogs->openFile(this, tr("Загрузить проект"), startDir, tr("Keyboard Project (*.kbk)"))
                                 : QString();

    if (path.isEmpty())
    {
        return;
    }

    loadProjectFromPath(path);
}

bool MainWindow::loadProjectFromPath(const QString& path)
{
    if (path.isEmpty())
    {
        return false;
    }

    Project project;
    if (!ProjectIO::load(path, project))
    {
        qDebug() << "Failed to load project";
        if (messageService)
        {
            messageService->showWarning(this, tr("Ошибка загрузки"), tr("Не удалось загрузить проект:\n%1").arg(path));
        }
        return false;
    }

    // Restore background
    setBackgroundImage(QPixmap::fromImage(project.background));

    emit appExecuteCommand(Command::ModeDiodeClear, {0, 0});

    // Restore LEDs
    for (const auto& ledDef : project.leds)
    {
        auto* diode = new DiodeItem(ledDef);
        scene->addItem(diode);
        addDiodeItem(diode);
        emit appExecuteCommand(Command::ModeDiodeConfig, {diode->getPin1(), diode->getPin2()});
    }

    // Restore buttons
    for (const auto& buttonDef : project.buttons)
    {
        auto* button = new ButtonItem(buttonDef);
        scene->addItem(button);
        addButtonItem(button);
    }

    qDebug() << "Project loaded successfully";
    m_recent.add(path);
    refreshRecentProjects();
    return true;
}

void MainWindow::clearItems()
{
    scene->clear();

    diodeItems.clear();
    buttonItems.clear();

    copiedItem = nullptr;
}

void MainWindow::setBackgroundImage(const QPixmap& pixmap)
{
    // 1) Prepare scene
    clearItems();
    backgroundImage = pixmap;
    auto* pix       = new QGraphicsPixmapItem(backgroundImage);
    pix->setZValue(-1);
    scene->addItem(pix);

    // 2) Adjust window size to fit image
    const QSize kMinWin(400, 300);
    const int   kPadding = 35; // Padding around the image

    // Calculate logical image size considering device pixel ratio
    auto logicalImageSize = [&]
    {
        QSize       px  = backgroundImage.size();             // пиксели
        const qreal dpr = backgroundImage.devicePixelRatio(); // 1, 2, ...
        if (dpr > 0.0)
        {
            return QSize(qRound(px.width() / dpr), qRound(px.height() / dpr));
        }
        return px;
    }();

    QSize desired = logicalImageSize + QSize(kPadding, kPadding);
    desired       = desired.expandedTo(kMinWin);

    // 3) Get available screen size
    QScreen* screen = (this->windowHandle() ? this->windowHandle()->screen() : nullptr);
    if (!screen)
    {
        screen = QGuiApplication::screenAt(QCursor::pos());
    }
    if (!screen)
    {
        screen = QGuiApplication::primaryScreen();
    }

    const QRect avail = screen ? screen->availableGeometry() : QRect(QPoint(), QSize(1920, 1080));
    // 4) Calculate decoration size
    QSize deco(0, 0);
    if (isVisible())
    {
        deco.setWidth(frameGeometry().width() - geometry().width());
        deco.setHeight(frameGeometry().height() - geometry().height());
    }
    // 5) Calculate maximum client size
    QSize maxClient = avail.size() - deco;
    maxClient.setWidth(qMax(100, maxClient.width()));
    maxClient.setHeight(qMax(100, maxClient.height()));

    const QSize target = desired.boundedTo(maxClient);

    // 6) Apply size and center window
    setMinimumSize(kMinWin);
    resize(target);
    move(avail.center() - QPoint(width() / 2, height() / 2));

    emit projectReady(true);
}

WorkMode MainWindow::currentMode() const
{
    return workModeState ? workModeState->currentMode() : WorkMode::Modify;
}
