#include "authwidget.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

AuthWidget::AuthWidget(QWidget *parent)
    : QWidget(parent)
    , failedAttempts(0)
    , lockLevel(0)
    , isLocked(false)
    , m_waitingForAuth(false)
{
    setupUI();
    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &AuthWidget::onLockTimerFired);

    connect(&ClientSingleton::instance(), &ClientSingleton::responseReceived,
            this, &AuthWidget::onAuthResponseReceived);
}

AuthWidget::~AuthWidget() {}

void AuthWidget::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(40, 40, 40, 40);

    auto *title = new QLabel(QString::fromUtf8("Вход"), this);
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    loginEdit = new QLineEdit(this);
    loginEdit->setPlaceholderText(QString::fromUtf8("Логин"));
    loginEdit->setObjectName("authLogin");
    layout->addWidget(loginEdit);

    auto *passRow = new QHBoxLayout;
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText(QString::fromUtf8("Пароль"));
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setObjectName("authPassword");
    togglePasswordBtn = new QPushButton(QString::fromUtf8("👁"), this);
    togglePasswordBtn->setFixedWidth(36);
    connect(togglePasswordBtn, &QPushButton::clicked, this, &AuthWidget::onTogglePassword);
    passRow->addWidget(passwordEdit);
    passRow->addWidget(togglePasswordBtn);
    layout->addLayout(passRow);

    statusLabel = new QLabel(this);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    layout->addWidget(statusLabel);

    attemptsLabel = new QLabel(this);
    attemptsLabel->setObjectName("attemptsLabel");
    attemptsLabel->setAlignment(Qt::AlignCenter);
    attemptsLabel->hide();
    layout->addWidget(attemptsLabel);

    loginBtn = new QPushButton(QString::fromUtf8("Войти"), this);
    loginBtn->setObjectName("primaryBtn");
    connect(loginBtn, &QPushButton::clicked, this, &AuthWidget::onLoginClicked);
    layout->addWidget(loginBtn);

    auto *bottomRow = new QHBoxLayout;
    registerBtn = new QPushButton(QString::fromUtf8("Регистрация"), this);
    registerBtn->setObjectName("linkBtn");
    connect(registerBtn, &QPushButton::clicked, this, &AuthWidget::onRegisterClicked);

    forgotBtn = new QPushButton(QString::fromUtf8("Забыл пароль"), this);
    forgotBtn->setObjectName("linkBtn");
    connect(forgotBtn, &QPushButton::clicked, this, &AuthWidget::onForgotClicked);

    bottomRow->addWidget(registerBtn);
    bottomRow->addStretch();
    bottomRow->addWidget(forgotBtn);
    layout->addLayout(bottomRow);
}

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

    // Hash password
    QString passHash = QString::fromLatin1(
        QCryptographicHash::hash(pass.toUtf8(), QCryptographicHash::Sha256).toHex());

    m_pendingLogin   = login;
    m_waitingForAuth = true;
    loginBtn->setEnabled(false);
    statusLabel->setText(QString::fromUtf8("Проверка..."));
    statusLabel->show();

    ClientSingleton::instance().sendRequestAsync(
        QString("auth||%1||%2").arg(login, passHash));
}

void AuthWidget::onRegisterClicked()  { emit showRegister(); }
void AuthWidget::onForgotClicked()    { emit showReset(); }

void AuthWidget::onAuthResponseReceived(const QString &response)
{
    if (!m_waitingForAuth) return;
    m_waitingForAuth = false;
    loginBtn->setEnabled(true);

    if (response.startsWith("auth_code_sent")) {
        // Parse hash: "auth_code_sent||<sha256hex>"
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
            statusLabel->setText(QString::fromUtf8("Неверный логин или пароль."));
            statusLabel->show();
            attemptsLabel->setText(QString::fromUtf8("Осталось попыток: %1").arg(4 - failedAttempts));
            attemptsLabel->show();
        }
        return;
    }

    // Unknown error
    statusLabel->setText(QString::fromUtf8("Ошибка соединения с сервером."));
    statusLabel->show();
}

void AuthWidget::applyLock(int durationSec, const QString &message)
{
    isLocked = true;
    loginBtn->setEnabled(false);
    statusLabel->setText(message);
    statusLabel->show();
    attemptsLabel->hide();
    lockTimer->start(durationSec * 1000);
}

void AuthWidget::onLockTimerFired()
{
    isLocked = false;
    loginBtn->setEnabled(true);
    statusLabel->hide();
    attemptsLabel->hide();
}
