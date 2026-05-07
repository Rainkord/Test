#include "mainwindow.h"
#include "taskdialog.h"
#include "schemadialog.h"

#include <QStyle>
#include <QApplication>
#include <QSpacerItem>
#include <QFrame>
#include <QPainter>
#include <QPixmap>
#include <QIcon>

// ── GitHub Dark palette ──────────────────────────────────────────────────
#define GH_BG          "#0d1117"
#define GH_TOPBAR      "#161b22"
#define GH_BORDER      "#30363d"
#define GH_TEXT        "#e6edf3"
#define GH_MUTED       "#8b949e"
#define GH_BTN_GHOST   "#21262d"
#define GH_BTN_GHOST_H "#30363d"
#define GH_BLUE        "#388bfd"
#define FONT_FAMILY    "Segoe UI"

static QIcon tintIcon(const QIcon &src, const QColor &color, int sz = 18)
{
    QPixmap pm = src.pixmap(sz, sz);
    QPainter painter(&pm);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pm.rect(), color);
    painter.end();
    return QIcon(pm);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setStyleSheet(QString(
        "QMainWindow { background-color: %1; }"
        "QWidget      { background-color: %1; color: %2; font-family: '%3'; }"
    ).arg(GH_BG).arg(GH_TEXT).arg(FONT_FAMILY));

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

    // ── Top bar ────────────────────────────────────────────────────────
    QWidget *topBar = new QWidget(centralWidget);
    topBar->setFixedHeight(44);
    topBar->setStyleSheet(QString(
        "QWidget { background-color: %1; border-bottom: 1px solid %2; }"
        "QPushButton {"
        "  background-color: %3; color: %4;"
        "  border: 1px solid %2; border-radius: 6px;"
        "  padding: 4px 12px; font-size: 11px; font-family: '%5';"
        "}"
        "QPushButton:hover   { background-color: %6; }"
        "QPushButton:pressed { background-color: %2; }"
    ).arg(GH_TOPBAR).arg(GH_BORDER).arg(GH_BTN_GHOST)
     .arg(GH_TEXT).arg(FONT_FAMILY).arg(GH_BTN_GHOST_H));

    topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(12, 0, 12, 0);
    topBarLayout->setSpacing(6);

    const QColor iconColor(0xe6, 0xed, 0xf3);

    taskBtn = new QPushButton(topBar);
    taskBtn->setIcon(tintIcon(style()->standardIcon(QStyle::SP_FileDialogInfoView), iconColor));
    taskBtn->setIconSize(QSize(16, 16));
    taskBtn->setText("  Задание");
    taskBtn->setToolTip("Показать задание");
    taskBtn->setMinimumWidth(100);
    taskBtn->setMinimumHeight(30);

    schemaBtn = new QPushButton(topBar);
    schemaBtn->setIcon(tintIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView), iconColor));
    schemaBtn->setIconSize(QSize(16, 16));
    schemaBtn->setText("  Блок-схема");
    schemaBtn->setToolTip("Показать блок-схему");
    schemaBtn->setMinimumWidth(110);
    schemaBtn->setMinimumHeight(30);

    appTitleLabel = new QLabel("ТМП — Подгруппа 5", topBar);
    appTitleLabel->setStyleSheet(QString(
        "QLabel { color: %1; font-size: 12px; font-weight: bold; background: transparent; border: none; }"
    ).arg(GH_TEXT));
    appTitleLabel->setAlignment(Qt::AlignCenter);

    topBarLayout->addWidget(taskBtn);
    topBarLayout->addWidget(schemaBtn);
    topBarLayout->addStretch(1);
    topBarLayout->addWidget(appTitleLabel);
    topBarLayout->addStretch(1);

    // ── StackedWidget ───────────────────────────────────────────
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
    mainVLayout->addWidget(stackedWidget);
}

void MainWindow::connectSignals()
{
    connect(authWidget, &AuthWidget::showRegister,    this, &MainWindow::onShowRegister);
    connect(authWidget, &AuthWidget::loginSuccess,    this, [this](const QString &login) {
        graphWidget->setLogin(login);
        stackedWidget->setCurrentIndex(IDX_GRAPH);
    });
    connect(authWidget, &AuthWidget::showVerifyAuth, this, &MainWindow::onShowVerifyAuth);
    connect(authWidget, &AuthWidget::showReset,      this, &MainWindow::onShowReset);

    connect(regWidget, &RegWidget::registrationSuccess, this, &MainWindow::onRegistrationSuccess);
    connect(regWidget, &RegWidget::showAuth,            this, &MainWindow::onShowAuth);

    connect(verifyWidget, &VerifyWidget::verificationSuccess, this, &MainWindow::onVerificationSuccess);
    connect(verifyWidget, &VerifyWidget::backToAuth,          this, &MainWindow::onBackToAuth);

    connect(resetWidget, &ResetWidget::backToAuth,   this, &MainWindow::onShowAuth);
    connect(resetWidget, &ResetWidget::resetSuccess, this, &MainWindow::onResetSuccess);

    connect(graphWidget, &GraphWidget::logout, this, &MainWindow::onLogout);

    connect(taskBtn,   &QPushButton::clicked, this, &MainWindow::onTaskBtnClicked);
    connect(schemaBtn, &QPushButton::clicked, this, &MainWindow::onSchemaBtnClicked);
}

void MainWindow::onShowRegister()  { regWidget->clearFields();  stackedWidget->setCurrentIndex(IDX_REG);   }
void MainWindow::onShowAuth()      { authWidget->clearFields(); stackedWidget->setCurrentIndex(IDX_AUTH);  }
void MainWindow::onShowReset()     { stackedWidget->setCurrentIndex(IDX_RESET); }

void MainWindow::onShowVerifyAuth(const QString &login)
{
    verifyWidget->setLogin(login);
    stackedWidget->setCurrentIndex(IDX_VERIFY);
}

void MainWindow::onVerificationSuccess(const QString &login)
{
    graphWidget->setLogin(login);
    stackedWidget->setCurrentIndex(IDX_GRAPH);
}

void MainWindow::onBackToAuth()    { stackedWidget->setCurrentIndex(IDX_AUTH); }

void MainWindow::onRegistrationSuccess(const QString &login)
{
    graphWidget->setLogin(login);
    stackedWidget->setCurrentIndex(IDX_GRAPH);
}

void MainWindow::onLogout()        { authWidget->clearFields(); stackedWidget->setCurrentIndex(IDX_AUTH); }
void MainWindow::onResetSuccess()  { stackedWidget->setCurrentIndex(IDX_AUTH); }

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
