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
#include <climits>

#define GH_BG          "#0d1117"
#define GH_CARD        "#161b22"
#define GH_BORDER      "#30363d"
#define GH_TEXT        "#e6edf3"
#define GH_MUTED       "#8b949e"
#define GH_GREEN       "#238636"
#define GH_GREEN_H     "#2ea043"
#define GH_BLUE        "#388bfd"
#define GH_BLUE_H      "#58a6ff"
#define GH_RED         "#f85149"
#define GH_INPUT_BG    "#0d1117"
#define GH_BTN_GHOST   "#21262d"
#define GH_BTN_GHOST_H "#30363d"

#define FONT_FAMILY     "Segoe UI"
#define FONT_SIZE_TITLE 16
#define FONT_SIZE_BTN   11
#define FONT_SIZE_INPUT 11
#define FONT_SIZE_SMALL 9

// 3 wrong attempts -> 30s -> 5min -> 10min -> permanent
static const int LOCK_DURATIONS_SEC[] = {30, 5*60, 10*60, INT_MAX};
static const int LOCK_LEVELS_COUNT    = 4;

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

    setStyleSheet(QString("QWidget { background-color: %1; color: %2; font-family: '%3'; font-size: %4pt; }")
                  .arg(GH_BG).arg(GH_TEXT).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT));

    setupUI();
}

AuthWidget::~AuthWidget() {}

void AuthWidget::clearFields()
{
    loginEdit->clear();
    passwordEdit->clear();
    passwordEdit->setEchoMode(QLineEdit::Password);
    statusLabel->hide();
    attemptsLabel->hide();
    loginBtn->setEnabled(true);
    failedAttempts   = 0;
    lockLevel        = 0;
    isLocked         = false;
    m_waitingForAuth = false;
    if (lockTimer->isActive())
        lockTimer->stop();
}

static QString inputStyle()
{
    return QString(
        "QLineEdit {"
        "  background-color: %1; color: %2; border: 1px solid %3;"
        "  border-radius: 6px; padding: 6px 10px;"
        "  font-family: '%4'; font-size: %5pt;"
        "}"
        "QLineEdit:focus { border-color: %6; }"
    ).arg(GH_INPUT_BG).arg(GH_TEXT).arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT).arg(GH_BLUE);
}

static QString primaryBtnStyle()
{
    return QString(
        "QPushButton {"
        "  background-color: %1; color: #ffffff;"
        "  border: 1px solid rgba(240,246,252,0.1);"
        "  border-radius: 6px; padding: 6px 16px;"
        "  font-family: '%3'; font-size: %4pt; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:disabled { background-color: rgba(35,134,54,0.4); color: rgba(255,255,255,0.4); }"
    ).arg(GH_GREEN).arg(GH_GREEN_H).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString ghostBtnStyle()
{
    return QString(
        "QPushButton {"
        "  background-color: %1; color: %3; border: 1px solid %4;"
        "  border-radius: 6px; padding: 5px 14px;"
        "  font-family: '%5'; font-size: %6pt;"
        "}"
        "QPushButton:hover { background-color: %2; }"
    ).arg(GH_BTN_GHOST).arg(GH_BTN_GHOST_H).arg(GH_TEXT).arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString linkBtnStyle()
{
    return QString(
        "QPushButton { color: %1; border: none; background: transparent;"
        "  font-family: '%2'; font-size: %3pt;"
        "}"
        "QPushButton:hover { color: %4; text-decoration: underline; }"
    ).arg(GH_BLUE).arg(FONT_FAMILY).arg(FONT_SIZE_BTN).arg(GH_BLUE_H);
}

void AuthWidget::setupUI()
{
    QVBoxLayout *outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->addStretch(1);

    QHBoxLayout *outerH = new QHBoxLayout();
    outerH->addStretch(1);

    QWidget *card = new QWidget(this);
    card->setFixedWidth(340);
    card->setStyleSheet(QString(
        "QWidget { background-color: %1; border: 1px solid %2; border-radius: 10px; }"
    ).arg(GH_CARD).arg(GH_BORDER));

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 28, 28, 28);
    cardLayout->setSpacing(10);

    QLabel *titleLabel = new QLabel(
        QString::fromUtf8("\xd0\x90\xd0\xb2\xd1\x82\xd0\xbe\xd1\x80\xd0\xb8\xd0\xb7\xd0\xb0\xd1\x86\xd0\xb8\xd1\x8f"), card);
    QFont titleFont(FONT_FAMILY, FONT_SIZE_TITLE, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString("QLabel { color: %1; border: none; }").arg(GH_TEXT));
    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(6);

    loginEdit = new QLineEdit(card);
    loginEdit->setPlaceholderText(QString::fromUtf8("\xd0\x9b\xd0\xbe\xd0\xb3\xd0\xb8\xd0\xbd"));
    loginEdit->setMinimumHeight(38);
    loginEdit->setStyleSheet(inputStyle());
    cardLayout->addWidget(loginEdit);

    QHBoxLayout *passRow = new QHBoxLayout();
    passRow->setSpacing(6);
    passwordEdit = new QLineEdit(card);
    passwordEdit->setPlaceholderText(QString::fromUtf8("\xd0\x9f\xd0\xb0\xd1\x80\xd0\xbe\xd0\xbb\xd1\x8c"));
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(38);
    passwordEdit->setStyleSheet(inputStyle());
    passRow->addWidget(passwordEdit);

    togglePasswordBtn = new QPushButton("\U0001f441", card);
    togglePasswordBtn->setFixedSize(38, 38);
    togglePasswordBtn->setToolTip(
        QString::fromUtf8("\xd0\x9f\xd0\xbe\xd0\xba\xd0\xb0\xd0\xb7\xd0\xb0\xd1\x82\xd1\x8c/\xd1\x81\xd0\xba\xd1\x80\xd1\x8b\xd1\x82\xd1\x8c \xd0\xbf\xd0\xb0\xd1\x80\xd0\xbe\xd0\xbb\xd1\x8c"));
    togglePasswordBtn->setStyleSheet(ghostBtnStyle());
    connect(togglePasswordBtn, &QPushButton::clicked, this, &AuthWidget::onTogglePassword);
    passRow->addWidget(togglePasswordBtn);
    cardLayout->addLayout(passRow);

    statusLabel = new QLabel(card);
    statusLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }")
                               .arg(GH_RED).arg(FONT_SIZE_SMALL));
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    cardLayout->addWidget(statusLabel);

    attemptsLabel = new QLabel(card);
    attemptsLabel->setStyleSheet(QString("QLabel { color: %1; border: none; font-size: %2pt; }")
                                 .arg(GH_MUTED).arg(FONT_SIZE_SMALL));
    attemptsLabel->setAlignment(Qt::AlignCenter);
    attemptsLabel->hide();
    cardLayout->addWidget(attemptsLabel);

    cardLayout->addSpacing(4);

    loginBtn = new QPushButton(
        QString::fromUtf8("\xd0\x92\xd0\xbe\xd0\xb9\xd1\x82\xd0\xb8"), card);
    loginBtn->setMinimumHeight(38);
    loginBtn->setDefault(true);
    loginBtn->setAutoDefault(true);
    loginBtn->setStyleSheet(primaryBtnStyle());
    connect(loginBtn,     &QPushButton::clicked,    this, &AuthWidget::onLoginClicked);
    connect(loginEdit,    &QLineEdit::returnPressed, loginBtn, &QPushButton::click);
    connect(passwordEdit, &QLineEdit::returnPressed, loginBtn, &QPushButton::click);
    cardLayout->addWidget(loginBtn);

    QFrame *line = new QFrame(card);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(
        QString("QFrame { background: %1; border: none; max-height: 1px; }").arg(GH_BORDER));
    cardLayout->addWidget(line);

    registerBtn = new QPushButton(
        QString::fromUtf8("\xd0\xa0\xd0\xb5\xd0\xb3\xd0\xb8\xd1\x81\xd1\x82\xd1\x80\xd0\xb0\xd1\x86\xd0\xb8\xd1\x8f"), card);
    registerBtn->setFlat(true);
    registerBtn->setStyleSheet(linkBtnStyle());
    connect(registerBtn, &QPushButton::clicked, this, &AuthWidget::onRegisterClicked);
    cardLayout->addWidget(registerBtn, 0, Qt::AlignCenter);

    forgotBtn = new QPushButton(
        QString::fromUtf8("\xd0\x97\xd0\xb1\xd1\x8b\xd0\xbb\xd0\xb8 \xd0\xbf\xd0\xb0\xd1\x80\xd0\xbe\xd0\xbb\xd1\x8c?"), card);
    forgotBtn->setFlat(true);
    forgotBtn->setStyleSheet(linkBtnStyle());
    connect(forgotBtn, &QPushButton::clicked, this, &AuthWidget::onForgotClicked);
    cardLayout->addWidget(forgotBtn, 0, Qt::AlignCenter);

    outerH->addWidget(card);
    outerH->addStretch(1);
    outerV->addLayout(outerH);
    outerV->addStretch(1);
    setLayout(outerV);
}

void AuthWidget::onTogglePassword()
{
    passwordEdit->setEchoMode(
        passwordEdit->echoMode() == QLineEdit::Password
            ? QLineEdit::Normal
            : QLineEdit::Password);
}

void AuthWidget::applyLock(int durationSec, const QString &message)
{
    isLocked = true;
    loginBtn->setEnabled(false);
    statusLabel->setText(message);
    statusLabel->setStyleSheet(
        QString("QLabel { color: %1; border: none; font-size: %2pt; }")
        .arg(GH_RED).arg(FONT_SIZE_SMALL));
    statusLabel->show();
    attemptsLabel->hide();
    if (durationSec != INT_MAX)
        lockTimer->start(durationSec * 1000);
    // INT_MAX == permanent ban: timer is never started
}

void AuthWidget::onLockTimerFired()
{
    isLocked       = false;
    failedAttempts = 0;
    loginBtn->setEnabled(true);
    statusLabel->hide();
    attemptsLabel->hide();
}

// ─── onLoginClicked ──────────────────────────────────────────────────────────
//
// Flow:
//   1. Local validation (empty fields, locked, already waiting)
//   2. sendRequest() — SYNCHRONOUS, fast (just a DB lookup on the server)
//      Server returns "auth-" instantly if wrong, "auth_code_sent" if correct.
//      Email sending happens AFTER the server sends the reply, so the client
//      is NOT blocked by SMTP.
//   3a. auth_code_sent  → navigate to VerifyWidget immediately
//   3b. auth-           → count attempt, show error, possibly lock
void AuthWidget::onLoginClicked()
{
    if (isLocked) {
        int rem = lockTimer->isActive() ? lockTimer->remainingTime() / 1000 : 0;
        int m   = rem / 60;
        int s   = rem % 60;
        if (!lockTimer->isActive() && lockLevel >= LOCK_LEVELS_COUNT) {
            statusLabel->setText(
                QString::fromUtf8("\xd0\x90\xd0\xba\xd0\xba\xd0\xb0\xd1\x83\xd0\xbd\xd1\x82 \xd0\xb7\xd0\xb0\xd0\xb1\xd0\xbb\xd0\xbe\xd0\xba\xd0\xb8\xd1\x80\xd0\xbe\xd0\xb2\xd0\xb0\xd0\xbd \xd0\xbd\xd0\xb0\xd0\xb2\xd1\x81\xd0\xb5\xd0\xb3\xd0\xb4\xd0\xb0."));
        } else {
            statusLabel->setText(m > 0
                ? QString::fromUtf8("\xd0\x97\xd0\xb0\xd0\xb1\xd0\xbb\xd0\xbe\xd0\xba\xd0\xb8\xd1\x80\xd0\xbe\xd0\xb2\xd0\xb0\xd0\xbd\xd0\xbe. \xd0\x9e\xd1\x81\xd1\x82\xd0\xb0\xd0\xbb\xd0\xbe\xd1\x81\xd1\x8c: %1 \xd0\xbc\xd0\xb8\xd0\xbd %2 \xd1\x81\xd0\xb5\xd0\xba.").arg(m).arg(s)
                : QString::fromUtf8("\xd0\x97\xd0\xb0\xd0\xb1\xd0\xbb\xd0\xbe\xd0\xba\xd0\xb8\xd1\x80\xd0\xbe\xd0\xb2\xd0\xb0\xd0\xbd\xd0\xbe. \xd0\x9e\xd1\x81\xd1\x82\xd0\xb0\xd0\xbb\xd0\xbe\xd1\x81\xd1\x8c: %1 \xd1\x81\xd0\xb5\xd0\xba.").arg(rem));
        }
        statusLabel->setStyleSheet(
            QString("QLabel { color: %1; border: none; font-size: %2pt; }")
            .arg(GH_RED).arg(FONT_SIZE_SMALL));
        statusLabel->show();
        return;
    }

    if (m_waitingForAuth) return;

    const QString login    = loginEdit->text().trimmed().remove("||");
    const QString password = passwordEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        statusLabel->setText(
            QString::fromUtf8("\xd0\x92\xd0\xb2\xd0\xb5\xd0\xb4\xd0\xb8\xd1\x82\xd0\xb5 \xd0\xbb\xd0\xbe\xd0\xb3\xd0\xb8\xd0\xbd \xd0\xb8 \xd0\xbf\xd0\xb0\xd1\x80\xd0\xbe\xd0\xbb\xd1\x8c."));
        statusLabel->setStyleSheet(
            QString("QLabel { color: %1; border: none; font-size: %2pt; }")
            .arg(GH_RED).arg(FONT_SIZE_SMALL));
        statusLabel->show();
        return;
    }

    const QString passwordHash = QString::fromLatin1(
        QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

    // Disable button to prevent double-click
    m_waitingForAuth = true;
    loginBtn->setEnabled(false);
    statusLabel->hide();
    attemptsLabel->hide();

    // ── Synchronous request ──────────────────────────────────────────────────
    // sendRequest() returns as soon as the server sends back the first line.
    // The server sends the reply BEFORE starting the SMTP send, so this
    // returns in milliseconds (pure DB lookup time, ~1-5ms locally).
    const QString response = ClientSingleton::instance().sendRequest(
        QString("auth||%1||%2").arg(login, passwordHash));

    m_waitingForAuth = false;
    const QString r = response.trimmed();

    // ── auth_code_sent: credentials OK, go to code screen immediately ────────
    if (r == "auth_code_sent") {
        loginBtn->setEnabled(true);
        emit showVerifyAuth(login);
        return;
    }

    // ── auth-: wrong credentials ─────────────────────────────────────────────
    if (r == "auth-" || r.startsWith("auth-||")) {
        failedAttempts++;
        const int left = 3 - failedAttempts;

        if (failedAttempts >= 3) {
            const int idx    = qMin(lockLevel, LOCK_LEVELS_COUNT - 1);
            const int durSec = LOCK_DURATIONS_SEC[idx];
            lockLevel++;
            failedAttempts = 0;

            QString msg;
            if (durSec == INT_MAX)
                msg = QString::fromUtf8("\xd0\x90\xd0\xba\xd0\xba\xd0\xb0\xd1\x83\xd0\xbd\xd1\x82 \xd0\xb7\xd0\xb0\xd0\xb1\xd0\xbb\xd0\xbe\xd0\xba\xd0\xb8\xd1\x80\xd0\xbe\xd0\xb2\xd0\xb0\xd0\xbd \xd0\xbd\xd0\xb0\xd0\xb2\xd1\x81\xd0\xb5\xd0\xb3\xd0\xb4\xd0\xb0.");
            else if (durSec < 60)
                msg = QString::fromUtf8("\xd0\x9f\xd1\x80\xd0\xb5\xd0\xb2\xd1\x8b\xd1\x88\xd0\xb5\xd0\xbd \xd0\xbb\xd0\xb8\xd0\xbc\xd0\xb8\xd1\x82. \xd0\x91\xd0\xbb\xd0\xbe\xd0\xba\xd0\xb8\xd1\x80\xd0\xbe\xd0\xb2\xd0\xba\xd0\xb0 \xd0\xbd\xd0\xb0 30 \xd1\x81\xd0\xb5\xd0\xba.");
            else
                msg = QString::fromUtf8("\xd0\x9f\xd1\x80\xd0\xb5\xd0\xb2\xd1\x8b\xd1\x88\xd0\xb5\xd0\xbd \xd0\xbb\xd0\xb8\xd0\xbc\xd0\xb8\xd1\x82. \xd0\x91\xd0\xbb\xd0\xbe\xd0\xba\xd0\xb8\xd1\x80\xd0\xbe\xd0\xb2\xd0\xba\xd0\xb0 \xd0\xbd\xd0\xb0 %1 \xd0\xbc\xd0\xb8\xd0\xbd.").arg(durSec / 60);

            applyLock(durSec, msg);
        } else {
            statusLabel->setText(
                QString::fromUtf8("\xd0\x9d\xd0\xb5\xd0\xb2\xd0\xb5\xd1\x80\xd0\xbd\xd1\x8b\xd0\xb9 \xd0\xbb\xd0\xbe\xd0\xb3\xd0\xb8\xd0\xbd \xd0\xb8\xd0\xbb\xd0\xb8 \xd0\xbf\xd0\xb0\xd1\x80\xd0\xbe\xd0\xbb\xd1\x8c. \xd0\x9e\xd1\x81\xd1\x82\xd0\xb0\xd0\xbb\xd0\xbe\xd1\x81\xd1\x8c \xd0\xbf\xd0\xbf\xd1\x8b\xd1\x82\xd0\xbe\xd0\xba: %1.").arg(left));
            statusLabel->setStyleSheet(
                QString("QLabel { color: %1; border: none; font-size: %2pt; }")
                .arg(GH_RED).arg(FONT_SIZE_SMALL));
            statusLabel->show();
            loginBtn->setEnabled(true);
        }
        return;
    }

    // ── locked response from server side ─────────────────────────────────────
    if (r.startsWith("locked")) {
        const QStringList p = r.split("||");
        const int sec = p.size() >= 2 ? p[1].toInt() : 30;
        applyLock(sec, QString::fromUtf8(
            "\xd0\x90\xd0\xba\xd0\xba\xd0\xb0\xd1\x83\xd0\xbd\xd1\x82 \xd0\xb7\xd0\xb0\xd0\xb1\xd0\xbb\xd0\xbe\xd0\xba\xd0\xb8\xd1\x80\xd0\xbe\xd0\xb2\xd0\xb0\xd0\xbd. \xd0\x9f\xd0\xbe\xd0\xb4\xd0\xbe\xd0\xb6\xd0\xb4\xd0\xb8\xd1\x82\xd0\xb5."));
        return;
    }

    // ── Fallback: connection error or empty response ──────────────────────────
    statusLabel->setText(
        QString::fromUtf8("\xd0\x9e\xd1\x88\xd0\xb8\xd0\xb1\xd0\xba\xd0\xb0 \xd1\x81\xd0\xbe\xd0\xb5\xd0\xb4\xd0\xb8\xd0\xbd\xd0\xb5\xd0\xbd\xd0\xb8\xd1\x8f. \xd0\x9f\xd1\x80\xd0\xbe\xd0\xb2\xd0\xb5\xd1\x80\xd1\x8c\xd1\x82\xd0\xb5 \xd1\x81\xd0\xb5\xd1\x80\xd0\xb2\xd0\xb5\xd1\x80."));
    statusLabel->setStyleSheet(
        QString("QLabel { color: %1; border: none; font-size: %2pt; }")
        .arg(GH_RED).arg(FONT_SIZE_SMALL));
    statusLabel->show();
    loginBtn->setEnabled(true);
}

void AuthWidget::onRegisterClicked() { emit showRegister(); }
void AuthWidget::onForgotClicked()   { emit showReset(); }

// onAuthResponseReceived is intentionally empty — we use sendRequest() (sync),
// so we never need the async responseReceived signal for auth.
void AuthWidget::onAuthResponseReceived(const QString &) {}
