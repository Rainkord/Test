#include "mainwindow.h"
#include "clientsingleton.h"

#include <QStackedWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("ТМП - Клиент (Подгруппа 5)");
    setMinimumSize(800, 600);

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    authWidget  = new AuthWidget(this);
    regWidget   = new RegWidget(this);
    graphWidget = new GraphWidget(this);

    stackedWidget->addWidget(authWidget);   // index 0
    stackedWidget->addWidget(regWidget);    // index 1
    stackedWidget->addWidget(graphWidget);  // index 2

    // Show auth screen first
    stackedWidget->setCurrentIndex(0);

    // Connections
    connect(authWidget, &AuthWidget::loginSuccess,
            this, &MainWindow::showGraph);

    connect(authWidget, &AuthWidget::showRegister,
            this, &MainWindow::showRegister);

    connect(regWidget, &RegWidget::registrationSuccess,
            this, &MainWindow::showAuth);

    connect(regWidget, &RegWidget::showAuth,
            this, &MainWindow::showAuth);

    connect(graphWidget, &GraphWidget::logout,
            this, &MainWindow::handleLogout);
}

void MainWindow::showGraph(const QString &login)
{
    graphWidget->setUserLogin(login);
    stackedWidget->setCurrentIndex(2);
}

void MainWindow::showAuth()
{
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::showRegister()
{
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::handleLogout()
{
    ClientSingleton::instance().disconnectFromServer();
    stackedWidget->setCurrentIndex(0);
}
