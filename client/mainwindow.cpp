#include "mainwindow.h"
#include "taskdialog.h"
#include "schemadialog.h"

#include <QStyle>
#include <QApplication>
#include <QSpacerItem>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    connectSignals();
    stackedWidget->setCurrentIndex(IDX_AUTH);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    setWindowTitle("ТМП — Подгруппа 5");
    setMinimumSize(1000, 700);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainVLayout = new QVBoxLayout(centralWidget);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->setSpacing(0);

    QWidget *topBar = new QWidget(centralWidget);
    topBar->setFixedHeight(40);
    topBar->setStyleSheet(
        "QWidget { background-color: #2c3e50; }"
        "QPushButton { background-color: #34495e; color: white; border: none; "
        "border-radius: 4px; padding: 4px 10px; font-size: 11px; }"
        "QPushButton:hover { background-color: #4a6278; }"
        "QPushButton:pressed { background-color: #1a252f; }"
    );

    topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(8, 4, 8, 4);
    topBarLayout->setSpacing(6);

    taskBtn = new QPushButton(topBar);
    taskBtn->setIcon(style()->standardIcon(QStyle::SP_FileDialogInfoView));
    taskBtn->setText("  Задание");
    taskBtn->setToolTip("Показать задание");
    taskBtn->setMinimumWidth(100);

    schemaBtn = new QPushButton(topBar);
    schemaBtn->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
    schemaBtn->setText("  Блок-схема");
    schemaBtn->setToolTip("Показать блок-схему");
    schemaBtn->setMinimumWidth(110);

    appTitleLabel = new QLabel("ТМП — Подгруппа 5", topBar);
    appTitleLabel->setStyleSheet(
        "QLabel { color: #ecf0f1; font-size: 12px; font-weight: bold; background: transparent; }"
    );
    appTitleLabel->setAlignment(Qt::AlignCenter);

    topBarLayout->addWidget(taskBtn);
    topBarLayout->addWidget(schemaBtn);
    topBarLayout->addStretch(1);
    topBarLayout->addWidget(appTitleLabel);
    topBarLayout->addStretch(1);

    QFrame *separator = new QFrame(centralWidget);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setStyleSheet("QFrame { color: #1a252f; }");
    separator->setFixedHeight(1);

    stackedWidget = new QStackedWidget(centralWidget);

    authWidget   = new AuthWidget(stackedWidget);
    regWidget    = new RegWidget(stackedWidget);
    verifyWidget = new VerifyWidget(stackedWidget);
    graphWidget  = new GraphWidget(stackedWidget);
    resetWidget  = new ResetWidget(stackedWidget);

    stackedWidget->addWidget(authWidget);    // 0
    stackedWidget->addWidget(regWidget);     // 1
    stackedWidget->addWidget(verifyWidget);  // 2
    stackedWidget->addWidget(graphWidget);   // 3
    stackedWidget->addWidget(resetWidget);   // 4

    mainVLayout->addWidget(topBar);
    mainVLayout->addWidget(separator);
    mainVLayout->addWidget(stackedWidget, 1);

    centralWidget->setLayout(mainVLayout);
}

void MainWindow::connectSignals()
{
    connect(taskBtn,   &QPushButton::clicked, this, &MainWindow::onTaskBtnClicked);
    connect(schemaBtn, &QPushButton::clicked, this, &MainWindow::onSchemaBtnClicked);

    connect(authWidget, &AuthWidget::showVerifyAuth,  this, &MainWindow::onShowVerifyAuth);
    connect(authWidget, &AuthWidget::showRegister,    this, &MainWindow::onShowRegister);
    connect(authWidget, &AuthWidget::showReset,       this, &MainWindow::onShowReset);
    connect(authWidget, &AuthWidget::loginSuccess,
            this, [this](const QString &login) {
                graphWidget->setUserLogin(login);
                graphWidget->updateGraph();
                stackedWidget->setCurrentIndex(IDX_GRAPH);
            });

    connect(regWidget, &RegWidget::registrationSuccess, this, &MainWindow::onRegistrationSuccess);
    connect(regWidget, &RegWidget::showAuth,            this, &MainWindow::onShowAuth);

    connect(verifyWidget, &VerifyWidget::verificationSuccess, this, &MainWindow::onVerificationSuccess);
    connect(verifyWidget, &VerifyWidget::backToAuth,          this, &MainWindow::onBackToAuth);

    connect(resetWidget, &ResetWidget::backToAuth,   this, &MainWindow::onShowAuth);
    connect(resetWidget, &ResetWidget::resetSuccess, this, &MainWindow::onResetSuccess);

    connect(graphWidget, &GraphWidget::logout, this, &MainWindow::onLogout);
}

void MainWindow::onShowRegister()  { stackedWidget->setCurrentIndex(IDX_REG);    }
void MainWindow::onShowAuth()      { stackedWidget->setCurrentIndex(IDX_AUTH);   }
void MainWindow::onShowReset()     { stackedWidget->setCurrentIndex(IDX_RESET);  }

void MainWindow::onShowVerifyAuth(const QString &login)
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

void MainWindow::onBackToAuth()         { stackedWidget->setCurrentIndex(IDX_AUTH); }
void MainWindow::onRegistrationSuccess(){ stackedWidget->setCurrentIndex(IDX_AUTH); }
void MainWindow::onLogout()             { stackedWidget->setCurrentIndex(IDX_AUTH); }

void MainWindow::onResetSuccess()
{
    stackedWidget->setCurrentIndex(IDX_AUTH);
}

void MainWindow::onTaskBtnClicked()
{
    TaskDialog dlg(this);
    dlg.exec();
}

void MainWindow::onSchemaBtnClicked()
{
    SchemaDialog dlg(this);
    dlg.exec();
}
