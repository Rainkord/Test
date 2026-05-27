#include "authwidget.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QFont>
#include <QFrame>
#include <QCryptographicHash>

// ── GitHub dark palette ────────────────────────────────────────────────────
#define GH_BG           "#0d1117"
#define GH_CARD         "#161b22"
#define GH_BORDER       "#30363d"
#define GH_TEXT         "#e6edf3"
#define GH_MUTED        "#8b949e"
#define GH_GREEN        "#238636"
#define GH_GREEN_H      "#2ea043"
#define GH_BLUE         "#388bfd"
#define GH_BLUE_H       "#58a6ff"
#define GH_RED          "#f85149"
#define GH_INPUT_BG     "#0d1117"
#define GH_BTN_GHOST    "#21262d"
#define GH_BTN_GHOST_H  "#30363d"

#define FONT_FAMILY      "Segoe UI"
#define FONT_SIZE_TITLE  16
#define FONT_SIZE_BTN    11
#define FONT_SIZE_INPUT  11
#define FONT_SIZE_SMALL   9

/**
 * @brief Генератор стиля для полей ввода
 * 
 * @return Строка CSS-стиля для QLineEdit в стиле GitHub dark
 */
static QString s_input()
{
    return QString(
        "QLineEdit {"
        "  background-color: %1; color: %2;"
        "  border: 1px solid %3; border-radius: 6px;"
        "  padding: 6px 10px; font-family: '%4'; font-size: %5pt;"
        "}"
        "QLineEdit:focus { border-color: %6; }"
    ).arg(GH_INPUT_BG).arg(GH_TEXT).arg(GH_BORDER)
     .arg(FONT_FAMILY).arg(FONT_SIZE_INPUT).arg(GH_BLUE);
}

/**
 * @brief Генератор стиля для основной кнопки (зелёная)
 * 
 * @return Строка CSS-стиля для QPushButton в стиле GitHub green
 */
static QString s_primaryBtn()
{
    return QString(
        "QPushButton {"
        "  background-color: %1; color: #ffffff;"
        "  border: 1px solid rgba(240,246,252,0.1); border-radius: 6px;"
        "  padding: 6px 16px; font-family: '%3'; font-size: %4pt; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:disabled { background-color: rgba(35,134,54,0.4); color: rgba(255,255,255,0.4); }"
    ).arg(GH_GREEN).arg(GH_GREEN_H).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

/**
 * @brief Генератор стиля для призрачной кнопки
 * 
 * @return Строка CSS-стиля для QPushButton с прозрачным фоном
 */
static QString s_ghostBtn()
{
    return QString(
        "QPushButton {"
        "  background-color: %1; color: %3;"
        "  border: 1px solid %4; border-radius: 6px;"
        "  padding: 5px 12px; font-family: '%5'; font-size: %6pt;"
        "}"
        "QPushButton:hover { background-color: %2; }"
    ).arg(GH_BTN_GHOST).arg(GH_BTN_GHOST_H).arg(GH_TEXT)
     .arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

/**
 * @brief Генератор стиля для кнопки-ссылки
 * 
 * @return Строка CSS-стиля для QPushButton в виде ссылки
 */
static QString s_linkBtn()
{
    return QString(
        "QPushButton { color: %1; border: none; background: transparent;"
        " font-family: '%2'; font-size: %3pt; }"
        "QPushButton:hover { color: %4; text-decoration: underline; }"
    ).arg(GH_BLUE).arg(FONT_FAMILY).arg(FONT_SIZE_BTN).arg(GH_BLUE_H);
}

/**
 * @brief Генератор стиля для метки ошибки
 * 
 * @return Строка CSS-стиля для QLabel с красным цветом текста
 */
static QString s_errorLabel()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_RED).arg(FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

/**
 * @brief Генератор стиля для метки-подсказки
 * 
 * @return Строка CSS-стиля для QLabel с серым цветом текста
 */
static QString s_hintLabel()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_MUTED).arg(FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

/**
 * @brief Конструктор виджета авторизации
 * 
 * Инициализирует переменные состояния, настраивает UI,
 * создаёт таймер блокировки и подключает сигнал ответа сервера.
 * 
 * @param parent Родительский виджет
 */
AuthWidget::AuthWidget(QWidget *parent)
    : QWidget(parent)
    , failedAttempts(0)
    , lockLevel(0)
    , isLocked(false)
    , m_waitingForAuth(false)
{
    setStyleSheet(QString(
        "QWidget { background-color: %1; color: %2; font-family: '%3'; font-size: %4pt; }")
        .arg(GH_BG).arg(GH_TEXT).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT));

    setupUI();

    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &AuthWidget::onLockTimerFired);

    connect(&ClientSingleton::instance(), &ClientSingleton::responseReceived,
            this, &AuthWidget::onAuthResponseReceived);
}

/**
 * @brief Деструктор виджета авторизации
 */
AuthWidget::~AuthWidget() {}

/**
 * @brief Настройка пользовательского интерфейса
 * 
 * Создаёт центрированную карточку с заголовком, полями ввода
 * логина и пароля, кнопками "Войти", "Регистрация" и "Забыл пароль",
 * а также статусными метками для отображения ошибок.
 */
void AuthWidget::setupUI()
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);
    outerLayout->addStretch(1);

    // ── Карточка
    QWidget *card = new QWidget(this);
    card->setFixedWidth(400);
    card->setStyleSheet(QString(
        "QWidget { background-color: %1; border: 1px solid %2; border-radius: 8px; }")
        .arg(GH_CARD).arg(GH_BORDER));

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 24, 24, 24);
    cardLayout->setSpacing(14);

    // Заголовок
    auto *title = new QLabel(QString::fromUtf8("Вход в аккаунт"), card);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QString(
        "QLabel { color: %1; font-size: %2pt; font-weight: bold; border: none; background: transparent; }")
        .arg(GH_TEXT).arg(FONT_SIZE_TITLE));
    cardLayout->addWidget(title);

    // Разделитель
    auto *sep = new QFrame(card);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(QString("QFrame { background: %1; border: none; max-height: 1px; }").arg(GH_BORDER));
    cardLayout->addWidget(sep);

    // Поле логина
    loginEdit = new QLineEdit(card);
    loginEdit->setPlaceholderText(QString::fromUtf8("Введите логин"));
    loginEdit->setStyleSheet(s_input());
    connect(loginEdit, &QLineEdit::returnPressed, this, &AuthWidget::onLoginClicked);
    cardLayout->addWidget(loginEdit);

    // Поле пароля
    auto *passRow = new QHBoxLayout;
    passRow->setSpacing(6);
    passwordEdit = new QLineEdit(card);
    passwordEdit->setPlaceholderText(QString::fromUtf8("Введите пароль"));
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(s_input());
    connect(passwordEdit, &QLineEdit::returnPressed, this, &AuthWidget::onLoginClicked);

    togglePasswordBtn = new QPushButton(QString::fromUtf8("👁"), card);
    togglePasswordBtn->setFixedSize(34, 34);
    togglePasswordBtn->setStyleSheet(s_ghostBtn());
    togglePasswordBtn->setToolTip(QString::fromUtf8("Показать/скрыть пароль"));
    connect(togglePasswordBtn, &QPushButton::clicked, this, &AuthWidget::onTogglePassword);

    passRow->addWidget(passwordEdit, 1);
    passRow->addWidget(togglePasswordBtn);
    cardLayout->addLayout(passRow);

    // Статус / ошибки
    statusLabel = new QLabel(card);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    statusLabel->setStyleSheet(s_errorLabel());
    statusLabel->hide();
    cardLayout->addWidget(statusLabel);

    attemptsLabel = new QLabel(card);
    attemptsLabel->setAlignment(Qt::AlignCenter);
    attemptsLabel->setStyleSheet(s_hintLabel());
    attemptsLabel->hide();
    cardLayout->addWidget(attemptsLabel);

    // Кнопка войти
    loginBtn = new QPushButton(QString::fromUtf8("Войти"), card);
    loginBtn->setDefault(true);
    loginBtn->setAutoDefault(true);
    loginBtn->setStyleSheet(s_primaryBtn());
    connect(loginBtn, &QPushButton::clicked, this, &AuthWidget::onLoginClicked);
    cardLayout->addWidget(loginBtn);

    // Нижняя строка: Регистрация | Забыл пароль
    auto *bottomRow = new QHBoxLayout;
    bottomRow->setSpacing(8);

    registerBtn = new QPushButton(QString::fromUtf8("Регистрация"), card);
    registerBtn->setStyleSheet(s_linkBtn());
    connect(registerBtn, &QPushButton::clicked, this, &AuthWidget::onRegisterClicked);

    forgotBtn = new QPushButton(QString::fromUtf8("Забыл пароль"), card);
    forgotBtn->setStyleSheet(s_linkBtn());
    connect(forgotBtn, &QPushButton::clicked, this, &AuthWidget::onForgotClicked);

    bottomRow->addWidget(registerBtn);
    bottomRow->addStretch();
    bottomRow->addWidget(forgotBtn);
    cardLayout->addLayout(bottomRow);

    auto *hCenter = new QHBoxLayout;
    hCenter->addStretch(1);
    hCenter->addWidget(card);
    hCenter->addStretch(1);
    outerLayout->addLayout(hCenter);
    outerLayout->addStretch(1);
}

/**
 * @brief Очистить все поля ввода и сбросить состояние виджета
 * 
 * Очищает поля логина и пароля, скрывает статусные метки,
 * включает кнопку входа и сбрасывает флаги ожидания.
 */
void AuthWidget::clearFields()
{
    loginEdit->clear();
    passwordEdit->clear();
    statusLabel->hide();
    attemptsLabel->hide();
    loginBtn->setEnabled(true);
    m_waitingForAuth = false;
    m_pendingLogin.clear();
    m_pendingCodeHash.clear();
}

/**
 * @brief Слот для переключения видимости пароля
 * 
 * Переключает режим отображения поля пароля между
 * скрытым (Password) и видимым (Normal).
 */
void AuthWidget::onTogglePassword()
{
    if (passwordEdit->echoMode() == QLineEdit::Password) {
        passwordEdit->setEchoMode(QLineEdit::Normal);
        togglePasswordBtn->setText(QString::fromUtf8("🚫"));
    } else {
        passwordEdit->setEchoMode(QLineEdit::Password);
        togglePasswordBtn->setText(QString::fromUtf8("👁"));
    }
}

/**
 * @brief Слот для обработки нажатия кнопки "Войти"
 * 
 * Проверяет заполненность полей, хэширует пароль (SHA-256),
 * отправляет асинхронный запрос авторизации на сервер
 * и отображает статус "Проверка...".
 */
void AuthWidget::onLoginClicked()
{
    if (isLocked || m_waitingForAuth) return;

    QString login = loginEdit->text().trimmed();
    QString pass  = passwordEdit->text();
    if (login.isEmpty() || pass.isEmpty()) {
        statusLabel->setText(QString::fromUtf8("Заполните все поля."));
        statusLabel->show();
        return;
    }

    QString passHash = QString::fromLatin1(
        QCryptographicHash::hash(pass.toUtf8(), QCryptographicHash::Sha256).toHex());

    m_pendingLogin   = login;
    m_waitingForAuth = true;
    loginBtn->setEnabled(false);
    statusLabel->setStyleSheet(s_hintLabel());
    statusLabel->setText(QString::fromUtf8("Проверка..."));
    statusLabel->show();

    ClientSingleton::instance().sendRequestAsync(
        QString("auth||%1||%2").arg(login, passHash));
}

/**
 * @brief Слот для обработки нажатия ссылки "Регистрация"
 * 
 * Испускает сигнал showRegister для переключения на экран регистрации.
 */
void AuthWidget::onRegisterClicked() { emit showRegister(); }

/**
 * @brief Слот для обработки нажатия ссылки "Забыл пароль"
 * 
 * Испускает сигнал showReset для переключения на экран восстановления пароля.
 */
void AuthWidget::onForgotClicked()   { emit showReset(); }

/**
 * @brief Слот для обработки ответа сервера на запрос авторизации
 * 
 * Обрабатывает три варианта ответа:
 * - auth_code_sent: сервер отправил код подтверждения, переход на верификацию
 * - auth-: неверные учётные данные, увеличение счётчика попыток с блокировкой
 * - другой: ошибка соединения с сервером
 * 
 * @param response Ответ сервера
 */
void AuthWidget::onAuthResponseReceived(const QString &response)
{
    if (!m_waitingForAuth) return;
    m_waitingForAuth = false;
    loginBtn->setEnabled(true);

    if (response.startsWith("auth_code_sent")) {
        QStringList parts = response.split("||");
        m_pendingCodeHash = (parts.size() >= 2) ? parts[1].trimmed() : QString();
        statusLabel->hide();
        attemptsLabel->hide();
        emit showVerifyAuth(m_pendingLogin, m_pendingCodeHash);
        return;
    }

    if (response == "auth-") {
        failedAttempts++;
        if (failedAttempts >= 4) {
            failedAttempts = 0;
            lockLevel++;
            int sec = (lockLevel == 1) ? 30 : (lockLevel == 2 ? 300 : 1800);
            applyLock(sec, QString::fromUtf8("Превышен лимит попыток. Блокировка на %1 сек.").arg(sec));
        } else {
            statusLabel->setStyleSheet(s_errorLabel());
            statusLabel->setText(QString::fromUtf8("Неверный логин или пароль."));
            statusLabel->show();
            attemptsLabel->setText(QString::fromUtf8("Осталось попыток: %1").arg(4 - failedAttempts));
            attemptsLabel->show();
        }
        return;
    }

    statusLabel->setStyleSheet(s_errorLabel());
    statusLabel->setText(QString::fromUtf8("Ошибка соединения с сервером."));
    statusLabel->show();
}

/**
 * @brief Применить блокировку входа на указанное время
 * 
 * Блокирует кнопку входа, отображает сообщение о блокировке
 * и запускает таймер на указанное количество секунд.
 * 
 * @param durationSec Длительность блокировки в секундах
 * @param message Сообщение для отображения пользователю
 */
void AuthWidget::applyLock(int durationSec, const QString &message)
{
    isLocked = true;
    loginBtn->setEnabled(false);
    statusLabel->setStyleSheet(s_errorLabel());
    statusLabel->setText(message);
    statusLabel->show();
    attemptsLabel->hide();
    lockTimer->start(durationSec * 1000);
}

/**
 * @brief Слот для обработки срабатывания таймера блокировки
 * 
 * Снимает блокировку, включает кнопку входа и скрывает статусные метки.
 */
void AuthWidget::onLockTimerFired()
{
    isLocked = false;
    loginBtn->setEnabled(true);
    statusLabel->hide();
    attemptsLabel->hide();
}
