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

static const char* GH_BG     = "#0d1117";
static const char* GH_PANEL  = "#161b22";
static const char* GH_BORDER = "#30363d";
static const char* GH_TEXT   = "#e6edf3";

/**
 * @brief Конструктор главного окна
 * 
 * Устанавливает заголовок окна, настраивает UI и подключает сигналы.
 * Центрирует окно на экране и устанавливает максимальный размер.
 * 
 * @param parent Родительский виджет
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QString::fromUtf8("\xd0\x93\xd1\x80\xd0\xb0\xd1\x84\xd0\xb8\xd0\xba \xd1\x84\xd1\x83\xd0\xbd\xd0\xba\xd1\x86\xd0\xb8\xd0\xb8"));
    setupUI();
    connectSignals();

    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect sg = screen->availableGeometry();
        resize(qMin(1280, sg.width()), qMin(800, sg.height()));
        move(sg.center() - rect().center());
    }
}

/**
 * @brief Деструктор главного окна
 */
MainWindow::~MainWindow() {}

/**
 * @brief Настройка пользовательского интерфейса
 * 
 * Создаёт центральный виджет, верхнюю панель с кнопками навигации
 * и стековый виджет с экранами: авторизация, регистрация, верификация,
 * график и восстановление пароля.
 */
void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    centralWidget->setStyleSheet(QString("background-color: %1;").arg(GH_BG));
    setCentralWidget(centralWidget);

    mainVLayout = new QVBoxLayout(centralWidget);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->setSpacing(0);

    // ── Top bar ───────────────────────────────────────────────────────────
    QWidget *topBar = new QWidget(centralWidget);
    topBar->setFixedHeight(48);
    topBar->setStyleSheet(QString(
        "QWidget { background-color: %1; border-bottom: 1px solid %2; }"
    ).arg(GH_PANEL).arg(GH_BORDER));

    topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(16, 0, 16, 0);
    topBarLayout->setSpacing(8);

    appTitleLabel = new QLabel(
        QString::fromUtf8("\xd0\x93\xd1\x80\xd0\xb0\xd1\x84\xd0\xb8\xd0\xba \xd1\x84\xd1\x83\xd0\xbd\xd0\xba\xd1\x86\xd0\xb8\xd0\xb8"),
        topBar);
    appTitleLabel->setStyleSheet(QString(
        "QLabel { color: %1; font-size: 14px; font-weight: bold; background: transparent; border: none; }"
    ).arg(GH_TEXT));

    QString btnStyle = QString(
        "QPushButton { background-color: %1; color: %2; border: 1px solid %3;"
        " border-radius: 6px; padding: 4px 12px; font-size: 12px; }"
        "QPushButton:hover { background-color: #21262d; }"
    ).arg(GH_PANEL).arg(GH_TEXT).arg(GH_BORDER);

    taskBtn = new QPushButton(
        QString::fromUtf8("\xd0\x97\xd0\xb0\xd0\xb4\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb5"),
        topBar);
    taskBtn->setStyleSheet(btnStyle);

    schemaBtn = new QPushButton(
        QString::fromUtf8("\xd0\xa1\xd1\x85\xd0\xb5\xd0\xbc\xd0\xb0"),
        topBar);
    schemaBtn->setStyleSheet(btnStyle);

    topBarLayout->addWidget(appTitleLabel);
    topBarLayout->addStretch();
    topBarLayout->addWidget(taskBtn);
    topBarLayout->addWidget(schemaBtn);

    mainVLayout->addWidget(topBar);

    // ── Stacked widget ────────────────────────────────────────────────────
    stackedWidget = new QStackedWidget(centralWidget);
    stackedWidget->setStyleSheet("background: transparent;");
    mainVLayout->addWidget(stackedWidget, 1);

    authWidget   = new AuthWidget(centralWidget);
    regWidget    = new RegWidget(centralWidget);
    verifyWidget = new VerifyWidget(centralWidget);
    graphWidget  = new GraphWidget(centralWidget);
    resetWidget  = new ResetWidget(centralWidget);

    stackedWidget->addWidget(authWidget);    // 0 IDX_AUTH
    stackedWidget->addWidget(regWidget);     // 1 IDX_REG
    stackedWidget->addWidget(verifyWidget);  // 2 IDX_VERIFY
    stackedWidget->addWidget(graphWidget);   // 3 IDX_GRAPH
    stackedWidget->addWidget(resetWidget);   // 4 IDX_RESET

    stackedWidget->setCurrentIndex(IDX_AUTH);
}

/**
 * @brief Подключение сигналов виджетов к слотам главного окна
 * 
 * Устанавливает все соединения для навигации между экранами:
 * авторизация ↔ регистрация, верификация, восстановление пароля,
 * выход из аккаунта, открытие диалогов задания и схемы.
 */
void MainWindow::connectSignals()
{
    connect(authWidget, &AuthWidget::showVerifyAuth, this, &MainWindow::onShowVerifyAuth);
    connect(authWidget, &AuthWidget::showRegister,   this, &MainWindow::onShowRegister);
    connect(authWidget, &AuthWidget::showReset,      this, &MainWindow::onShowReset);

    connect(regWidget,  &RegWidget::registrationSuccess, this, &MainWindow::onRegistrationSuccess);
    connect(regWidget,  &RegWidget::showAuth,             this, &MainWindow::onShowAuth);

    connect(verifyWidget, &VerifyWidget::verificationSuccess, this, &MainWindow::onVerificationSuccess);
    connect(verifyWidget, &VerifyWidget::backToAuth,          this, &MainWindow::onBackToAuth);

    connect(resetWidget, &ResetWidget::resetSuccess, this, &MainWindow::onResetSuccess);
    connect(resetWidget, &ResetWidget::backToAuth,   this, &MainWindow::onBackToAuth);

    connect(graphWidget, &GraphWidget::logout, this, &MainWindow::onLogout);

    connect(taskBtn,   &QPushButton::clicked, this, &MainWindow::onTaskBtnClicked);
    connect(schemaBtn, &QPushButton::clicked, this, &MainWindow::onSchemaBtnClicked);
}

/**
 * @brief Слот для переключения на экран регистрации
 * 
 * Очищает поля виджета регистрации и устанавливает текущий индекс стека на IDX_REG.
 */
void MainWindow::onShowRegister()
{
    regWidget->clearFields();
    stackedWidget->setCurrentIndex(IDX_REG);
}

/**
 * @brief Слот для переключения на экран авторизации
 * 
 * Очищает поля виджета авторизации и устанавливает текущий индекс стека на IDX_AUTH.
 */
void MainWindow::onShowAuth()
{
    authWidget->clearFields();
    stackedWidget->setCurrentIndex(IDX_AUTH);
}

/**
 * @brief Слот для возврата на экран авторизации
 * 
 * Сбрасывает виджеты восстановления пароля и авторизации,
 * гарантируя что флаги ожидания не останутся в состоянии true.
 */
void MainWindow::onBackToAuth()
{
    resetWidget->clearFields();
    authWidget->clearFields();
    stackedWidget->setCurrentIndex(IDX_AUTH);
}

/**
 * @brief Слот для выхода из аккаунта
 * 
 * Очищает поля виджетов восстановления пароля и авторизации,
 * возвращая пользователя на экран входа.
 */
void MainWindow::onLogout()
{
    resetWidget->clearFields();
    authWidget->clearFields();
    stackedWidget->setCurrentIndex(IDX_AUTH);
}

/**
 * @brief Слот для переключения на экран верификации при входе
 * 
 * Устанавливает логин и хэш кода в виджете верификации,
 * затем переключает стек на IDX_VERIFY.
 * 
 * @param login Логин пользователя, проходящего верификацию
 * @param codeHash Хэш кода подтверждения, полученный от сервера
 */
void MainWindow::onShowVerifyAuth(const QString &login, const QString &codeHash)
{
    verifyWidget->setLogin(login, codeHash);
    stackedWidget->setCurrentIndex(IDX_VERIFY);
}

/**
 * @brief Слот, вызываемый при успешной верификации
 * 
 * Устанавливает логин в виджете графика, обновляет данные графика
 * и переключает стек на IDX_GRAPH.
 * 
 * @param login Логин верифицированного пользователя
 */
void MainWindow::onVerificationSuccess(const QString &login)
{
    graphWidget->setUserLogin(login);
    graphWidget->updateGraph();
    stackedWidget->setCurrentIndex(IDX_GRAPH);
}

/**
 * @brief Слот, вызываемый при успешной регистрации
 * 
 * Устанавливает логин в виджете графика, обновляет данные графика
 * и переключает стек на IDX_GRAPH.
 * 
 * @param login Логин зарегистрированного пользователя
 */
void MainWindow::onRegistrationSuccess(const QString &login)
{
    graphWidget->setUserLogin(login);
    graphWidget->updateGraph();
    stackedWidget->setCurrentIndex(IDX_GRAPH);
}

/**
 * @brief Слот для переключения на экран восстановления пароля
 * 
 * Сбрасывает виджет восстановления пароля перед открытием
 * для устранения зависания при повторном открытии.
 */
void MainWindow::onShowReset()
{
    resetWidget->clearFields();
    stackedWidget->setCurrentIndex(IDX_RESET);
}

/**
 * @brief Слот, вызываемый при успешном сбросе пароля
 * 
 * Очищает поля авторизации и возвращает пользователя на экран входа.
 */
void MainWindow::onResetSuccess()
{
    authWidget->clearFields();
    stackedWidget->setCurrentIndex(IDX_AUTH);
}

/**
 * @brief Слот для открытия диалога задания
 * 
 * Создаёт и отображает модальный диалог TaskDialog.
 */
void MainWindow::onTaskBtnClicked()
{
    TaskDialog dlg(this);
    dlg.exec();
}

/**
 * @brief Слот для открытия диалога схемы
 * 
 * Создаёт и отображает модальный диалог SchemaDialog.
 */
void MainWindow::onSchemaBtnClicked()
{
    SchemaDialog dlg(this);
    dlg.exec();
}
