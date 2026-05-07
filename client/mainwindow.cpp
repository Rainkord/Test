#include "mainwindow.h"
#include "authwidget.h"
#include "regwidget.h"
#include "verifywidget.h"
#include "graphwidget.h"
#include "clientsingleton.h"

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QMessageBox>

static const int IDX_AUTH   = 0;
static const int IDX_REG    = 1;
static const int IDX_VERIFY = 2;
static const int IDX_GRAPH  = 3;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QString::fromUtf8("\xd0\x93\xd1\x80\xd0\xb0\xd1\x84\xd0\xb8\xd0\xba \xd1\x84\xd1\x83\xd0\xbd\xd0\xba\xd1\x86\xd0\xb8\xd0\xb8"));

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    authWidget   = new AuthWidget(this);
    regWidget    = new RegWidget(this);
    verifyWidget = new VerifyWidget(this);
    graphWidget  = new GraphWidget(this);

    stackedWidget->addWidget(authWidget);    // 0
    stackedWidget->addWidget(regWidget);     // 1
    stackedWidget->addWidget(verifyWidget);  // 2
    stackedWidget->addWidget(graphWidget);   // 3

    stackedWidget->setCurrentIndex(IDX_AUTH);

    // Auth
    connect(authWidget, &AuthWidget::loginSuccess, this, &MainWindow::onLoginSuccess);
    connect(authWidget, &AuthWidget::showRegister, this, [this]() {
        regWidget->clearFields();
        stackedWidget->setCurrentIndex(IDX_REG);
    });

    // Reg
    connect(regWidget, &RegWidget::registrationSuccess, this, &MainWindow::onRegistrationSuccess);
    connect(regWidget, &RegWidget::showLogin, this, [this]() {
        authWidget->clearFields();
        stackedWidget->setCurrentIndex(IDX_AUTH);
    });
    connect(regWidget, &RegWidget::verificationRequired,
            this, &MainWindow::onVerificationRequired);

    // Verify
    connect(verifyWidget, &VerifyWidget::verificationSuccess,
            this, &MainWindow::onVerificationSuccess);
    connect(verifyWidget, &VerifyWidget::backToRegister, this, [this]() {
        stackedWidget->setCurrentIndex(IDX_REG);
    });

    // Graph
    connect(graphWidget, &GraphWidget::logout, this, &MainWindow::onLogout);

    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect sg = screen->availableGeometry();
        resize(qMin(1280, sg.width()), qMin(800, sg.height()));
        move(sg.center() - rect().center());
    }
}

MainWindow::~MainWindow() {}

void MainWindow::onLoginSuccess(const QString &login)
{
    graphWidget->setUserLogin(login);
    graphWidget->updateGraph();
    stackedWidget->setCurrentIndex(IDX_GRAPH);
}

void MainWindow::onLogout()
{
    authWidget->clearFields();
    stackedWidget->setCurrentIndex(IDX_AUTH);
}

void MainWindow::onVerificationRequired(const QString &login)
{
    verifyWidget->setLogin(login);
    stackedWidget->setCurrentIndex(IDX_VERIFY);
}

void MainWindow::onVerificationSuccess(const QString &login)
{
    graphWidget->setUserLogin(login);
    graphWidget->updateGraph();
    stackedWidget->setCurrentIndex(IDX_GRAPH);
}

void MainWindow::onRegistrationSuccess(const QString &login)
{
    graphWidget->setUserLogin(login);
    graphWidget->updateGraph();
    stackedWidget->setCurrentIndex(IDX_GRAPH);
}
