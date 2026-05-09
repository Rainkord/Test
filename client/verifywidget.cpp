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
#include <QFont>
#include <QFrame>

// ── GitHub dark palette ──────────────────────────────────────────────
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

#define FONT_FAMILY      "Segoe UI"
#define FONT_SIZE_TITLE  16
#define FONT_SIZE_BTN    11
#define FONT_SIZE_INPUT  11
#define FONT_SIZE_SMALL   9

static QString vInputStyle()
{
    return QString(
        "QLineEdit {"
        "  background-color: %1; color: %2;"
        "  border: 1px solid %3; border-radius: 6px;"
        "  padding: 6px 10px; font-family: '%4'; font-size: %5pt;"
        "}"
        "QLineEdit:focus { border-color: %6; }"
    ).arg(GH_INPUT_BG, GH_TEXT, GH_BORDER, FONT_FAMILY)
     .arg(FONT_SIZE_INPUT).arg(GH_BLUE);
}

static QString vCodeInputStyle()
{
    // Идентичный стиль поля кода в regwidget / resetwidget
    return vInputStyle() + "QLineEdit { letter-spacing: 4px; }";
}

static QString vPrimaryBtnStyle(bool enabled)
{
    if (enabled)
        return QString(
            "QPushButton {"
            "  background-color: %1; color: #ffffff;"
            "  border: 1px solid rgba(240,246,252,0.1); border-radius: 6px;"
            "  padding: 6px 16px; font-family: '%3'; font-size: %4pt; font-weight: bold;"
            "}"
            "QPushButton:hover { background-color: %2; }"
        ).arg(GH_GREEN, GH_GREEN_H, FONT_FAMILY).arg(FONT_SIZE_BTN);
    return QString(
        "QPushButton {"
        "  background-color: rgba(35,134,54,0.35); color: rgba(255,255,255,0.4);"
        "  border: 1px solid rgba(240,246,252,0.05); border-radius: 6px;"
        "  padding: 6px 16px; font-family: '%1'; font-size: %2pt; font-weight: bold;"
        "}"
    ).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString vLinkBtnStyle()
{
    return QString(
        "QPushButton { color: %1; border: none; background: transparent;"
        " font-family: '%2'; font-size: %3pt; }"
        "QPushButton:hover { color: %4; text-decoration: underline; }"
    ).arg(GH_BLUE, FONT_FAMILY).arg(FONT_SIZE_BTN).arg(GH_BLUE_H);
}

static QString vErrorStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_RED, FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

static QString vHintStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_MUTED, FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

static QString vSuccessStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_GREEN_H, FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

// ── Constructor ────────────────────────────────────────────────────────────────────────
VerifyWidget::VerifyWidget(QWidget *parent)
    : QWidget(parent)
    , isLocked(false)
    , m_attemptsLeft(3)
{
    setStyleSheet(QString(
        "QWidget { background-color: %1; color: %2; font-family: '%3'; font-size: %4pt; }")
        .arg(GH_BG, GH_TEXT, FONT_FAMILY).arg(FONT_SIZE_INPUT));

    setupUI();

    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &VerifyWidget::onLockTimerFired);
}

VerifyWidget::~VerifyWidget() {}

// ── setupUI ──────────────────────────────────────────────────────────────────────────
void VerifyWidget::setupUI()
{
    auto *outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->addStretch(1);

    auto *outerH = new QHBoxLayout();
    outerH->addStretch(1);

    QWidget *card = new QWidget(this);
    card->setFixedWidth(380);
    card->setStyleSheet(QString(
        "QWidget { background-color: %1; border: 1px solid %2; border-radius: 10px; }")
        .arg(GH_CARD, GH_BORDER));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(8);

    // Заголовок
    auto *title = new QLabel(QString::fromUtf8("Подтверждение входа"), card);
    QFont tf(FONT_FAMILY, FONT_SIZE_TITLE, QFont::Bold);
    title->setFont(tf);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QString("QLabel { color: %1; border: none; background: transparent; }").arg(GH_TEXT));
    layout->addWidget(title);

    // Разделитель
    auto *sep = new QFrame(card);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(QString("QFrame { background: %1; border: none; max-height: 1px; }").arg(GH_BORDER));
    layout->addWidget(sep);
    layout->addSpacing(4);

    // Подсказка
    hintLabel = new QLabel(QString::fromUtf8("Код отправлен на ваш email."), card);
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setWordWrap(true);
    hintLabel->setStyleSheet(vHintStyle());
    layout->addWidget(hintLabel);

    // Поле кода — центрирование + letter-spacing, как в reg/reset
    codeEdit = new QLineEdit(card);
    codeEdit->setPlaceholderText(QString::fromUtf8("Код из письма (6 цифр)"));
    codeEdit->setMaxLength(6);
    codeEdit->setMinimumHeight(38);
    codeEdit->setAlignment(Qt::AlignCenter);
    codeEdit->setStyleSheet(vCodeInputStyle());
    connect(codeEdit, &QLineEdit::textChanged, this, &VerifyWidget::onCodeTextChanged);
    connect(codeEdit, &QLineEdit::returnPressed, this, &VerifyWidget::onVerifyClicked);
    layout->addWidget(codeEdit);

    // Статус
    statusLabel = new QLabel(card);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    statusLabel->setStyleSheet(vErrorStyle());
    statusLabel->hide();
    layout->addWidget(statusLabel);

    // Кнопка подтвердить
    verifyBtn = new QPushButton(QString::fromUtf8("Подтвердить"), card);
    verifyBtn->setEnabled(false);
    verifyBtn->setDefault(true);
    verifyBtn->setAutoDefault(true);
    verifyBtn->setMinimumHeight(38);
    verifyBtn->setStyleSheet(vPrimaryBtnStyle(false));
    connect(verifyBtn, &QPushButton::clicked, this, &VerifyWidget::onVerifyClicked);
    layout->addWidget(verifyBtn);

    // Кнопка назад
    backBtn = new QPushButton(QString::fromUtf8("Назад"), card);
    backBtn->setStyleSheet(vLinkBtnStyle());
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
    codeEdit->clear();
    codeEdit->setEnabled(true);
    verifyBtn->setEnabled(false);
    verifyBtn->setStyleSheet(vPrimaryBtnStyle(false));
    statusLabel->hide();
    hintLabel->setText(QString::fromUtf8("Код отправлен на ваш email."));
    hintLabel->setStyleSheet(vHintStyle());
}

void VerifyWidget::clearFields()
{
    codeEdit->clear();
    statusLabel->hide();
    verifyBtn->setEnabled(false);
    verifyBtn->setStyleSheet(vPrimaryBtnStyle(false));
    m_codeHash.clear();
    m_login.clear();
    m_attemptsLeft = 3;
    isLocked       = false;
}

void VerifyWidget::onCodeTextChanged(const QString &text)
{
    const bool ok = !isLocked && text.length() == 6;
    verifyBtn->setEnabled(ok);
    verifyBtn->setStyleSheet(vPrimaryBtnStyle(ok));
}

void VerifyWidget::onVerifyClicked()
{
    if (isLocked) return;
    verifyBtn->setEnabled(false);
    verifyBtn->setStyleSheet(vPrimaryBtnStyle(false));

    const QString code = codeEdit->text().trimmed();
    if (code.length() != 6) {
        statusLabel->setStyleSheet(vErrorStyle());
        statusLabel->setText(QString::fromUtf8("Введите 6-значный код."));
        statusLabel->show();
        verifyBtn->setEnabled(true);
        verifyBtn->setStyleSheet(vPrimaryBtnStyle(true));
        return;
    }

    const QString entered = QString::fromLatin1(
        QCryptographicHash::hash(code.toUtf8(), QCryptographicHash::Sha256).toHex());

    if (entered == m_codeHash) {
        statusLabel->setStyleSheet(vSuccessStyle());
        statusLabel->setText(QString::fromUtf8("Код верен!"));
        statusLabel->show();
        codeEdit->setEnabled(false);
        emit verificationSuccess(m_login);
        return;
    }

    m_attemptsLeft--;
    if (m_attemptsLeft > 0) {
        statusLabel->setStyleSheet(vErrorStyle());
        statusLabel->setText(
            QString::fromUtf8("Неверный код. Осталось попыток: %1.").arg(m_attemptsLeft));
        statusLabel->show();
        verifyBtn->setEnabled(true);
        verifyBtn->setStyleSheet(vPrimaryBtnStyle(true));
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
    verifyBtn->setStyleSheet(vPrimaryBtnStyle(false));
    statusLabel->setStyleSheet(vErrorStyle());
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
    verifyBtn->setEnabled(false);
    verifyBtn->setStyleSheet(vPrimaryBtnStyle(false));
}

void VerifyWidget::onVerifyResponseReceived(const QString &) {}
