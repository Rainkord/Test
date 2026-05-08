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
    setWindowTitle(QString::fromUtf8("Проект"));
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
    // AuthWidget испускает showVerifyAuth — переходим к VerifyWidget.
    connect(authWidget, &AuthWidget::showVerifyAuth, this, &MainWindow::onShowVerifyAuth);
    connect(authWidget, &AuthWidget::showRegister,   this, &MainWindow::onShowRegister);
    connect(authWidget, &AuthWidget::showReset,      this, &MainWindow::onShowReset);

    connect(verifyWidget, &VerifyWidget::verificationSuccess, this, &MainWindow::onAuthSuccess);
    connect(verifyWidget, &VerifyWidget::backToAuth,          this, &MainWindow::onShowAuth);

    connect(regWidget, &RegWidget::registrationSuccess, this, &MainWindow::onShowMain);
    connect(regWidget, &RegWidget::showLogin,           this, &MainWindow::onShowAuth);

    connect(resetWidget, &ResetWidget::resetSuccess, this, &MainWindow::onResetSuccess);
    connect(resetWidget, &ResetWidget::backToLogin,  this, &MainWindow::onShowAuth);

    connect(mainAppWidget, &MainAppWidget::logout, this, &MainWindow::onLogout);
}

void MainWindow::onShowAuth()
{
    authWidget->clearFields();
    stack->setCurrentWidget(authWidget);
}

void MainWindow::onShowRegister()
{
    regWidget->clearAll();
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
    resetWidget->clearAll();
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
