/**
 * @file verifywidget.cpp
 * @brief Локальная верификация SHA-256 хэша кода (без сетевого запроса).
 */

#include "verifywidget.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QCryptographicHash>

VerifyWidget::VerifyWidget(QWidget *parent)
    : QWidget(parent)
    , isLocked(false)
    , m_attemptsLeft(3)
{
    setupUI();
    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &VerifyWidget::onLockTimerFired);
}

VerifyWidget::~VerifyWidget() {}

void VerifyWidget::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(40, 40, 40, 40);

    auto *title = new QLabel(QString::fromUtf8("Подтверждение входа"), this);
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    hintLabel = new QLabel(QString::fromUtf8("Код отправлен на ваш email."), this);
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setWordWrap(true);
    layout->addWidget(hintLabel);

    codeEdit = new QLineEdit(this);
    codeEdit->setPlaceholderText(QString::fromUtf8("Введите 6-значный код"));
    codeEdit->setMaxLength(6);
    codeEdit->setObjectName("codeEdit");
    connect(codeEdit, &QLineEdit::textChanged, this, &VerifyWidget::onCodeTextChanged);
    layout->addWidget(codeEdit);

    statusLabel = new QLabel(this);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    layout->addWidget(statusLabel);

    verifyBtn = new QPushButton(QString::fromUtf8("Подтвердить"), this);
    verifyBtn->setObjectName("primaryBtn");
    verifyBtn->setEnabled(false);
    connect(verifyBtn, &QPushButton::clicked, this, &VerifyWidget::onVerifyClicked);
    layout->addWidget(verifyBtn);

    backBtn = new QPushButton(QString::fromUtf8("Назад"), this);
    backBtn->setObjectName("linkBtn");
    connect(backBtn, &QPushButton::clicked, this, &VerifyWidget::onBackClicked);
    layout->addWidget(backBtn);
}

void VerifyWidget::setLogin(const QString &login, const QString &codeHash)
{
    m_login       = login;
    m_codeHash    = codeHash;
    m_attemptsLeft = 3;
    isLocked      = false;
    codeEdit->clear();
    codeEdit->setEnabled(true);
    verifyBtn->setEnabled(false);
    statusLabel->hide();
    hintLabel->setText(QString::fromUtf8("Код отправлен на ваш email."));
}

void VerifyWidget::clearFields()
{
    codeEdit->clear();
    statusLabel->hide();
    verifyBtn->setEnabled(false);
    m_codeHash.clear();
    m_login.clear();
    m_attemptsLeft = 3;
    isLocked       = false;
}

void VerifyWidget::onCodeTextChanged(const QString &text)
{
    verifyBtn->setEnabled(!isLocked && text.length() == 6);
}

void VerifyWidget::onVerifyClicked()
{
    if (isLocked) return;
    verifyBtn->setEnabled(false);

    QString code = codeEdit->text().trimmed();
    if (code.length() != 6) {
        statusLabel->setText(QString::fromUtf8("Введите 6-значный код."));
        statusLabel->show();
        verifyBtn->setEnabled(true);
        return;
    }

    // local verification: SHA-256(entered) == stored hash from server
    QString enteredHash = QString::fromLatin1(
        QCryptographicHash::hash(code.toUtf8(), QCryptographicHash::Sha256).toHex());

    if (enteredHash == m_codeHash) {
        statusLabel->setText(QString::fromUtf8("Код верен!"));
        statusLabel->show();
        codeEdit->setEnabled(false);
        emit verificationSuccess(m_login);
        return;
    }

    // Wrong code
    m_attemptsLeft--;
    if (m_attemptsLeft > 0) {
        statusLabel->setText(
            QString::fromUtf8("Неверный код. Осталось попыток: %1.").arg(m_attemptsLeft));
        statusLabel->show();
        verifyBtn->setEnabled(true);
    } else {
        applyLock(30, QString::fromUtf8("Превышен лимит попыток. Блокировка на 30 сек."));
    }
}

void VerifyWidget::onBackClicked()
{
    clearFields();
    emit backToAuth();
}

void VerifyWidget::applyLock(int seconds, const QString &message)
{
    isLocked = true;
    codeEdit->setEnabled(false);
    verifyBtn->setEnabled(false);
    statusLabel->setText(message);
    statusLabel->show();
    lockTimer->start(seconds * 1000);
}

void VerifyWidget::onLockTimerFired()
{
    isLocked       = false;
    m_attemptsLeft = 3;
    codeEdit->setEnabled(true);
    codeEdit->clear();
    statusLabel->hide();
}

void VerifyWidget::onVerifyResponseReceived(const QString &/*response*/)
{
    // Не используется: проверка выполняется локально.
}
