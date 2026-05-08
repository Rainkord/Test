#include "authwidget.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QCryptographicHash>

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

    auto *title = new QLabel(QString::fromUtf8("\u0412\u0445\u043e\u0434"), this);
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    loginEdit = new QLineEdit(this);
    loginEdit->setPlaceholderText(QString::fromUtf8("\u041b\u043e\u0433\u0438\u043d"));
    loginEdit->setObjectName("authLogin");
    layout->addWidget(loginEdit);

    auto *passRow = new QHBoxLayout;
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText(QString::fromUtf8("\u041f\u0430\u0440\u043e\u043b\u044c"));
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setObjectName("authPassword");
    togglePasswordBtn = new QPushButton(QString::fromUtf8("\U0001f441"), this);
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

    loginBtn = new QPushButton(QString::fromUtf8("\u0412\u043e\u0439\u0442\u0438"), this);
    loginBtn->setObjectName("primaryBtn");
    connect(loginBtn, &QPushButton::clicked, this, &AuthWidget::onLoginClicked);
    layout->addWidget(loginBtn);

    auto *bottomRow = new QHBoxLayout;
    registerBtn = new QPushButton(QString::fromUtf8("\u0420\u0435\u0433\u0438\u0441\u0442\u0440\u0430\u0446\u0438\u044f"), this);
    registerBtn->setObjectName("linkBtn");
    connect(registerBtn, &QPushButton::clicked, this, &AuthWidget::onRegisterClicked);

    forgotBtn = new QPushButton(QString::fromUtf8("\u0417\u0430\u0431\u044b\u043b \u043f\u0430\u0440\u043e\u043b\u044c"), this);
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
        togglePasswordBtn->setText(QString::fromUtf8("\U0001f6ab"));
    } else {
        passwordEdit->setEchoMode(QLineEdit::Password);
        togglePasswordBtn->setText(QString::fromUtf8("\U0001f441"));
    }
}

void AuthWidget::onLoginClicked()
{
    if (isLocked || m_waitingForAuth) return;

    QString login = loginEdit->text().trimmed();
    QString pass  = passwordEdit->text();
    if (login.isEmpty() || pass.isEmpty()) {
        statusLabel->setText(QString::fromUtf8("\u0417\u0430\u043f\u043e\u043b\u043d\u0438\u0442\u0435 \u0432\u0441\u0435 \u043f\u043e\u043b\u044f."));
        statusLabel->show();
        return;
    }

    // Hash password SHA-256
    QString passHash = QString::fromLatin1(
        QCryptographicHash::hash(pass.toUtf8(), QCryptographicHash::Sha256).toHex());

    m_pendingLogin   = login;
    m_waitingForAuth = true;
    loginBtn->setEnabled(false);
    statusLabel->setText(QString::fromUtf8("\u041f\u0440\u043e\u0432\u0435\u0440\u043a\u0430..."));
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
            applyLock(sec, QString::fromUtf8("\u041f\u0440\u0435\u0432\u044b\u0448\u0435\u043d \u043b\u0438\u043c\u0438\u0442 \u043f\u043e\u043f\u044b\u0442\u043e\u043a. \u0411\u043b\u043e\u043a\u0438\u0440\u043e\u0432\u043a\u0430 \u043d\u0430 %1 \u0441\u0435\u043a.").arg(sec));
        } else {
            statusLabel->setText(QString::fromUtf8("\u041d\u0435\u0432\u0435\u0440\u043d\u044b\u0439 \u043b\u043e\u0433\u0438\u043d \u0438\u043b\u0438 \u043f\u0430\u0440\u043e\u043b\u044c."));
            statusLabel->show();
            attemptsLabel->setText(QString::fromUtf8("\u041e\u0441\u0442\u0430\u043b\u043e\u0441\u044c \u043f\u043e\u043f\u044b\u0442\u043e\u043a: %1").arg(4 - failedAttempts));
            attemptsLabel->show();
        }
        return;
    }

    // Unknown / connection error
    statusLabel->setText(QString::fromUtf8("\u041e\u0448\u0438\u0431\u043a\u0430 \u0441\u043e\u0435\u0434\u0438\u043d\u0435\u043d\u0438\u044f \u0441 \u0441\u0435\u0440\u0432\u0435\u0440\u043e\u043c."));
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
