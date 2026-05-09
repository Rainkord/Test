/**
 * @file resetwidget.cpp
 * @brief \u0412\u043e\u0441\u0441\u0442\u0430\u043d\u043e\u0432\u043b\u0435\u043d\u0438\u0435 \u043f\u0430\u0440\u043e\u043b\u044f. \u0428\u0430\u0433 2 \u0438\u0441\u043f\u043e\u043b\u044c\u0437\u0443\u0435\u0442 OTP-\u0432\u0432\u043e\u0434 6 \u0431\u043e\u043a\u0441\u043e\u0432. ESC \u2192 \u0448\u0430\u0433 1.
 */

#include "resetwidget.h"
#include "otpinput.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QFont>
#include <QFrame>
#include <QKeyEvent>

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
#define FONT_FAMILY    "Segoe UI"
#define FS_TITLE       16
#define FS_BTN         11
#define FS_INPUT       11
#define FS_SMALL        9

// ── Style helpers ─────────────────────────────────────────────────────────────

static QString inputStyleR()
{
    return QString(
        "QLineEdit { background: %1; color: %2; border: 1px solid %3;"
        " border-radius: 6px; padding: 6px 10px;"
        " font-family: '%4'; font-size: %5pt; }"
        "QLineEdit:focus { border-color: %6; }"
    ).arg(GH_INPUT_BG, GH_TEXT, GH_BORDER, FONT_FAMILY).arg(FS_INPUT).arg(GH_BLUE);
}

static QString primaryR()
{
    return QString(
        "QPushButton { background: %1; color: #fff;"
        " border: 1px solid rgba(240,246,252,.1); border-radius: 6px;"
        " padding: 6px 16px; font-family: '%3'; font-size: %4pt; font-weight: bold; }"
        "QPushButton:hover { background: %2; }"
        "QPushButton:disabled { background: rgba(35,134,54,.4); color: rgba(255,255,255,.4); }"
    ).arg(GH_GREEN, GH_GREEN_H, FONT_FAMILY).arg(FS_BTN);
}

static QString secondaryR()
{
    return QString(
        "QPushButton { background: %1; color: %3;"
        " border: 1px solid %4; border-radius: 6px;"
        " padding: 6px 16px; font-family: '%5'; font-size: %6pt; font-weight: bold; }"
        "QPushButton:hover { background: %2; }"
    ).arg(GH_BTN_GHOST, GH_BTN_GHOST_H, GH_TEXT, GH_BORDER, FONT_FAMILY).arg(FS_BTN);
}

static QString ghostR()
{
    return QString(
        "QPushButton { background: %1; color: %3;"
        " border: 1px solid %4; border-radius: 6px;"
        " padding: 5px 14px; font-family: '%5'; font-size: %6pt; }"
        "QPushButton:hover { background: %2; }"
    ).arg(GH_BTN_GHOST, GH_BTN_GHOST_H, GH_TEXT, GH_BORDER, FONT_FAMILY).arg(FS_BTN);
}

static QString linkR()
{
    return QString(
        "QPushButton { color: %1; border: none; background: transparent;"
        " font-family: '%2'; font-size: %3pt; }"
        "QPushButton:hover { color: %4; text-decoration: underline; }"
    ).arg(GH_BLUE, FONT_FAMILY).arg(FS_BTN).arg(GH_BLUE_H);
}

static QString errorR()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_RED, FONT_FAMILY).arg(FS_SMALL);
}

static QString infoR()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_MUTED, FONT_FAMILY).arg(FS_SMALL);
}

static QString successR()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_GREEN_H, FONT_FAMILY).arg(FS_SMALL);
}

// ── Constructor ──────────────────────────────────────────────────────────────────
ResetWidget::ResetWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentStep(StepEmail)
    , failedAttempts(0)
    , lockLevel(0)
    , isLocked(false)
    , m_waitingForCodeHash(false)
    , m_waitingForSave(false)
{
    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &ResetWidget::onLockTimerFired);

    connect(&ClientSingleton::instance(), &ClientSingleton::responseReceived,
            this, &ResetWidget::onResetResponseReceived);

    setStyleSheet(QString(
        "QWidget { background: %1; color: %2; font-family: '%3'; font-size: %4pt; }")
        .arg(GH_BG, GH_TEXT, FONT_FAMILY).arg(FS_INPUT));

    setupUI();
}

ResetWidget::~ResetWidget() {}

void ResetWidget::setupUI()
{
    auto *outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->addStretch(1);

    auto *outerH = new QHBoxLayout();
    outerH->addStretch(1);

    QWidget *card = new QWidget(this);
    card->setFixedWidth(420);
    card->setStyleSheet(QString(
        "QWidget { background: %1; border: 1px solid %2; border-radius: 10px; }")
        .arg(GH_CARD, GH_BORDER));

    auto *mainL = new QVBoxLayout(card);
    mainL->setContentsMargins(28, 28, 28, 28);
    mainL->setSpacing(8);

    auto *titleLbl = new QLabel(QString::fromUtf8("\u0412\u043e\u0441\u0441\u0442\u0430\u043d\u043e\u0432\u043b\u0435\u043d\u0438\u0435 \u043f\u0430\u0440\u043e\u043b\u044f"), card);
    QFont tf(FONT_FAMILY, FS_TITLE, QFont::Bold);
    titleLbl->setFont(tf);
    titleLbl->setAlignment(Qt::AlignCenter);
    titleLbl->setStyleSheet(QString("QLabel { color: %1; border: none; }").arg(GH_TEXT));
    mainL->addWidget(titleLbl);

    auto *sep = new QFrame(card);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(QString("QFrame { background: %1; border: none; max-height: 1px; }").arg(GH_BORDER));
    mainL->addWidget(sep);
    mainL->addSpacing(4);

    // ── \u0428\u0430\u0433 1: email ────────────────────────────────────────────────────────────
    step1Widget = new QWidget(card);
    step1Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    auto *s1 = new QVBoxLayout(step1Widget);
    s1->setContentsMargins(0, 0, 0, 0);
    s1->setSpacing(6);

    emailEdit = new QLineEdit(step1Widget);
    emailEdit->setPlaceholderText("Email");
    emailEdit->setMinimumHeight(38);
    emailEdit->setStyleSheet(inputStyleR());
    s1->addWidget(emailEdit);
    connect(emailEdit, &QLineEdit::textChanged, this, &ResetWidget::onEmailTextChanged);

    emailErrorLabel = new QLabel(step1Widget);
    emailErrorLabel->setStyleSheet(errorR());
    emailErrorLabel->hide();
    s1->addWidget(emailErrorLabel);
    s1->addSpacing(4);

    auto *s1Btns = new QHBoxLayout();
    s1Btns->setSpacing(8);

    backBtn = new QPushButton(QString::fromUtf8("\u2190 \u041d\u0430\u0437\u0430\u0434"), step1Widget);
    backBtn->setMinimumHeight(38);
    backBtn->setStyleSheet(secondaryR());
    connect(backBtn, &QPushButton::clicked, this, &ResetWidget::onBackClicked);
    s1Btns->addWidget(backBtn);

    continueBtn = new QPushButton(QString::fromUtf8("\u041e\u0442\u043f\u0440\u0430\u0432\u0438\u0442\u044c \u043a\u043e\u0434"), step1Widget);
    continueBtn->setMinimumHeight(38);
    continueBtn->setEnabled(false);
    continueBtn->setDefault(true);
    continueBtn->setAutoDefault(true);
    continueBtn->setStyleSheet(primaryR());
    connect(continueBtn, &QPushButton::clicked, this, &ResetWidget::onContinueClicked);
    connect(emailEdit, &QLineEdit::returnPressed, continueBtn, &QPushButton::click);
    s1Btns->addWidget(continueBtn);

    s1->addLayout(s1Btns);
    mainL->addWidget(step1Widget);

    // ── \u0428\u0430\u0433 2: OTP-\u0432\u0432\u043e\u0434 \u043a\u043e\u0434\u0430 ─────────────────────────────────────────────────────
    step2Widget = new QWidget(card);
    step2Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    auto *s2 = new QVBoxLayout(step2Widget);
    s2->setContentsMargins(0, 0, 0, 0);
    s2->setSpacing(8);

    auto *codeHintLbl = new QLabel(
        QString::fromUtf8("\u0412\u0432\u0435\u0434\u0438\u0442\u0435 \u043a\u043e\u0434 \u0438\u0437 \u043f\u0438\u0441\u044c\u043c\u0430:"),
        step2Widget);
    codeHintLbl->setAlignment(Qt::AlignCenter);
    codeHintLbl->setStyleSheet(infoR());
    s2->addWidget(codeHintLbl);

    codeEdit = new OtpInput(step2Widget);
    s2->addWidget(codeEdit, 0, Qt::AlignCenter);
    connect(codeEdit, &OtpInput::completed,  this, &ResetWidget::onCodeTextChanged);
    connect(codeEdit, &OtpInput::escPressed, this, [this]() { showStep(StepEmail); });

    codeErrorLabel = new QLabel(step2Widget);
    codeErrorLabel->setStyleSheet(errorR());
    codeErrorLabel->setAlignment(Qt::AlignCenter);
    codeErrorLabel->hide();
    s2->addWidget(codeErrorLabel);

    codeStatusLabel = new QLabel(step2Widget);
    codeStatusLabel->setStyleSheet(successR());
    codeStatusLabel->setAlignment(Qt::AlignCenter);
    codeStatusLabel->hide();
    s2->addWidget(codeStatusLabel);
    s2->addSpacing(4);

    verifyCodeBtn = new QPushButton(QString::fromUtf8("\u041f\u043e\u0434\u0442\u0432\u0435\u0440\u0434\u0438\u0442\u044c"), step2Widget);
    verifyCodeBtn->setMinimumHeight(38);
    verifyCodeBtn->setEnabled(false);
    verifyCodeBtn->setDefault(true);
    verifyCodeBtn->setAutoDefault(true);
    verifyCodeBtn->setStyleSheet(primaryR());
    connect(verifyCodeBtn, &QPushButton::clicked, this, &ResetWidget::onVerifyCodeClicked);
    s2->addWidget(verifyCodeBtn);

    mainL->addWidget(step2Widget);

    // ── \u0428\u0430\u0433 3: \u043d\u043e\u0432\u044b\u0439 \u043f\u0430\u0440\u043e\u043b\u044c ──────────────────────────────────────────────────
    step3Widget = new QWidget(card);
    step3Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    auto *s3 = new QVBoxLayout(step3Widget);
    s3->setContentsMargins(0, 0, 0, 0);
    s3->setSpacing(6);

    auto *newPassHintLbl = new QLabel(
        QString::fromUtf8("\u0412\u0432\u0435\u0434\u0438\u0442\u0435 \u043d\u043e\u0432\u044b\u0439 \u043f\u0430\u0440\u043e\u043b\u044c:"),
        step3Widget);
    newPassHintLbl->setAlignment(Qt::AlignCenter);
    newPassHintLbl->setStyleSheet(infoR());
    s3->addWidget(newPassHintLbl);

    auto *np1Row = new QHBoxLayout();
    np1Row->setSpacing(6);
    newPasswordEdit = new QLineEdit(step3Widget);
    newPasswordEdit->setPlaceholderText(QString::fromUtf8("\u041d\u043e\u0432\u044b\u0439 \u043f\u0430\u0440\u043e\u043b\u044c (\u043c\u0438\u043d. 8 \u0441\u0438\u043c\u0432.)"));
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setMinimumHeight(38);
    newPasswordEdit->setStyleSheet(inputStyleR());
    np1Row->addWidget(newPasswordEdit);
    connect(newPasswordEdit, &QLineEdit::textChanged, this, &ResetWidget::onNewPasswordTextChanged);

    togglePassBtn1 = new QPushButton("\xF0\x9F\x91\x81", step3Widget);
    togglePassBtn1->setFixedSize(38, 38);
    togglePassBtn1->setStyleSheet(ghostR());
    connect(togglePassBtn1, &QPushButton::clicked, this, &ResetWidget::onTogglePassword1);
    np1Row->addWidget(togglePassBtn1);
    s3->addLayout(np1Row);

    newPasswordErrorLabel = new QLabel(step3Widget);
    newPasswordErrorLabel->setStyleSheet(errorR());
    newPasswordErrorLabel->hide();
    s3->addWidget(newPasswordErrorLabel);

    auto *np2Row = new QHBoxLayout();
    np2Row->setSpacing(6);
    confirmPasswordEdit = new QLineEdit(step3Widget);
    confirmPasswordEdit->setPlaceholderText(QString::fromUtf8("\u041f\u043e\u0434\u0442\u0432\u0435\u0440\u0434\u0438\u0442\u0435 \u043f\u0430\u0440\u043e\u043b\u044c"));
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setMinimumHeight(38);
    confirmPasswordEdit->setStyleSheet(inputStyleR());
    np2Row->addWidget(confirmPasswordEdit);
    connect(confirmPasswordEdit, &QLineEdit::textChanged, this, &ResetWidget::onConfirmPasswordTextChanged);

    togglePassBtn2 = new QPushButton("\xF0\x9F\x91\x81", step3Widget);
    togglePassBtn2->setFixedSize(38, 38);
    togglePassBtn2->setStyleSheet(ghostR());
    connect(togglePassBtn2, &QPushButton::clicked, this, &ResetWidget::onTogglePassword2);
    np2Row->addWidget(togglePassBtn2);
    s3->addLayout(np2Row);

    confirmErrorLabel = new QLabel(step3Widget);
    confirmErrorLabel->setStyleSheet(errorR());
    confirmErrorLabel->hide();
    s3->addWidget(confirmErrorLabel);
    s3->addSpacing(4);

    saveBtn = new QPushButton(QString::fromUtf8("\u0421\u043e\u0445\u0440\u0430\u043d\u0438\u0442\u044c \u043f\u0430\u0440\u043e\u043b\u044c"), step3Widget);
    saveBtn->setMinimumHeight(38);
    saveBtn->setEnabled(false);
    saveBtn->setDefault(true);
    saveBtn->setAutoDefault(true);
    saveBtn->setStyleSheet(primaryR());
    connect(saveBtn, &QPushButton::clicked, this, &ResetWidget::onSavePasswordClicked);
    connect(newPasswordEdit, &QLineEdit::returnPressed, saveBtn, &QPushButton::click);
    connect(confirmPasswordEdit, &QLineEdit::returnPressed, saveBtn, &QPushButton::click);
    s3->addWidget(saveBtn);

    mainL->addWidget(step3Widget);

    outerH->addWidget(card);
    outerH->addStretch(1);
    outerV->addLayout(outerH);
    outerV->addStretch(1);
    setLayout(outerV);

    showStep(StepEmail);
}

void ResetWidget::showStep(Step step)
{
    m_currentStep = step;
    step1Widget->setVisible(step == StepEmail);
    step2Widget->setVisible(step == StepCode);
    step3Widget->setVisible(step == StepPassword);

    if (step == StepCode) {
        codeEdit->clear();
        codeEdit->setEnabled(true);
    }
}

void ResetWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        switch (m_currentStep) {
        case StepCode:     showStep(StepEmail);    break;
        case StepPassword: showStep(StepCode);     break;
        case StepEmail:    emit backToAuth();       break;
        }
    } else {
        QWidget::keyPressEvent(e);
    }
}

bool ResetWidget::isEmailValid(const QString &email) const
{
    QRegularExpression re("^[a-zA-Z0-9._%+\\-]+@[a-zA-Z0-9.\\-]+\\.[a-zA-Z]{2,}$");
    return re.match(email).hasMatch();
}

void ResetWidget::validatePasswords()
{
    const QString np = newPasswordEdit->text();
    const QString cp = confirmPasswordEdit->text();
    bool ok = np.length() >= 8 && !cp.isEmpty() && np == cp;
    saveBtn->setEnabled(ok);
}

void ResetWidget::onEmailTextChanged(const QString &text)
{
    if (text.isEmpty()) { emailErrorLabel->hide(); continueBtn->setEnabled(false); return; }
    if (!isEmailValid(text)) {
        emailErrorLabel->setText(QString::fromUtf8("\u041d\u0435\u0432\u0435\u0440\u043d\u044b\u0439 \u0444\u043e\u0440\u043c\u0430\u0442"));
        emailErrorLabel->show();
        continueBtn->setEnabled(false);
    } else {
        emailErrorLabel->hide();
        continueBtn->setEnabled(true);
    }
}

void ResetWidget::onContinueClicked()
{
    if (!continueBtn->isEnabled()) return;
    m_email = emailEdit->text().trimmed();
    continueBtn->setEnabled(false);
    continueBtn->setText(QString::fromUtf8("\u041e\u0442\u043f\u0440\u0430\u0432\u043a\u0430..."));
    emailErrorLabel->hide();
    m_waitingForCodeHash = true;
    ClientSingleton::instance().sendRequestAsync(
        QString("reset_password||%1").arg(m_email));
}

void ResetWidget::onCodeTextChanged(const QString &)
{
    verifyCodeBtn->setEnabled(!isLocked && codeEdit->isComplete() && !m_pendingCodeHash.isEmpty());
}

void ResetWidget::onVerifyCodeClicked()
{
    if (isLocked) return;
    const QString c = codeEdit->code();
    if (c.length() != 6) return;

    verifyCodeBtn->setEnabled(false);

    const QString entered = QString::fromLatin1(
        QCryptographicHash::hash(c.toUtf8(), QCryptographicHash::Sha256).toHex());

    if (entered == m_pendingCodeHash) {
        codeErrorLabel->hide();
        codeStatusLabel->setText(QString::fromUtf8("\u041a\u043e\u0434 \u0432\u0435\u0440\u0435\u043d!"));
        codeStatusLabel->show();
        newPasswordEdit->clear();
        confirmPasswordEdit->clear();
        newPasswordErrorLabel->hide();
        confirmErrorLabel->hide();
        saveBtn->setEnabled(false);
        showStep(StepPassword);
        return;
    }

    failedAttempts++;
    if (failedAttempts < 4) {
        codeErrorLabel->setText(
            QString::fromUtf8("\u041d\u0435\u0432\u0435\u0440\u043d\u044b\u0439 \u043a\u043e\u0434. \u041e\u0441\u0442\u0430\u043b\u043e\u0441\u044c: %1.").arg(4 - failedAttempts));
        codeErrorLabel->show();
        codeEdit->clear();
        verifyCodeBtn->setEnabled(false);
    } else {
        lockLevel++;
        failedAttempts = 0;
        const int lockMin = (lockLevel == 1) ? 0 : (lockLevel == 2) ? 5 : 30;
        applyLock(lockMin,
            lockMin == 0
                ? QString::fromUtf8("\u041f\u0440\u0435\u0432\u044b\u0448\u0435\u043d \u043b\u0438\u043c\u0438\u0442. \u0411\u043b\u043e\u043a\u0438\u0440\u043e\u0432\u043a\u0430 30 \u0441\u0435\u043a.")
                : QString::fromUtf8("\u041f\u0440\u0435\u0432\u044b\u0448\u0435\u043d \u043b\u0438\u043c\u0438\u0442. \u0411\u043b\u043e\u043a\u0438\u0440\u043e\u0432\u043a\u0430 %1 \u043c\u0438\u043d.").arg(lockMin));
    }
}

void ResetWidget::onNewPasswordTextChanged(const QString &text)
{
    if (!text.isEmpty() && text.length() < 8) {
        newPasswordErrorLabel->setText(QString::fromUtf8("\u041c\u0438\u043d\u0438\u043c\u0443\u043c 8 \u0441\u0438\u043c\u0432."));
        newPasswordErrorLabel->show();
    } else {
        newPasswordErrorLabel->hide();
    }
    if (!confirmPasswordEdit->text().isEmpty() && confirmPasswordEdit->text() != text) {
        confirmErrorLabel->setText(QString::fromUtf8("\u041f\u0430\u0440\u043e\u043b\u0438 \u043d\u0435 \u0441\u043e\u0432\u043f\u0430\u0434\u0430\u044e\u0442"));
        confirmErrorLabel->show();
    } else {
        confirmErrorLabel->hide();
    }
    validatePasswords();
}

void ResetWidget::onConfirmPasswordTextChanged(const QString &text)
{
    if (!text.isEmpty() && text != newPasswordEdit->text()) {
        confirmErrorLabel->setText(QString::fromUtf8("\u041f\u0430\u0440\u043e\u043b\u0438 \u043d\u0435 \u0441\u043e\u0432\u043f\u0430\u0434\u0430\u044e\u0442"));
        confirmErrorLabel->show();
    } else {
        confirmErrorLabel->hide();
    }
    validatePasswords();
}

void ResetWidget::onTogglePassword1()
{
    newPasswordEdit->setEchoMode(newPasswordEdit->echoMode() == QLineEdit::Password
                                     ? QLineEdit::Normal : QLineEdit::Password);
}

void ResetWidget::onTogglePassword2()
{
    confirmPasswordEdit->setEchoMode(confirmPasswordEdit->echoMode() == QLineEdit::Password
                                         ? QLineEdit::Normal : QLineEdit::Password);
}

void ResetWidget::onSavePasswordClicked()
{
    if (!saveBtn->isEnabled()) return;
    saveBtn->setEnabled(false);
    saveBtn->setText(QString::fromUtf8("\u0421\u043e\u0445\u0440\u0430\u043d\u0435\u043d\u0438\u0435..."));

    const QString hash = QString::fromLatin1(
        QCryptographicHash::hash(
            newPasswordEdit->text().toUtf8(), QCryptographicHash::Sha256).toHex());

    m_waitingForSave = true;
    ClientSingleton::instance().sendRequestAsync(
        QString("set_new_password||%1||%2").arg(m_email, hash));
}

void ResetWidget::onBackClicked()
{
    emit backToAuth();
}

void ResetWidget::onLockTimerFired()
{
    isLocked = false;
    codeEdit->setEnabled(true);
    codeEdit->clear();
    verifyCodeBtn->setEnabled(false);
    codeErrorLabel->hide();
}

void ResetWidget::applyLock(int minutes, const QString &message)
{
    isLocked = true;
    codeEdit->setEnabled(false);
    verifyCodeBtn->setEnabled(false);
    codeErrorLabel->setText(message);
    codeErrorLabel->show();
    lockTimer->start(minutes == 0 ? 30 * 1000 : minutes * 60 * 1000);
}

void ResetWidget::onResetResponseReceived(const QString &response)
{
    QString r = response.trimmed();
    if (r.isEmpty()) return;

    if (m_waitingForCodeHash) {
        m_waitingForCodeHash = false;
        continueBtn->setText(QString::fromUtf8("\u041e\u0442\u043f\u0440\u0430\u0432\u0438\u0442\u044c \u043a\u043e\u0434"));
        continueBtn->setEnabled(isEmailValid(emailEdit->text()));

        if (r.startsWith("reset_code_sent")) {
            const QStringList parts = r.split("||");
            m_pendingCodeHash = (parts.size() >= 2) ? parts[1].trimmed() : QString();
            failedAttempts = 0;
            isLocked = false;
            showStep(StepCode);
        } else if (r == "user_not_found" || r == "email_not_found") {
            emailErrorLabel->setText(QString::fromUtf8("\u041f\u043e\u043b\u044c\u0437\u043e\u0432\u0430\u0442\u0435\u043b\u044c \u0441 \u0442\u0430\u043a\u0438\u043c email \u043d\u0435 \u043d\u0430\u0439\u0434\u0435\u043d"));
            emailErrorLabel->show();
        } else {
            emailErrorLabel->setText(QString::fromUtf8("\u041e\u0448\u0438\u0431\u043a\u0430 \u0441\u0435\u0440\u0432\u0435\u0440\u0430. \u041f\u043e\u043f\u0440\u043e\u0431\u0443\u0439\u0442\u0435 \u0441\u043d\u043e\u0432\u0430."));
            emailErrorLabel->show();
        }
        return;
    }

    if (m_waitingForSave) {
        m_waitingForSave = false;
        if (r == "password_changed") {
            emit resetSuccess();
        } else {
            saveBtn->setEnabled(true);
            saveBtn->setText(QString::fromUtf8("\u0421\u043e\u0445\u0440\u0430\u043d\u0438\u0442\u044c \u043f\u0430\u0440\u043e\u043b\u044c"));
            confirmErrorLabel->setText(QString::fromUtf8("\u041e\u0448\u0438\u0431\u043a\u0430 \u0441\u0435\u0440\u0432\u0435\u0440\u0430. \u041f\u043e\u043f\u0440\u043e\u0431\u0443\u0439\u0442\u0435 \u0441\u043d\u043e\u0432\u0430."));
            confirmErrorLabel->show();
        }
    }
}
