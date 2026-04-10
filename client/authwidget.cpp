#include "authwidget.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QCryptographicHash>
#include <QFont>

AuthWidget::AuthWidget(QWidget *parent)
    : QWidget(parent),
      failedAttempts(0),
      lockLevel(0),
      isLocked(false),
      m_waitingForAuth(false)
{
    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &AuthWidget::onLockTimerFired);

    connect(&ClientSingleton::instance(), &ClientSingleton::responseReceived,
            this, &AuthWidget::onAuthResponseReceived);

    setupUI();
}

AuthWidget::~AuthWidget() {}

void AuthWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(12);

    QLabel *titleLabel = new QLabel("Авторизация", this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(18);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(10);

    loginEdit = new QLineEdit(this);
    loginEdit->setPlaceholderText("Логин");
    loginEdit->setMinimumHeight(36);
    loginEdit->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #cccccc; border-radius: 4px; }");
    mainLayout->addWidget(loginEdit);

    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordLayout->setSpacing(6);

    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Пароль");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(36);
    passwordEdit->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #cccccc; border-radius: 4px; }");
    passwordLayout->addWidget(passwordEdit);

    togglePasswordBtn = new QPushButton("👁", this);
    togglePasswordBtn->setFixedWidth(35);
    togglePasswordBtn->setFixedHeight(36);
    togglePasswordBtn->setToolTip("Показать/скрыть пароль");
    togglePasswordBtn->setStyleSheet(
        "QPushButton { border: 1px solid #cccccc; border-radius: 4px; background: #f5f5f5; }"
        "QPushButton:hover { background: #e0e0e0; }"
    );
    connect(togglePasswordBtn, &QPushButton::clicked, this, &AuthWidget::onTogglePassword);
    passwordLayout->addWidget(togglePasswordBtn);
    mainLayout->addLayout(passwordLayout);

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("QLabel { color: red; }");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    mainLayout->addWidget(statusLabel);

    attemptsLabel = new QLabel(this);
    attemptsLabel->setStyleSheet("QLabel { color: #888888; font-size: 10pt; }");
    attemptsLabel->setAlignment(Qt::AlignCenter);
    attemptsLabel->hide();
    mainLayout->addWidget(attemptsLabel);

    mainLayout->addSpacing(8);

    loginBtn = new QPushButton("Войти", this);
    loginBtn->setMinimumHeight(38);
    loginBtn->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; border: none; border-radius: 4px; font-size: 13pt; }"
        "QPushButton:hover { background-color: #1976D2; }"
        "QPushButton:disabled { background-color: #cccccc; color: #666666; }"
    );
    connect(loginBtn, &QPushButton::clicked, this, &AuthWidget::onLoginClicked);
    mainLayout->addWidget(loginBtn);

    mainLayout->addSpacing(8);

    registerBtn = new QPushButton("Регистрация", this);
    registerBtn->setFlat(true);
    registerBtn->setStyleSheet(
        "QPushButton { color: #2196F3; border: none; text-decoration: underline; font-size: 11pt; }"
        "QPushButton:hover { color: #1565C0; }"
    );
    connect(registerBtn, &QPushButton::clicked, this, &AuthWidget::onRegisterClicked);
    mainLayout->addWidget(registerBtn, 0, Qt::AlignCenter);

    forgotBtn = new QPushButton("Забыли пароль? Восстановить", this);
    forgotBtn->setFlat(true);
    forgotBtn->setStyleSheet(
        "QPushButton { color: #2196F3; border: none; text-decoration: underline; font-size: 11pt; }"
        "QPushButton:hover { color: #1565C0; }"
    );
    connect(forgotBtn, &QPushButton::clicked, this, &AuthWidget::onForgotClicked);
    mainLayout->addWidget(forgotBtn, 0, Qt::AlignCenter);

    mainLayout->addStretch();
}

void AuthWidget::onTogglePassword()
{
    passwordEdit->setEchoMode(
        passwordEdit->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password
    );
}

void AuthWidget::applyLock(int minutes, const QString &message)
{
    isLocked = true;
    loginBtn->setEnabled(false);
    statusLabel->setText(message);
    statusLabel->show();
    attemptsLabel->hide();
    lockTimer->start(minutes == 0 ? 30 * 1000 : minutes * 60 * 1000);
}

void AuthWidget::onLockTimerFired()
{
    isLocked = false;
    loginBtn->setEnabled(true);
    statusLabel->hide();
    attemptsLabel->hide();
}

void AuthWidget::onLoginClicked()
{
    if (isLocked) {
        int remainingSec    = lockTimer->remainingTime() / 1000;
        int remainingMin    = remainingSec / 60;
        int remainingSecMod = remainingSec % 60;
        QString timeStr = remainingMin > 0
            ? QString("Осталось %1 мин %2 сек").arg(remainingMin).arg(remainingSecMod)
            : QString("Осталось %1 сек").arg(remainingSec);
        statusLabel->setText("Аккаунт заблокирован. " + timeStr);
        statusLabel->show();
        return;
    }

    QString login    = loginEdit->text().trimmed();
    QString password = passwordEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        statusLabel->setText("Введите логин и пароль.");
        statusLabel->show();
        return;
    }

    loginBtn->setEnabled(false);
    statusLabel->setText("Подключаемся...");
    statusLabel->setStyleSheet("QLabel { color: #888888; }");
    statusLabel->show();

    QByteArray hashBytes = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString passwordHash = QString::fromLatin1(hashBytes.toHex());

    m_waitingForAuth = true;
    QString request = QString("auth||%1||%2").arg(login, passwordHash);
    ClientSingleton::instance().sendRequestAsync(request);
}

void AuthWidget::onAuthResponseReceived(const QString &response)
{
    if (!m_waitingForAuth) return;
    m_waitingForAuth = false;

    QString r = response.trimmed();
    if (r.isEmpty()) {
        loginBtn->setEnabled(true);
        statusLabel->setText("Ошибка соединения с сервером.");
        statusLabel->setStyleSheet("QLabel { color: red; }");
        statusLabel->show();
        return;
    }

    if (r == "auth_code_sent") {
        statusLabel->hide();
        emit showVerifyAuth(loginEdit->text().trimmed());
        return;
    }

    loginBtn->setEnabled(true);
    statusLabel->setStyleSheet("QLabel { color: red; }");

    if (r == "auth-") {
        failedAttempts++;
        if (failedAttempts < 4) {
            statusLabel->setText(
                QString("Неверный логин или пароль. Осталось попыток: %1").arg(4 - failedAttempts)
            );
            statusLabel->show();
            return;
        }
        if (failedAttempts == 4) { lockLevel = 1; applyLock(0,    "Аккаунт заблокирован на 30 секунд");           return; }
        if (failedAttempts == 5) { lockLevel = 2; applyLock(5,    "Аккаунт заблокирован на 5 минут");             return; }
        if (failedAttempts == 6) { lockLevel = 3; applyLock(10,   "Аккаунт заблокирован на 10 минут");            return; }
        lockLevel = 4; applyLock(9999, "Аккаунт заблокирован на длительное время");
        return;
    }

    m_waitingForAuth = false;
}

void AuthWidget::onRegisterClicked() { emit showRegister(); }
void AuthWidget::onForgotClicked()   { emit showReset(); }
