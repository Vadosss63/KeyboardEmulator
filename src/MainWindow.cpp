#include "MainWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QCursor>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QGraphicsPixmapItem>
#include <QGuiApplication>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScreen>
#include <QSerialPortInfo>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWindow>

#include "ImageZoomWidget.h"
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
    connect(scene, &CustomScene::pasteItem, this, &MainWindow::pasteItem);

    connect(this, &MainWindow::workModeChanged, this, &MainWindow::handleNewWorkMode);
    connect(this, &MainWindow::modifyModStatusChanged, scene, &CustomScene::setModifiable);

    emit workModeChanged(WorkMode::Modify);
    createStartWidget();
    createImageViewer();

    stackedWidget->addWidget(startWidget);
    stackedWidget->addWidget(imageViewer);
    stackedWidget->addWidget(view);
    stackedWidget->setCurrentWidget(startWidget);
    setCentralWidget(stackedWidget);

    connect(this, &MainWindow::projectReady, &MainWindow::enableSceneMode);
}

void MainWindow::createStartWidget()
{
    auto* root = new QVBoxLayout(startWidget);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    auto* column = new QWidget(startWidget);
    auto* colLay = new QVBoxLayout(column);
    colLay->setContentsMargins(0, 0, 0, 0);
    colLay->setSpacing(8);
    root->addWidget(column, 0, Qt::AlignHCenter | Qt::AlignTop);

    auto* label = new QLabel(tr("Выберите изображение или загрузите проект:"), column);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    colLay->addWidget(label);

    const int SIDE_PAD = 24;
    const int colWidth = QFontMetrics(label->font()).horizontalAdvance(label->text()) + SIDE_PAD;

    auto* btnLoadImage   = new QPushButton(tr("Загрузить изображение"), column);
    auto* btnLoadProject = new QPushButton(tr("Загрузить проект"), column);
    btnLoadImage->setFixedWidth(colWidth);
    btnLoadProject->setFixedWidth(colWidth);

    colLay->addWidget(btnLoadImage, 0, Qt::AlignLeft);
    colLay->addWidget(btnLoadProject, 0, Qt::AlignLeft);

    connect(btnLoadImage, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(btnLoadProject, &QPushButton::clicked, this, &MainWindow::loadProject);

    auto* sep = new QFrame(column);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    colLay->addWidget(sep);

    auto* recentLabel = new QLabel(tr("Последние проекты:"), column);
    recentLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    colLay->addWidget(recentLabel);

    recentList = new QListWidget(column);
    recentList->setFixedWidth(colWidth);
    recentList->setUniformItemSizes(true);
    recentList->setAlternatingRowColors(false);
    recentList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    colLay->addWidget(recentList, 0, Qt::AlignLeft);

    connect(recentList, &QListWidget::itemActivated, this, &MainWindow::openRecentItem);

    auto* row = new QHBoxLayout;
    row->addStretch();
    auto* btnClear = new QPushButton(tr("Очистить список"), column);
    row->addWidget(btnClear);
    colLay->addLayout(row);

    connect(btnClear, &QPushButton::clicked, this, &MainWindow::clearRecentList);

    updateRecentListWidget();
    resizeRecentListToContents();

    startWidget->setMinimumSize(column->sizeHint().expandedTo(QSize(colWidth + 24, 0)));
}

void MainWindow::resizeRecentListToContents()
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

void MainWindow::openRecentItem(QListWidgetItem* item)
{
    if (!item)
    {
        return;
    }

    const QString path = item->data(Qt::UserRole).toString();
    if (path.isEmpty())
    {
        return;
    }

    if (!QFileInfo::exists(path))
    {
        QMessageBox::warning(this, tr("Файл не найден"), tr("Файл не существует:\n%1").arg(path));
        m_recent.remove(path);
        updateRecentListWidget();
        return;
    }

    const bool ok = loadProjectFromPath(path);

    if (ok)
    {
        m_recent.add(path);
        updateRecentListWidget();
        return;
    }

    QMessageBox::warning(this, tr("Ошибка открытия"), tr("Не удалось открыть проект:\n%1").arg(path));
    m_recent.remove(path);
    updateRecentListWidget();
}

void MainWindow::clearRecentList()
{
    if (!recentList || recentList->count() == 0)
    {
        return;
    }

    const auto reply = QMessageBox::question(this,
                                             tr("Очистить список"),
                                             tr("Очистить список последних проектов?"),
                                             QMessageBox::Yes | QMessageBox::No,
                                             QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    m_recent.clear();
    updateRecentListWidget();
}

void MainWindow::updateRecentListWidget()
{
    if (!recentList)
    {
        return;
    }

    recentList->clear();
    const QStringList items = m_recent.list();
    for (const QString& p : items)
    {
        QFileInfo fi(p);
        auto*     it = new QListWidgetItem(fi.fileName(), recentList);
        it->setToolTip(p);
        it->setData(Qt::UserRole, p);
        recentList->addItem(it);
    }
    recentList->setEnabled(!items.isEmpty());
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
    emit checkModStatusChanged(isCheckMode());
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
                emit       workModeChanged(mode);
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

    if (!imageViewer)
    {
        return;
    }

    imageViewer->setImage(QPixmap(path));
    stackedWidget->setCurrentWidget(imageViewer);
}

void MainWindow::enableSceneMode(bool enable)
{
    stackedWidget->setCurrentWidget(enable ? view : startWidget);
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
    QMenu* comMenu = new QMenu("COM Порт", this);
    currentComPort = comMenu->addAction("Текущий порт: None");
    comMenu->addAction(currentComPort);
    comMenu->addSeparator();
    QAction* refreshAction = comMenu->addAction("Поиск устройства");
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshComPortList);
    comMenu->addAction(refreshAction);
    menuBar()->addMenu(comMenu);

    QMenu* versionMenu = menuBar()->addMenu("Версия ПО");
    versionMenu->addAction(QStringLiteral(APP_VERSION));
}

void MainWindow::updateStatus(Pins pins, const QVector<Pins>& leds)
{
    if (currentWorkMode == WorkMode::Modify || currentWorkMode == WorkMode::DiodeConf)
    {
        return;
    }

    if (currentWorkMode == WorkMode::Work)
    {
        clearDiodeStatus();
        for (const auto& led : leds)
        {
            emit updateDiodeStatus(led);
        }

        return;
    }

    if (currentWorkMode == WorkMode::Check)
    {
        clearButtonStatus();
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
        statusAction->setText(statusText);

        emit updateButtonStatus(pins, true);
    }
}

void MainWindow::addDiodeItem(DiodeItem* diode)
{
    connect(
        diode, &DiodeItem::buttonPressed, [this](Pins pins) { emit appExecuteCommand(Command::DiodePressed, pins); });
    connect(
        diode, &DiodeItem::buttonReleased, [this](Pins pins) { emit appExecuteCommand(Command::DiodeReleased, pins); });

    connect(this, &MainWindow::updateDiodeStatus, diode, &DiodeItem::onStatusUpdate);
    connect(this, &MainWindow::clearDiodeStatus, diode, &DiodeItem::clearStatus);
    connect(this, &MainWindow::checkModStatusChanged, diode, &DiodeItem::setClickable);

    connect(this,
            &MainWindow::workModeChanged,
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
    item->setResizable(isModifyMode());
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
    currentComPort->setText("Соединение: " + portName);
}

void MainWindow::addButtonItem(ButtonItem* button)
{
    connect(button,
            &ButtonItem::buttonPressed,
            [this](Pins pins) { emit appExecuteCommand(Command::ButtonPressed, pins); });
    connect(button,
            &ButtonItem::buttonReleased,
            [this](Pins pins) { emit appExecuteCommand(Command::ButtonReleased, pins); });
    connect(this, &MainWindow::updateButtonStatus, button, &ButtonItem::onStatusUpdate);
    connect(this, &MainWindow::clearButtonStatus, button, &ButtonItem::clearStatus);
    connect(this, &MainWindow::workingModStatusChanged, button, &ButtonItem::setClickable);

    connect(this,
            &MainWindow::workModeChanged,
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

    appExecuteCommand(Command::ModeDiodeConfigDel, {diode->getPin1(), diode->getPin2()});

    scene->removeItem(diode);
    diodeItems.removeOne(diode);
    delete diode;
}

void MainWindow::saveProject()
{
    QString dir = m_recent.lastOpenDir();

    QString path = QFileDialog::getSaveFileName(
        this, "Сохранить проект", dir.isEmpty() ? QDir::homePath() : dir, "Keyboard Project (*.kbk)");

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
    m_recent.add(path);
    updateRecentListWidget();
}

void MainWindow::loadProject()
{
    QString dir = m_recent.lastOpenDir();

    QString path = QFileDialog::getOpenFileName(
        this, "Загрузить проект", dir.isEmpty() ? QDir::homePath() : dir, "Keyboard Project (*.kbk)");

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
    updateRecentListWidget();
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
