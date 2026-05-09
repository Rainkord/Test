#include "resetwidget.h"
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
#include <QRegularExpression>

// ── GitHub dark palette ─────────────────────────────────────────────────────
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
#define FONT_SIZE_SMALL  9

// ── Style helpers ───────────────────────────────────────────────────────────
static QString inputStyle()
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

static QString primaryBtnStyle(bool enabled)
{
    if (enabled)
        return QString(
            "QPushButton {"
            "  background-color: %1; color: #ffffff;"
            "  border: 1px solid rgba(240,246,252,0.1); border-radius: 6px;"
            "  padding: 6px 16px; font-family: '%3'; font-size: %4pt; font-weight: bold;"
            "}"
            "QPushButton:hover { background-color: %2; }"
        ).arg(GH_GREEN).arg(GH_GREEN_H).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
    return QString(
        "QPushButton {"
        "  background-color: rgba(35,134,54,0.35); color: rgba(255,255,255,0.4);"
        "  border: 1px solid rgba(240,246,252,0.05); border-radius: 6px;"
        "  padding: 6px 16px; font-family: '%1'; font-size: %2pt; font-weight: bold;"
        "}"
    ).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString blueBtnStyle(bool enabled)
{
    if (enabled)
        return QString(
            "QPushButton {"
            "  background-color: %1; color: #ffffff;"
            "  border: 1px solid rgba(240,246,252,0.1); border-radius: 6px;"
            "  padding: 6px 16px; font-family: '%3'; font-size: %4pt; font-weight: bold;"
            "}"
            "QPushButton:hover { background-color: %2; }"
        ).arg(GH_BLUE).arg(GH_BLUE_H).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
    return QString(
        "QPushButton {"
        "  background-color: rgba(56,139,253,0.3); color: rgba(255,255,255,0.4);"
        "  border: 1px solid rgba(240,246,252,0.05); border-radius: 6px;"
        "  padding: 6px 16px; font-family: '%1'; font-size: %2pt; font-weight: bold;"
        "}"
    ).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString ghostBtnStyle()
{
    return QString(
        "QPushButton {"
        "  background-color: %1; color: %3;"
        "  border: 1px solid %4; border-radius: 6px;"
        "  padding: 5px 10px; font-family: '%5'; font-size: %6pt;"
        "}"
        "QPushButton:hover { background-color: %2; }"
    ).arg(GH_BTN_GHOST).arg(GH_BTN_GHOST_H).arg(GH_TEXT)
     .arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString linkBtnStyle()
{
    return QString(
        "QPushButton { color: %1; border: none; background: transparent;"
        " font-family: '%2'; font-size: %3pt; }"
        "QPushButton:hover { color: %4; text-decoration: underline; }"
    ).arg(GH_BLUE).arg(FONT_FAMILY).arg(FONT_SIZE_BTN).arg(GH_BLUE_H);
}

static QString hintLabelStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_MUTED).arg(FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

static QString errorLabelStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_RED).arg(FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

// ── Constructor ─────────────────────────────────────────────────────────────
ResetWidget::ResetWidget(QWidget *parent)
    : QWidget(parent),
      m_currentStep(StepEmail),
      failedAttempts(0),
      lockLevel(0),
      isLocked(false),
      m_waitingForCodeHash(false),
      m_waitingForSave(false)
{
    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &ResetWidget::onLockTimerFired);

    connect(&ClientSingleton::instance(), &ClientSingleton::responseReceived,
            this, &ResetWidget::onResetResponseReceived);

    setStyleSheet(QString(
        "QWidget { background-color: %1; color: %2; font-family: '%3'; font-size: %4pt; }")
        .arg(GH_BG).arg(GH_TEXT).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT));

    setupUI();
}

ResetWidget::~ResetWidget() {}

// ── setupUI ──────────────────────────────────────────────────────────────────
void ResetWidget::setupUI()
{
    QVBoxLayout *outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->addStretch(1);

    QHBoxLayout *outerH = new QHBoxLayout();
    outerH->addStretch(1);

    QWidget *card = new QWidget(this);
    card->setFixedWidth(380);
    card->setStyleSheet(QString(
        "QWidget { background-color: %1; border: 1px solid %2; border-radius: 10px; }"
    ).arg(GH_CARD).arg(GH_BORDER));

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 24, 28, 24);
    cardLayout->setSpacing(8);

    QLabel *titleLabel = new QLabel(QString::fromUtf8("\u0412\u043e\u0441\u0441\u0442\u0430\u043d\u043e\u0432\u043b\u0435\u043d\u0438\u0435 \u043f\u0430\u0440\u043e\u043b\u044f"), card);
    QFont titleFont(FONT_FAMILY, FONT_SIZE_TITLE, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString("QLabel { color: %1; border: none; }").arg(GH_TEXT));
    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(8);

    // ── Шаг 1: email ────────────────────────────────────────────────────────
    step1Widget = new QWidget(card);
    step1Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *s1 = new QVBoxLayout(step1Widget);
    s1->setContentsMargins(0, 0, 0, 0);
    s1->setSpacing(6);

    QLabel *emailHint = new QLabel(QString::fromUtf8("\u0412\u0432\u0435\u0434\u0438\u0442\u0435 \u043f\u043e\u0447\u0442\u0443, \u043f\u0440\u0438\u0432\u044f\u0437\u0430\u043d\u043d\u0443\u044e \u043a \u0430\u043a\u043a\u0430\u0443\u043d\u0442\u0443:"), step1Widget);
    emailHint->setStyleSheet(hintLabelStyle());
    s1->addWidget(emailHint);

    emailEdit = new QLineEdit(step1Widget);
    emailEdit->setPlaceholderText("Email");
    emailEdit->setMinimumHeight(38);
    emailEdit->setStyleSheet(inputStyle());
    s1->addWidget(emailEdit);
    connect(emailEdit, &QLineEdit::textChanged, this, &ResetWidget::onEmailTextChanged);
    connect(emailEdit, &QLineEdit::returnPressed, this, [this]() {
        if (continueBtn->isEnabled()) continueBtn->click();
    });

    emailErrorLabel = new QLabel(step1Widget);
    emailErrorLabel->setStyleSheet(errorLabelStyle());
    emailErrorLabel->hide();
    s1->addWidget(emailErrorLabel);

    s1->addSpacing(4);
    continueBtn = new QPushButton(QString::fromUtf8("\u041f\u0440\u043e\u0434\u043e\u043b\u0436\u0438\u0442\u044c"), step1Widget);
    continueBtn->setMinimumHeight(38);
    continueBtn->setEnabled(false);
    continueBtn->setDefault(true);
    continueBtn->setAutoDefault(true);
    continueBtn->setStyleSheet(primaryBtnStyle(false));
    connect(continueBtn, &QPushButton::clicked, this, &ResetWidget::onContinueClicked);
    s1->addWidget(continueBtn);
    cardLayout->addWidget(step1Widget);

    // ── Шаг 2: код ──────────────────────────────────────────────────────────
    step2Widget = new QWidget(card);
    step2Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *s2 = new QVBoxLayout(step2Widget);
    s2->setContentsMargins(0, 0, 0, 0);
    s2->setSpacing(6);

    codeStatusLabel = new QLabel(step2Widget);
    codeStatusLabel->setStyleSheet(hintLabelStyle());
    codeStatusLabel->setAlignment(Qt::AlignCenter);
    codeStatusLabel->hide();
    s2->addWidget(codeStatusLabel);

    codeEdit = new QLineEdit(step2Widget);
    codeEdit->setPlaceholderText(QString::fromUtf8("\u0412\u0432\u0435\u0434\u0438\u0442\u0435 \u043a\u043e\u0434 \u0438\u0437 \u043f\u0438\u0441\u044c\u043c\u0430"));
    codeEdit->setMaxLength(6);
    codeEdit->setMinimumHeight(38);
    codeEdit->setAlignment(Qt::AlignCenter);
    codeEdit->setStyleSheet(inputStyle());
    s2->addWidget(codeEdit);
    connect(codeEdit, &QLineEdit::textChanged, this, &ResetWidget::onCodeTextChanged);
    connect(codeEdit, &QLineEdit::returnPressed, this, [this]() {
        if (verifyCodeBtn->isEnabled()) verifyCodeBtn->click();
    });

    codeErrorLabel = new QLabel(step2Widget);
    codeErrorLabel->setStyleSheet(errorLabelStyle());
    codeErrorLabel->setAlignment(Qt::AlignCenter);
    codeErrorLabel->hide();
    s2->addWidget(codeErrorLabel);

    s2->addSpacing(4);
    verifyCodeBtn = new QPushButton(QString::fromUtf8("\u041f\u043e\u0434\u0442\u0432\u0435\u0440\u0434\u0438\u0442\u044c \u043a\u043e\u0434"), step2Widget);
    verifyCodeBtn->setMinimumHeight(38);
    verifyCodeBtn->setEnabled(false);
    verifyCodeBtn->setDefault(true);
    verifyCodeBtn->setAutoDefault(true);
    verifyCodeBtn->setStyleSheet(blueBtnStyle(false));
    connect(verifyCodeBtn, &QPushButton::clicked, this, &ResetWidget::onVerifyCodeClicked);
    s2->addWidget(verifyCodeBtn);
    cardLayout->addWidget(step2Widget);

    // ── Шаг 3: новый пароль ─────────────────────────────────────────────────
    step3Widget = new QWidget(card);
    step3Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *s3 = new QVBoxLayout(step3Widget);
    s3->setContentsMargins(0, 0, 0, 0);
    s3->setSpacing(6);

    QLabel *passHint = new QLabel(QString::fromUtf8("\u041f\u0440\u0438\u0434\u0443\u043c\u0430\u0439\u0442\u0435 \u043d\u043e\u0432\u044b\u0439 \u043f\u0430\u0440\u043e\u043b\u044c:"), step3Widget);
    passHint->setStyleSheet(hintLabelStyle());
    s3->addWidget(passHint);

    QHBoxLayout *pass1Row = new QHBoxLayout();
    pass1Row->setSpacing(6);
    newPasswordEdit = new QLineEdit(step3Widget);
    newPasswordEdit->setPlaceholderText(QString::fromUtf8("\u041d\u043e\u0432\u044b\u0439 \u043f\u0430\u0440\u043e\u043b\u044c"));
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setMinimumHeight(38);
    newPasswordEdit->setStyleSheet(inputStyle());
    pass1Row->addWidget(newPasswordEdit);
    connect(newPasswordEdit, &QLineEdit::textChanged, this, &ResetWidget::onNewPasswordTextChanged);
    connect(newPasswordEdit, &QLineEdit::returnPressed, this, [this]() {
        if (saveBtn->isEnabled()) saveBtn->click();
    });

    togglePassBtn1 = new QPushButton("\xF0\x9F\x91\x81", step3Widget);
    togglePassBtn1->setFixedSize(38, 38);
    togglePassBtn1->setStyleSheet(ghostBtnStyle());
    connect(togglePassBtn1, &QPushButton::clicked, this, &ResetWidget::onTogglePassword1);
    pass1Row->addWidget(togglePassBtn1);
    s3->addLayout(pass1Row);

    newPasswordErrorLabel = new QLabel(step3Widget);
    newPasswordErrorLabel->setStyleSheet(errorLabelStyle());
    newPasswordErrorLabel->hide();
    s3->addWidget(newPasswordErrorLabel);

    QHBoxLayout *pass2Row = new QHBoxLayout();
    pass2Row->setSpacing(6);
    confirmPasswordEdit = new QLineEdit(step3Widget);
    confirmPasswordEdit->setPlaceholderText(QString::fromUtf8("\u041f\u043e\u0434\u0442\u0432\u0435\u0440\u0434\u0438\u0442\u0435 \u043f\u0430\u0440\u043e\u043b\u044c"));
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setMinimumHeight(38);
    confirmPasswordEdit->setStyleSheet(inputStyle());
    pass2Row->addWidget(confirmPasswordEdit);
    connect(confirmPasswordEdit, &QLineEdit::textChanged, this, &ResetWidget::onConfirmPasswordTextChanged);
    connect(confirmPasswordEdit, &QLineEdit::returnPressed, this, [this]() {
        if (saveBtn->isEnabled()) saveBtn->click();
    });

    togglePassBtn2 = new QPushButton("\xF0\x9F\x91\x81", step3Widget);
    togglePassBtn2->setFixedSize(38, 38);
    togglePassBtn2->setStyleSheet(ghostBtnStyle());
    connect(togglePassBtn2, &QPushButton::clicked, this, &ResetWidget::onTogglePassword2);
    pass2Row->addWidget(togglePassBtn2);
    s3->addLayout(pass2Row);

    confirmErrorLabel = new QLabel(step3Widget);
    confirmErrorLabel->setStyleSheet(errorLabelStyle());
    confirmErrorLabel->hide();
    s3->addWidget(confirmErrorLabel);

    s3->addSpacing(6);
    saveBtn = new QPushButton(QString::fromUtf8("\u0421\u043e\u0445\u0440\u0430\u043d\u0438\u0442\u044c \u043f\u0430\u0440\u043e\u043b\u044c"), step3Widget);
    saveBtn->setMinimumHeight(38);
    saveBtn->setEnabled(false);
    saveBtn->setDefault(true);
    saveBtn->setAutoDefault(true);
    saveBtn->setStyleSheet(primaryBtnStyle(false));
    connect(saveBtn, &QPushButton::clicked, this, &ResetWidget::onSavePasswordClicked);
    s3->addWidget(saveBtn);
    cardLayout->addWidget(step3Widget);

    cardLayout->addSpacing(8);
    QFrame *line = new QFrame(card);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("QFrame { background: %1; border: none; max-height: 1px; }").arg(GH_BORDER));
    cardLayout->addWidget(line);
    cardLayout->addSpacing(4);

    backBtn = new QPushButton(QString::fromUtf8("\u2190 \u041d\u0430\u0437\u0430\u0434 \u043a \u0432\u0445\u043e\u0434\u0443"), card);
    backBtn->setFlat(true);
    backBtn->setStyleSheet(linkBtnStyle());
    connect(backBtn, &QPushButton::clicked, this, &ResetWidget::onBackClicked);
    cardLayout->addWidget(backBtn, 0, Qt::AlignCenter);

    outerH->addWidget(card);
    outerH->addStretch(1);
    outerV->addLayout(outerH);
    outerV->addStretch(1);
    setLayout(outerV);

    showStep(StepEmail);
}

// ── showStep ─────────────────────────────────────────────────────────────────
void ResetWidget::showStep(Step step)
{
    m_currentStep = step;
    step1Widget->setVisible(step == StepEmail);
    step2Widget->setVisible(step == StepCode);
    step3Widget->setVisible(step == StepPassword);
}

// ── Helpers ───────────────────────────────────────────────────────────────────
bool ResetWidget::isEmailValid(const QString &email) const
{
    QRegularExpression re("^[a-zA-Z0-9._%+\\-]+@[a-zA-Z0-9.\\-]+\\.[a-zA-Z]{2,}$");
    return re.match(email).hasMatch();
}

void ResetWidget::validatePasswords()
{
    bool ok = newPasswordEdit->text().length() >= 8
           && !confirmPasswordEdit->text().isEmpty()
           && confirmPasswordEdit->text() == newPasswordEdit->text();
    saveBtn->setEnabled(ok);
    saveBtn->setStyleSheet(primaryBtnStyle(ok));
}

void ResetWidget::applyLock(int minutes, const QString &message)
{
    isLocked = true;
    verifyCodeBtn->setEnabled(false);
    verifyCodeBtn->setStyleSheet(blueBtnStyle(false));
    codeErrorLabel->setText(message);
    codeErrorLabel->show();
    lockTimer->start(minutes == 0 ? 30 * 1000 : minutes * 60 * 1000);
}

// ── Шаг 1 ────────────────────────────────────────────────────────────────────
void ResetWidget::onEmailTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        emailErrorLabel->hide();
        continueBtn->setEnabled(false);
        continueBtn->setStyleSheet(primaryBtnStyle(false));
        return;
    }
    bool valid = isEmailValid(text);
    if (!valid) {
        emailErrorLabel->setText(QString::fromUtf8("\u041d\u0435\u0432\u0435\u0440\u043d\u044b\u0439 \u0444\u043e\u0440\u043c\u0430\u0442 \u043f\u043e\u0447\u0442\u044b"));
        emailErrorLabel->show();
    } else {
        emailErrorLabel->hide();
    }
    continueBtn->setEnabled(valid);
    continueBtn->setStyleSheet(primaryBtnStyle(valid));
}

void ResetWidget::onContinueClicked()
{
    m_email = emailEdit->text().trimmed();
    m_pendingCodeHash.clear();
    m_waitingForCodeHash = true;
    failedAttempts = 0;
    isLocked = false;

    continueBtn->setEnabled(false);
    continueBtn->setText(QString::fromUtf8("\u041e\u0442\u043f\u0440\u0430\u0432\u043a\u0430..."));
    continueBtn->setStyleSheet(primaryBtnStyle(false));
    emailErrorLabel->hide();

    ClientSingleton::instance().sendRequestAsync(
        QString("reset_password||%1").arg(m_email));
}

// ── Шаг 2 ────────────────────────────────────────────────────────────────────
void ResetWidget::onCodeTextChanged(const QString &text)
{
    bool en = text.length() == 6 && !isLocked && !m_pendingCodeHash.isEmpty();
    verifyCodeBtn->setEnabled(en);
    verifyCodeBtn->setStyleSheet(blueBtnStyle(en));
}

void ResetWidget::onVerifyCodeClicked()
{
    if (isLocked || m_pendingCodeHash.isEmpty()) return;

    const QString code = codeEdit->text().trimmed();
    if (code.length() != 6) {
        codeErrorLabel->setText(QString::fromUtf8("\u0412\u0432\u0435\u0434\u0438\u0442\u0435 6-\u0437\u043d\u0430\u0447\u043d\u044b\u0439 \u043a\u043e\u0434."));
        codeErrorLabel->show();
        return;
    }

    const QString enteredHash = QString::fromLatin1(
        QCryptographicHash::hash(code.toUtf8(), QCryptographicHash::Sha256).toHex());

    if (enteredHash == m_pendingCodeHash) {
        codeErrorLabel->hide();
        codeStatusLabel->hide();
        newPasswordEdit->clear();
        confirmPasswordEdit->clear();
        newPasswordErrorLabel->hide();
        confirmErrorLabel->hide();
        saveBtn->setEnabled(false);
        saveBtn->setStyleSheet(primaryBtnStyle(false));
        showStep(StepPassword);
        return;
    }

    failedAttempts++;
    verifyCodeBtn->setEnabled(false);
    verifyCodeBtn->setStyleSheet(blueBtnStyle(false));
    codeStatusLabel->hide();

    if (failedAttempts < 4) {
        codeErrorLabel->setText(
            QString::fromUtf8("\u041d\u0435\u0432\u0435\u0440\u043d\u044b\u0439 \u043a\u043e\u0434. \u041e\u0441\u0442\u0430\u043b\u043e\u0441\u044c \u043f\u043e\u043f\u044b\u0442\u043e\u043a: %1").arg(4 - failedAttempts));
        codeErrorLabel->show();
        verifyCodeBtn->setEnabled(true);
        verifyCodeBtn->setStyleSheet(blueBtnStyle(true));
    } else {
        lockLevel++;
        failedAttempts = 0;
        const int lockMin = (lockLevel == 1) ? 0 : (lockLevel == 2) ? 5 : 30;
        applyLock(lockMin,
            lockMin == 0
                ? QString::fromUtf8("\u0421\u043b\u0438\u0448\u043a\u043e\u043c \u043c\u043d\u043e\u0433\u043e \u043f\u043e\u043f\u044b\u0442\u043e\u043a. \u0411\u043b\u043e\u043a\u0438\u0440\u043e\u0432\u043a\u0430 \u043d\u0430 30 \u0441\u0435\u043a.")
                : QString::fromUtf8("\u0421\u043b\u0438\u0448\u043a\u043e\u043c \u043c\u043d\u043e\u0433\u043e \u043f\u043e\u043f\u044b\u0442\u043e\u043a. \u0411\u043b\u043e\u043a\u0438\u0440\u043e\u0432\u043a\u0430 \u043d\u0430 %1 \u043c\u0438\u043d.").arg(lockMin));
    }
}

void ResetWidget::onLockTimerFired()
{
    isLocked = false;
    if (codeEdit->text().length() == 6 && !m_pendingCodeHash.isEmpty()) {
        verifyCodeBtn->setEnabled(true);
        verifyCodeBtn->setStyleSheet(blueBtnStyle(true));
    }
    codeErrorLabel->hide();
}

// ── Шаг 3 ────────────────────────────────────────────────────────────────────
void ResetWidget::onNewPasswordTextChanged(const QString &text)
{
    if (!text.isEmpty() && text.length() < 8) {
        newPasswordErrorLabel->setText(QString::fromUtf8("\u041c\u0438\u043d\u0438\u043c\u0443\u043c 8 \u0441\u0438\u043c\u0432\u043e\u043b\u043e\u0432"));
        newPasswordErrorLabel->show();
    } else {
        newPasswordErrorLabel->hide();
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
    newPasswordEdit->setEchoMode(
        newPasswordEdit->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password);
}

void ResetWidget::onTogglePassword2()
{
    confirmPasswordEdit->setEchoMode(
        confirmPasswordEdit->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password);
}

void ResetWidget::onSavePasswordClicked()
{
    if (m_waitingForSave) return;

    const QString passHash = QString::fromLatin1(
        QCryptographicHash::hash(
            newPasswordEdit->text().toUtf8(),
            QCryptographicHash::Sha256).toHex());

    m_waitingForSave = true;
    saveBtn->setEnabled(false);
    saveBtn->setText(QString::fromUtf8("\u0421\u043e\u0445\u0440\u0430\u043d\u044f\u0435\u043c..."));
    saveBtn->setStyleSheet(primaryBtnStyle(false));

    ClientSingleton::instance().sendRequestAsync(
        QString("set_new_password||%1||%2").arg(m_email, passHash));
}

// ── Response handler ──────────────────────────────────────────────────────────
void ResetWidget::onResetResponseReceived(const QString &response)
{
    const QString r = response.trimmed();
    if (r.isEmpty()) return;

    if (m_waitingForCodeHash) {
        m_waitingForCodeHash = false;
        continueBtn->setText(QString::fromUtf8("\u041f\u0440\u043e\u0434\u043e\u043b\u0436\u0438\u0442\u044c"));

        if (r.startsWith("reset_code_sent")) {
            const QStringList parts = r.split("||");
            m_pendingCodeHash = (parts.size() >= 2) ? parts[1].trimmed() : QString();

            codeEdit->clear();
            codeErrorLabel->hide();
            codeStatusLabel->setText(
                QString::fromUtf8("\u041a\u043e\u0434 \u043e\u0442\u043f\u0440\u0430\u0432\u043b\u0435\u043d \u043d\u0430: ") + m_email);
            codeStatusLabel->setStyleSheet(hintLabelStyle());
            codeStatusLabel->show();
            verifyCodeBtn->setEnabled(false);
            verifyCodeBtn->setStyleSheet(blueBtnStyle(false));
            showStep(StepCode);
            return;
        }

        continueBtn->setEnabled(true);
        continueBtn->setStyleSheet(primaryBtnStyle(true));
        if (r == "reset_error" || r.startsWith("reset_error")) {
            emailErrorLabel->setText(QString::fromUtf8("\u041f\u043e\u0447\u0442\u0430 \u043d\u0435 \u043d\u0430\u0439\u0434\u0435\u043d\u0430."));
        } else {
            emailErrorLabel->setText(QString::fromUtf8("\u041e\u0448\u0438\u0431\u043a\u0430 \u0441\u043e\u0435\u0434\u0438\u043d\u0435\u043d\u0438\u044f. \u041f\u043e\u043f\u0440\u043e\u0431\u0443\u0439\u0442\u0435 \u0441\u043d\u043e\u0432\u0430."));
        }
        emailErrorLabel->show();
        return;
    }

    if (m_waitingForSave) {
        m_waitingForSave = false;

        if (r == "set_password+") {
            QTimer::singleShot(500, this, [this]() { emit resetSuccess(); });
            return;
        }

        saveBtn->setEnabled(true);
        saveBtn->setText(QString::fromUtf8("\u0421\u043e\u0445\u0440\u0430\u043d\u0438\u0442\u044c \u043f\u0430\u0440\u043e\u043b\u044c"));
        saveBtn->setStyleSheet(primaryBtnStyle(true));
        confirmErrorLabel->setText(QString::fromUtf8("\u041e\u0448\u0438\u0431\u043a\u0430 \u0441\u0435\u0440\u0432\u0435\u0440\u0430. \u041f\u043e\u043f\u0440\u043e\u0431\u0443\u0439\u0442\u0435 \u0441\u043d\u043e\u0432\u0430."));
        confirmErrorLabel->show();
    }
}

// ── Back ──────────────────────────────────────────────────────────────────────
void ResetWidget::onBackClicked()
{
    m_waitingForCodeHash = false;
    m_waitingForSave     = false;
    isLocked             = false;
    failedAttempts       = 0;
    lockLevel            = 0;
    if (lockTimer->isActive()) lockTimer->stop();
    m_pendingCodeHash.clear();

    emailEdit->clear();
    emailErrorLabel->hide();
    continueBtn->setEnabled(false);
    continueBtn->setText(QString::fromUtf8("\u041f\u0440\u043e\u0434\u043e\u043b\u0436\u0438\u0442\u044c"));
    continueBtn->setStyleSheet(primaryBtnStyle(false));

    codeEdit->clear();
    codeErrorLabel->hide();
    codeStatusLabel->hide();
    verifyCodeBtn->setEnabled(false);
    verifyCodeBtn->setStyleSheet(blueBtnStyle(false));

    newPasswordEdit->clear();
    confirmPasswordEdit->clear();
    newPasswordErrorLabel->hide();
    confirmErrorLabel->hide();

    showStep(StepEmail);
    emit backToAuth();
}
