/**
 * @file verifywidget.cpp
 * @brief Локальная верификация SHA-256 хэша кода. OTP-ввод 6 боксов.
 */

#include "verifywidget.h"
#include "otpinput.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QCryptographicHash>
#include <QFont>
#include <QFrame>

#define GH_BG       "#0d1117"
#define GH_CARD     "#161b22"
#define GH_BORDER   "#30363d"
#define GH_TEXT     "#e6edf3"
#define GH_MUTED    "#8b949e"
#define GH_GREEN    "#238636"
#define GH_GREEN_H  "#2ea043"
#define GH_BLUE     "#388bfd"
#define GH_BLUE_H   "#58a6ff"
#define GH_RED      "#f85149"
#define FONT_FAMILY "Segoe UI"
#define FS_TITLE    16
#define FS_BTN      11
#define FS_SMALL     9

static QString primaryBtnStyle(bool enabled)
{
    if (enabled)
        return QString(
            "QPushButton {"
            "  background-color: %1; color: #fff;"
            "  border: 1px solid rgba(240,246,252,.1); border-radius: 6px;"
            "  padding: 6px 16px; font-family: '%3'; font-size: %4pt; font-weight: bold;"
            "}"
            "QPushButton:hover { background-color: %2; }"
        ).arg(GH_GREEN, GH_GREEN_H, FONT_FAMILY).arg(FS_BTN);
    return QString(
        "QPushButton {"
        "  background-color: rgba(35,134,54,.35); color: rgba(255,255,255,.4);"
        "  border: 1px solid rgba(240,246,252,.05); border-radius: 6px;"
        "  padding: 6px 16px; font-family: '%1'; font-size: %2pt; font-weight: bold;"
        "}"
    ).arg(FONT_FAMILY).arg(FS_BTN);
}

static QString linkBtnStyle()
{
    return QString(
        "QPushButton { color: %1; border: none; background: transparent;"
        " font-family: '%2'; font-size: %3pt; }"
        "QPushButton:hover { color: %4; text-decoration: underline; }"
    ).arg(GH_BLUE, FONT_FAMILY).arg(FS_BTN).arg(GH_BLUE_H);
}

static QString errorStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_RED, FONT_FAMILY).arg(FS_SMALL);
}

static QString hintStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_MUTED, FONT_FAMILY).arg(FS_SMALL);
}

static QString successStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_GREEN_H, FONT_FAMILY).arg(FS_SMALL);
}

// ── Constructor ──────────────────────────────────────────────────────────────
VerifyWidget::VerifyWidget(QWidget *parent)
    : QWidget(parent), isLocked(false), m_attemptsLeft(3)
{
    setStyleSheet(QString(
        "QWidget { background-color: %1; color: %2; font-family: '%3'; font-size: 11pt; }")
        .arg(GH_BG, GH_TEXT, FONT_FAMILY));

    setupUI();

    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &VerifyWidget::onLockTimerFired);
}

VerifyWidget::~VerifyWidget() {}

void VerifyWidget::setupUI()
{
    auto *outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->addStretch(1);

    auto *outerH = new QHBoxLayout();
    outerH->addStretch(1);

    QWidget *card = new QWidget(this);
    card->setFixedWidth(420);
    card->setStyleSheet(QString(
        "QWidget { background-color: %1; border: 1px solid %2; border-radius: 10px; }")
        .arg(GH_CARD, GH_BORDER));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(8);

    // Заголовок
    auto *title = new QLabel(QString::fromUtf8("Подтверждение входа"), card);
    QFont tf(FONT_FAMILY, FS_TITLE, QFont::Bold);
    title->setFont(tf);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QString("QLabel { color: %1; border: none; background: transparent; }").arg(GH_TEXT));
    layout->addWidget(title);

    auto *sep = new QFrame(card);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(QString("QFrame { background: %1; border: none; max-height: 1px; }").arg(GH_BORDER));
    layout->addWidget(sep);
    layout->addSpacing(4);

    hintLabel = new QLabel(QString::fromUtf8("Введите код из письма:"), card);
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setWordWrap(true);
    hintLabel->setStyleSheet(hintStyle());
    layout->addWidget(hintLabel);
    layout->addSpacing(4);

    // OTP-ввод
    otpInput = new OtpInput(card);
    layout->addWidget(otpInput, 0, Qt::AlignCenter);
    connect(otpInput, &OtpInput::completed,  this, &VerifyWidget::onCodeCompleted);
    // ESC из OtpInput → назад
    connect(otpInput, &OtpInput::escPressed, this, &VerifyWidget::onBackClicked);

    statusLabel = new QLabel(card);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    statusLabel->setStyleSheet(errorStyle());
    statusLabel->hide();
    layout->addWidget(statusLabel);
    layout->addSpacing(4);

    verifyBtn = new QPushButton(QString::fromUtf8("Подтвердить"), card);
    verifyBtn->setEnabled(false);
    verifyBtn->setDefault(true);
    verifyBtn->setAutoDefault(true);
    verifyBtn->setMinimumHeight(38);
    verifyBtn->setStyleSheet(primaryBtnStyle(false));
    connect(verifyBtn, &QPushButton::clicked, this, &VerifyWidget::onVerifyClicked);
    layout->addWidget(verifyBtn);

    backBtn = new QPushButton(QString::fromUtf8("Назад"), card);
    backBtn->setStyleSheet(linkBtnStyle());
    connect(backBtn, &QPushButton::clicked, this, &VerifyWidget::onBackClicked);
    layout->addWidget(backBtn, 0, Qt::AlignCenter);

    outerH->addWidget(card);
    outerH->addStretch(1);
    outerV->addLayout(outerH);
    outerV->addStretch(1);
}

void VerifyWidget::setLogin(const QString &login, const QString &codeHash)
{
    m_login        = login;
    m_codeHash     = codeHash;
    m_attemptsLeft = 3;
    isLocked       = false;
    otpInput->clear();          // clear() сам сбрасывает setError(false)
    otpInput->setEnabled(true);
    updateVerifyBtn();
    statusLabel->hide();
    hintLabel->setText(QString::fromUtf8("Введите код из письма:"));
    hintLabel->setStyleSheet(hintStyle());
}

void VerifyWidget::clearFields()
{
    otpInput->clear();
    otpInput->setError(false);
    statusLabel->hide();
    updateVerifyBtn();
    m_codeHash.clear();
    m_login.clear();
    m_attemptsLeft = 3;
    isLocked       = false;
}

void VerifyWidget::updateVerifyBtn()
{
    const bool ok = !isLocked && otpInput->isComplete();
    verifyBtn->setEnabled(ok);
    verifyBtn->setStyleSheet(primaryBtnStyle(ok));
}

void VerifyWidget::onCodeCompleted(const QString &)
{
    otpInput->setError(false);
    updateVerifyBtn();
    // автоматически нажимаем кнопку при заполнении
    if (!isLocked)
        onVerifyClicked();
}

void VerifyWidget::onVerifyClicked()
{
    if (isLocked) return;
    const QString c = otpInput->code();
    if (c.length() != 6) return;

    verifyBtn->setEnabled(false);
    verifyBtn->setStyleSheet(primaryBtnStyle(false));

    const QString entered = QString::fromLatin1(
        QCryptographicHash::hash(c.toUtf8(), QCryptographicHash::Sha256).toHex());

    if (entered == m_codeHash) {
        otpInput->setError(false);
        statusLabel->setStyleSheet(successStyle());
        statusLabel->setText(QString::fromUtf8("Код верен!"));
        statusLabel->show();
        otpInput->setEnabled(false);
        emit verificationSuccess(m_login);
        return;
    }

    // Неверный код — шейкаем боксы красным
    otpInput->setError(true);
    m_attemptsLeft--;
    if (m_attemptsLeft > 0) {
        statusLabel->setStyleSheet(errorStyle());
        statusLabel->setText(
            QString::fromUtf8("Неверный код. Осталось попыток: %1.").arg(m_attemptsLeft));
        statusLabel->show();
        // через короткую паузу очищаем и даём ввести снова
        QTimer::singleShot(600, this, [this]() {
            otpInput->clear();   // clear() сбрасывает setError внутри
            // ошибку убираем после очистки, чтобы statusLabel оставался
            updateVerifyBtn();
        });
    } else {
        applyLock(30, QString::fromUtf8("Превышен лимит попыток. Блокировка на 30 сек."));
    }
}

void VerifyWidget::onBackClicked()
{
    clearFields();
    emit backToAuth();
}

// ESC обрабатывается через OtpInput::escPressed — keyPressEvent не нужен,
// но оставляем для случаев, когда фокус не в OtpInput.
void VerifyWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        onBackClicked();
    else
        QWidget::keyPressEvent(e);
}

void VerifyWidget::applyLock(int seconds, const QString &message)
{
    isLocked = true;
    otpInput->setEnabled(false);
    verifyBtn->setEnabled(false);
    verifyBtn->setStyleSheet(primaryBtnStyle(false));
    statusLabel->setStyleSheet(errorStyle());
    statusLabel->setText(message);
    statusLabel->show();
    lockTimer->start(seconds * 1000);
}

void VerifyWidget::onLockTimerFired()
{
    isLocked       = false;
    m_attemptsLeft = 3;
    otpInput->setEnabled(true);
    otpInput->clear();
    statusLabel->hide();
    updateVerifyBtn();
}

void VerifyWidget::onVerifyResponseReceived(const QString &) {}
