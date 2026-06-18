#include "mainwindow.h"
#include <QCoreApplication>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>

namespace QeriPlayerQt {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupCentralWidget();
}

MainWindow::~MainWindow() { }

void MainWindow::setupUi()
{
    setWindowTitle(tr("QeriPlayer Qt"));
    resize(800, 600);
}

void MainWindow::setupMenuBar()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    // Exit action
    QAction *exitAction = fileMenu->addAction(tr("E&xit"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::on_actionExit_triggered);

    // Help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    // About action
    QAction *aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QString version = QCoreApplication::applicationVersion();
        QMessageBox::about(this, tr("About QeriPlayer Qt"),
                           tr("QeriPlayer Qt Desktop Client\nVersion %1").arg(version));
    });
}

void MainWindow::setupToolBar()
{
    QToolBar *mainToolBar = addToolBar(tr("Main Toolbar"));
    mainToolBar->setMovable(false);

    // Add some placeholder actions
    QAction *playAction = mainToolBar->addAction(tr("Play"));
    QAction *pauseAction = mainToolBar->addAction(tr("Pause"));
    QAction *stopAction = mainToolBar->addAction(tr("Stop"));

    // Connect actions to slots (placeholder implementations)
    connect(playAction, &QAction::triggered, this, [this]() { statusBar()->showMessage(tr("Playing..."), 2000); });

    connect(pauseAction, &QAction::triggered, this, [this]() { statusBar()->showMessage(tr("Paused"), 2000); });

    connect(stopAction, &QAction::triggered, this, [this]() { statusBar()->showMessage(tr("Stopped"), 2000); });
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::setupCentralWidget()
{
    // Create central widget with placeholder content
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    QLabel *welcomeLabel = new QLabel(tr("Welcome to QeriPlayer Qt Desktop Client"), centralWidget);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");

    QLabel *versionLabel
        = new QLabel(tr("Version %1 - Under Development").arg(QCoreApplication::applicationVersion()), centralWidget);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet("font-size: 14px; color: #666;");

    layout->addStretch();
    layout->addWidget(welcomeLabel);
    layout->addWidget(versionLabel);
    layout->addStretch();

    setCentralWidget(centralWidget);
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

} // namespace QeriPlayerQt