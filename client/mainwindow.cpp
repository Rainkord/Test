#include "mainwindow.h"

#include "authwidget.h"
#include "regwidget.h"
#include "verifywidget.h"
#include "resetwidget.h"
#include "mainappwidget.h"

#include <QStackedWidget>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QString::fromUtf8("\u041f\u0440\u043e\u0435\u043a\u0442"));
    resize(480, 600);

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    authWidget    = new AuthWidget(this);
    regWidget     = new RegWidget(this);
    verifyWidget  = new VerifyWidget(this);
    resetWidget   = new ResetWidget(this);
    mainAppWidget = new MainAppWidget(this);

    stack->addWidget(authWidget);
    stack->addWidget(regWidget);
    stack->addWidget(verifyWidget);
    stack->addWidget(resetWidget);
    stack->addWidget(mainAppWidget);

    setupConnections();
    stack->setCurrentWidget(authWidget);
}

MainWindow::~MainWindow() {}

void MainWindow::setupConnections()
{
    // AuthWidget
    connect(authWidget, &AuthWidget::showVerifyAuth, this, &MainWindow::onShowVerifyAuth);
    connect(authWidget, &AuthWidget::showRegister,   this, &MainWindow::onShowRegister);
    connect(authWidget, &AuthWidget::showReset,      this, &MainWindow::onShowReset);

    // VerifyWidget
    connect(verifyWidget, &VerifyWidget::verificationSuccess, this, &MainWindow::onAuthSuccess);
    connect(verifyWidget, &VerifyWidget::backToAuth,          this, &MainWindow::onShowAuth);

    // RegWidget
    // NOTE: signal name is showAuth (not showLogin)
    connect(regWidget, &RegWidget::registrationSuccess, this, &MainWindow::onShowMain);
    connect(regWidget, &RegWidget::showAuth,            this, &MainWindow::onShowAuth);

    // ResetWidget
    // NOTE: signal name is backToAuth (not backToLogin)
    connect(resetWidget, &ResetWidget::resetSuccess, this, &MainWindow::onResetSuccess);
    connect(resetWidget, &ResetWidget::backToAuth,   this, &MainWindow::onShowAuth);

    // MainAppWidget
    connect(mainAppWidget, &MainAppWidget::logout, this, &MainWindow::onLogout);
}

void MainWindow::onShowAuth()
{
    authWidget->clearFields();
    stack->setCurrentWidget(authWidget);
}

void MainWindow::onShowRegister()
{
    // NOTE: method is clearFields() (not clearAll())
    regWidget->clearFields();
    stack->setCurrentWidget(regWidget);
}

void MainWindow::onShowVerifyAuth(const QString &login, const QString &codeHash)
{
    verifyWidget->setLogin(login, codeHash);
    stack->setCurrentWidget(verifyWidget);
}

void MainWindow::onAuthSuccess(const QString &login)
{
    onShowMain(login);
}

void MainWindow::onShowMain(const QString &login)
{
    mainAppWidget->setLogin(login);
    stack->setCurrentWidget(mainAppWidget);
}

void MainWindow::onShowReset()
{
    // NOTE: ResetWidget has no clearAll(), reset state is in constructor defaults.
    // Call a manual reset by showing fresh widget state.
    stack->setCurrentWidget(resetWidget);
}

void MainWindow::onResetSuccess()
{
    authWidget->clearFields();
    stack->setCurrentWidget(authWidget);
}

void MainWindow::onLogout()
{
    authWidget->clearFields();
    stack->setCurrentWidget(authWidget);
}
