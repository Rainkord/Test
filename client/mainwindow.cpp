
#include "mainwindow.h"
#include "clientsingleton.h"
#include "taskdialog.h"
#include "schemadialog.h"

#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QMessageBox>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QShowEvent>
#include <QCloseEvent>
#include <QIcon>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QDebug>

static const int IDX_AUTH  = 0;
static const int IDX_REG   = 1;
static const int IDX_GRAPH = 2;
static const int IDX_RESET = 3;
static const int IDX_VERIFY= 4;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("TIMP Client");
    setMinimumSize(960, 620);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *root = new QVBoxLayout(central);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    stackedWidget = new QStackedWidget(this);
    root->addWidget(stackedWidget);

    authWidget   = new AuthWidget(this);
    regWidget    = new RegWidget(this);
    graphWidget  = new GraphWidget(this);
    resetWidget  = new ResetWidget(this);
    verifyWidget = new VerifyWidget(this);

    stackedWidget->addWidget(authWidget);    // 0
    stackedWidget->addWidget(regWidget);     // 1
    stackedWidget->addWidget(graphWidget);   // 2
    stackedWidget->addWidget(resetWidget);   // 3
    stackedWidget->addWidget(verifyWidget);  // 4

    // ── Auth widget signals ────────────────────────────────────────────────
    connect(authWidget, &AuthWidget::showRegister,   this, &MainWindow::onShowRegister);
    connect(authWidget, &AuthWidget::loginSuccess,   this, &MainWindow::onLoginSuccess);
    connect(authWidget, &AuthWidget::showReset,      this, &MainWindow::onShowReset);

    // ── Reg widget signals ─────────────────────────────────────────────────
    connect(regWidget, &RegWidget::showAuth,          this, &MainWindow::onShowAuth);
    connect(regWidget, &RegWidget::registrationSuccess, this, &MainWindow::onRegistrationSuccess);

    // ── Reset widget signals ───────────────────────────────────────────────
    connect(resetWidget, &ResetWidget::showAuth,      this, &MainWindow::onShowAuth);
    connect(resetWidget, &ResetWidget::showVerify,    this, &MainWindow::onShowVerify);

    // ── Verify widget signals ──────────────────────────────────────────────
    connect(verifyWidget, &VerifyWidget::showAuth,    this, &MainWindow::onShowAuth);
    connect(verifyWidget, &VerifyWidget::verifySuccess, this, &MainWindow::onVerifySuccess);

    // ── Graph widget signals ───────────────────────────────────────────────
    connect(graphWidget, &GraphWidget::logout,        this, &MainWindow::onLogout);
    connect(graphWidget, &GraphWidget::showTask,      this, &MainWindow::onShowTask);
    connect(graphWidget, &GraphWidget::showSchema,    this, &MainWindow::onShowSchema);

    stackedWidget->setCurrentIndex(IDX_AUTH);

    // centre window
    QScreen *scr = QApplication::primaryScreen();
    if (scr) {
        QRect sg = scr->availableGeometry();
        move(sg.center() - rect().center());
    }
}

MainWindow::~MainWindow() {}

// ─────────────────────────────────────────────────────────────────────────────
// Slot implementations
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onShowRegister()
{
    regWidget->clearFields();
    stackedWidget->setCurrentIndex(IDX_REG);
}

void MainWindow::onShowAuth()
{
    authWidget->clearFields();
    stackedWidget->setCurrentIndex(IDX_AUTH);
}

void MainWindow::onShowReset()
{
    stackedWidget->setCurrentIndex(IDX_RESET);
}

void MainWindow::onShowVerify(const QString &email)
{
    verifyWidget->setEmail(email);
    stackedWidget->setCurrentIndex(IDX_VERIFY);
}

void MainWindow::onLogout()
{
    authWidget->clearFields();
    stackedWidget->setCurrentIndex(IDX_AUTH);
}

void MainWindow::onLoginSuccess(const QString &login)
{
    graphWidget->setLogin(login);
    stackedWidget->setCurrentIndex(IDX_GRAPH);
}

void MainWindow::onRegistrationSuccess(const QString &login)
{
    authWidget->clearFields();
    graphWidget->setLogin(login);
    stackedWidget->setCurrentIndex(IDX_GRAPH);
}

void MainWindow::onVerifySuccess(const QString &login)
{
    authWidget->clearFields();
    graphWidget->setLogin(login);
    stackedWidget->setCurrentIndex(IDX_GRAPH);
}

void MainWindow::onShowTask()
{
    TaskDialog dlg(this);
    dlg.exec();
}

void MainWindow::onShowSchema()
{
    SchemaDialog dlg(this);
    dlg.exec();
}
