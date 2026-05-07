#include "regwidget.h"
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
#include <QString>

#define GH_BG         "#0d1117"
#define GH_CARD       "#161b22"
#define GH_BORDER     "#30363d"
#define GH_TEXT       "#e6edf3"
#define GH_MUTED      "#8b949e"
#define GH_GREEN      "#238636"
#define GH_GREEN_H    "#2ea043"
#define GH_BLUE       "#388bfd"
#define GH_BLUE_H     "#58a6ff"
#define GH_RED        "#f85149"
#define GH_INPUT_BG   "#0d1117"
#define GH_BTN_GHOST  "#21262d"
#define GH_BTN_GHOST_H "#30363d"

#define FONT_FAMILY   "Segoe UI"
#define FONT_SIZE_TITLE 16
#define FONT_SIZE_BTN   11
#define FONT_SIZE_INPUT 11
#define FONT_SIZE_SMALL 9

RegWidget::RegWidget(QWidget *parent)
    : QWidget(parent),
      codeFailedAttempts(0),
      codeLockLevel(0),
      codeIsLocked(false),
      m_checkingLogin(false),
      m_verifyingCode(false),
      m_codeSent(false)
{
    codeLockTimer = new QTimer(this);
    codeLockTimer->setSingleShot(true);
    connect(codeLockTimer, &QTimer::timeout, this, &RegWidget::onCodeLockTimerFired);

    connect(&ClientSingleton::instance(), &ClientSingleton::responseReceived,
            this, &RegWidget::onRegistrationResponseReceived);

    setStyleSheet(QString("QWidget { background-color: %1; color: %2; font-family: '%3'; font-size: %4pt; }")
                  .arg(GH_BG).arg(GH_TEXT).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT));

    setupUI();
}

RegWidget::~RegWidget() {}

void RegWidget::clearFields()
{
    loginEdit->clear();
    passwordEdit->clear();
    confirmPasswordEdit->clear();
    emailEdit->clear();
    codeEdit->clear();

    loginErrorLabel->hide();
    passwordErrorLabel->hide();
    confirmErrorLabel->hide();
    emailErrorLabel->hide();
    codeErrorLabel->hide();
    codeStatusLabel->hide();
    codeSentToLabel->hide();

    loginEdit->setReadOnly(false);
    passwordEdit->setReadOnly(false);
    confirmPasswordEdit->setReadOnly(false);

    continueBtn->setEnabled(false);
    continueBtn->setText(QString::fromUtf8("\xd0\x9f\xd1\x80\xd0\xbe\xd0\xb4\xd0\xbe\xd0\xbb\xd0\xb6\xd0\xb8\xd1\x82\xd1\x8c"));

    emailNextBtn->setEnabled(false);
    confirmEmailBtn->setEnabled(true);
    confirmEmailBtn->setText(QString::fromUtf8("\xd0\x9e\xd1\x82\xd0\xbf\xd1\x80\xd0\xb0\xd0\xb2\xd0\xb8\xd1\x82\xd1\x8c \xd0\xba\xd0\xbe\xd0\xb4 \xd0\xbf\xd0\xbe\xd0\xb2\xd1\x82\xd0\xbe\xd1\x80\xd0\xbd\xd0\xbe"));

    codeFailedAttempts = 0;
    codeLockLevel      = 0;
    codeIsLocked       = false;
    m_verifyingCode    = false;
    m_checkingLogin    = false;
    m_codeSent         = false;
    currentLogin.clear();
    currentEmail.clear();

    if (codeLockTimer->isActive())
        codeLockTimer->stop();

    showStep(1);
}

// ── Style helpers ─────────────────────────────────────────────────────────
static QString inputStyle()
{
    return QString(
        "QLineEdit { background-color: %1; color: %2; border: 1px solid %3;"
        " border-radius: 6px; padding: 6px 10px; font-family: '%4'; font-size: %5pt; }"
        "QLineEdit:focus { border-color: %6; }"
        "QLineEdit:read-only { color: %7; }"
    ).arg(GH_INPUT_BG).arg(GH_TEXT).arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT).arg(GH_BLUE).arg(GH_MUTED);
}

static QString primaryBtnStyle()
{
    return QString(
        "QPushButton { background-color: %1; color: #ffffff; border: 1px solid rgba(240,246,252,0.1);"
        " border-radius: 6px; padding: 6px 16px; font-family: '%3'; font-size: %4pt; font-weight: bold; }"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:disabled { background-color: rgba(35,134,54,0.4); color: rgba(255,255,255,0.4); }"
    ).arg(GH_GREEN).arg(GH_GREEN_H).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString secondaryBtnStyle()
{
    return QString(
        "QPushButton { background-color: %1; color: %3; border: 1px solid %4;"
        " border-radius: 6px; padding: 6px 16px; font-family: '%5'; font-size: %6pt; font-weight: bold; }"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:disabled { background-color: rgba(33,38,45,0.5); color: rgba(139,148,158,0.5); }"
    ).arg(GH_BTN_GHOST).arg(GH_BTN_GHOST_H).arg(GH_TEXT).arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString ghostBtnStyle()
{
    return QString(
        "QPushButton { background-color: %1; color: %3; border: 1px solid %4;"
        " border-radius: 6px; padding: 5px 14px; font-family: '%5'; font-size: %6pt; }"
        "QPushButton:hover { background-color: %2; }"
    ).arg(GH_BTN_GHOST).arg(GH_BTN_GHOST_H).arg(GH_TEXT).arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString linkBtnStyle()
{
    return QString(
        "QPushButton { color: %1; border: none; background: transparent;"
        " font-family: '%2'; font-size: %3pt; }"
        "QPushButton:hover { color: %4; text-decoration: underline; }"
    ).arg(GH_BLUE).arg(FONT_FAMILY).arg(FONT_SIZE_BTN).arg(GH_BLUE_H);
}

static QString errorLabelStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_RED).arg(FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

static QString infoLabelStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_MUTED).arg(FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

static QString successLabelStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_GREEN_H).arg(FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

void RegWidget::setupUI()
{
    QVBoxLayout *outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->addStretch(1);

    QHBoxLayout *outerH = new QHBoxLayout();
    outerH->addStretch(1);

    QWidget *card = new QWidget(this);
    card->setFixedWidth(360);
    card->setStyleSheet(QString(
        "QWidget { background-color: %1; border: 1px solid %2; border-radius: 10px; }"
    ).arg(GH_CARD).arg(GH_BORDER));

    QVBoxLayout *mainLayout = new QVBoxLayout(card);
    mainLayout->setContentsMargins(28, 28, 28, 28);
    mainLayout->setSpacing(8);

    QLabel *titleLabel = new QLabel(QString::fromUtf8("\xd0\xa0\xd0\xb5\xd0\xb3\xd0\xb8\xd1\x81\xd1\x82\xd1\x80\xd0\xb0\xd1\x86\xd0\xb8\xd1\x8f"), card);
    QFont titleFont(FONT_FAMILY, FONT_SIZE_TITLE, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString("QLabel { color: %1; border: none; }").arg(GH_TEXT));
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(6);

    // ── ШАГ 1: логин + пароль ─────────────────────────────────────────────
    step1Widget = new QWidget(card);
    step1Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *s1 = new QVBoxLayout(step1Widget);
    s1->setContentsMargins(0,0,0,0); s1->setSpacing(6);

    loginEdit = new QLineEdit(step1Widget);
    loginEdit->setPlaceholderText(QString::fromUtf8("\xd0\x9b\xd0\xbe\xd0\xb3\xd0\xb8\xd0\xbd (\xd0\xbc\xd0\xb8\xd0\xbd. 4 \xd1\x81\xd0\xb8\xd0\xbc\xd0\xb2\xd0\xbe\xd0\xbb\xd0\xb0)"));
    loginEdit->setMinimumHeight(38);
    loginEdit->setStyleSheet(inputStyle());
    s1->addWidget(loginEdit);
    connect(loginEdit, &QLineEdit::textChanged, this, &RegWidget::onLoginTextChanged);

    loginErrorLabel = new QLabel(step1Widget);
    loginErrorLabel->setStyleSheet(errorLabelStyle());
    loginErrorLabel->hide();
    s1->addWidget(loginErrorLabel);

    QHBoxLayout *pass1Row = new QHBoxLayout();
    pass1Row->setSpacing(6);
    passwordEdit = new QLineEdit(step1Widget);
    passwordEdit->setPlaceholderText(QString::fromUtf8("\xd0\x9f\xd0\xb0\xd1\x80\xd0\xbe\xd0\xbb\xd1\x8c (\xd0\xbc\xd0\xb8\xd0\xbd. 8 \xd1\x81\xd0\xb8\xd0\xbc\xd0\xb2\xd0\xbe\xd0\xbb\xd0\xbe\xd0\xb2)"));
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(38);
    passwordEdit->setStyleSheet(inputStyle());
    pass1Row->addWidget(passwordEdit);
    connect(passwordEdit, &QLineEdit::textChanged, this, &RegWidget::onPasswordTextChanged);
    togglePassBtn1 = new QPushButton("\U0001f441", step1Widget);
    togglePassBtn1->setFixedSize(38,38);
    togglePassBtn1->setStyleSheet(ghostBtnStyle());
    connect(togglePassBtn1, &QPushButton::clicked, this, &RegWidget::onTogglePassword1);
    pass1Row->addWidget(togglePassBtn1);
    s1->addLayout(pass1Row);

    passwordErrorLabel = new QLabel(step1Widget);
    passwordErrorLabel->setStyleSheet(errorLabelStyle());
    passwordErrorLabel->hide();
    s1->addWidget(passwordErrorLabel);

    QHBoxLayout *pass2Row = new QHBoxLayout();
    pass2Row->setSpacing(6);
    confirmPasswordEdit = new QLineEdit(step1Widget);
    confirmPasswordEdit->setPlaceholderText(QString::fromUtf8("\xd0\x9f\xd0\xbe\xd0\xb4\xd1\x82\xd0\xb2\xd0\xb5\xd1\x80\xd0\xb4\xd0\xb8\xd1\x82\xd0\xb5 \xd0\xbf\xd0\xb0\xd1\x80\xd0\xbe\xd0\xbb\xd1\x8c"));
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setMinimumHeight(38);
    confirmPasswordEdit->setStyleSheet(inputStyle());
    pass2Row->addWidget(confirmPasswordEdit);
    connect(confirmPasswordEdit, &QLineEdit::textChanged, this, &RegWidget::onConfirmPasswordTextChanged);
    togglePassBtn2 = new QPushButton("\U0001f441", step1Widget);
    togglePassBtn2->setFixedSize(38,38);
    togglePassBtn2->setStyleSheet(ghostBtnStyle());
    connect(togglePassBtn2, &QPushButton::clicked, this, &RegWidget::onTogglePassword2);
    pass2Row->addWidget(togglePassBtn2);
    s1->addLayout(pass2Row);

    confirmErrorLabel = new QLabel(step1Widget);
    confirmErrorLabel->setStyleSheet(errorLabelStyle());
    confirmErrorLabel->hide();
    s1->addWidget(confirmErrorLabel);
    s1->addSpacing(4);

    continueBtn = new QPushButton(QString::fromUtf8("\xd0\x9f\xd1\x80\xd0\xbe\xd0\xb4\xd0\xbe\xd0\xbb\xd0\xb6\xd0\xb8\xd1\x82\xd1\x8c"), step1Widget);
    continueBtn->setMinimumHeight(38);
    continueBtn->setEnabled(false);
    continueBtn->setDefault(true);
    continueBtn->setAutoDefault(true);
    continueBtn->setStyleSheet(primaryBtnStyle());
    connect(continueBtn, &QPushButton::clicked, this, &RegWidget::onContinueClicked);
    // Enter в любом поле шага 1 -> continueBtn
    connect(loginEdit,           &QLineEdit::returnPressed, continueBtn, &QPushButton::click);
    connect(passwordEdit,        &QLineEdit::returnPressed, continueBtn, &QPushButton::click);
    connect(confirmPasswordEdit, &QLineEdit::returnPressed, continueBtn, &QPushButton::click);
    s1->addWidget(continueBtn);

    mainLayout->addWidget(step1Widget);

    // ── ШАГ 2: ввод email ─────────────────────────────────────────────────
    step2Widget = new QWidget(card);
    step2Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *s2 = new QVBoxLayout(step2Widget);
    s2->setContentsMargins(0,0,0,0); s2->setSpacing(6);

    QLabel *emailHint = new QLabel(
        QString::fromUtf8("\xd0\x92\xd0\xb2\xd0\xb5\xd0\xb4\xd0\xb8\xd1\x82\xd0\xb5 \xd0\xb2\xd0\xb0\xd1\x88 e-mail \xe2\x80\x94 \xd0\xbd\xd0\xb0 \xd0\xbd\xd0\xb5\xd0\xb3\xd0\xbe \xd0\xbf\xd1\x80\xd0\xb8\xd0\xb4\xd1\x91\xd1\x82 \xd0\xba\xd0\xbe\xd0\xb4 \xd0\xbf\xd0\xbe\xd0\xb4\xd1\x82\xd0\xb2\xd0\xb5\xd1\x80\xd0\xb6\xd0\xb4\xd0\xb5\xd0\xbd\xd0\xb8\xd1\x8f."),
        step2Widget);
    emailHint->setStyleSheet(infoLabelStyle());
    emailHint->setWordWrap(true);
    s2->addWidget(emailHint);

    emailEdit = new QLineEdit(step2Widget);
    emailEdit->setPlaceholderText("Email");
    emailEdit->setMinimumHeight(38);
    emailEdit->setStyleSheet(inputStyle());
    s2->addWidget(emailEdit);
    connect(emailEdit, &QLineEdit::textChanged, this, &RegWidget::onEmailTextChanged);

    emailErrorLabel = new QLabel(step2Widget);
    emailErrorLabel->setStyleSheet(errorLabelStyle());
    emailErrorLabel->hide();
    s2->addWidget(emailErrorLabel);
    s2->addSpacing(4);

    QHBoxLayout *s2Btns = new QHBoxLayout();
    s2Btns->setSpacing(8);
    backFromEmailBtn = new QPushButton(QString::fromUtf8("\xe2\x86\x90 \xd0\x9d\xd0\xb0\xd0\xb7\xd0\xb0\xd0\xb4"), step2Widget);
    backFromEmailBtn->setMinimumHeight(38);
    backFromEmailBtn->setStyleSheet(secondaryBtnStyle());
    connect(backFromEmailBtn, &QPushButton::clicked, this, &RegWidget::onBackFromEmailClicked);
    s2Btns->addWidget(backFromEmailBtn);

    emailNextBtn = new QPushButton(QString::fromUtf8("\xd0\x94\xd0\xb0\xd0\xbb\xd0\xb5\xd0\xb5 \xe2\x86\x92"), step2Widget);
    emailNextBtn->setMinimumHeight(38);
    emailNextBtn->setEnabled(false);
    emailNextBtn->setDefault(true);
    emailNextBtn->setAutoDefault(true);
    emailNextBtn->setStyleSheet(primaryBtnStyle());
    connect(emailNextBtn, &QPushButton::clicked, this, &RegWidget::onEmailNextClicked);
    connect(emailEdit, &QLineEdit::returnPressed, emailNextBtn, &QPushButton::click);
    s2Btns->addWidget(emailNextBtn);
    s2->addLayout(s2Btns);

    mainLayout->addWidget(step2Widget);

    // ── ШАГ 3: ввод кода ──────────────────────────────────────────────────
    step3Widget = new QWidget(card);
    step3Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *s3 = new QVBoxLayout(step3Widget);
    s3->setContentsMargins(0,0,0,0); s3->setSpacing(6);

    codeSentToLabel = new QLabel(step3Widget);
    codeSentToLabel->setStyleSheet(infoLabelStyle());
    codeSentToLabel->setWordWrap(true);
    codeSentToLabel->setAlignment(Qt::AlignCenter);
    codeSentToLabel->hide();
    s3->addWidget(codeSentToLabel);

    codeStatusLabel = new QLabel(step3Widget);
    codeStatusLabel->setStyleSheet(successLabelStyle());
    codeStatusLabel->setAlignment(Qt::AlignCenter);
    codeStatusLabel->hide();
    s3->addWidget(codeStatusLabel);

    codeEdit = new QLineEdit(step3Widget);
    codeEdit->setPlaceholderText(QString::fromUtf8("\xd0\x9a\xd0\xbe\xd0\xb4 \xd0\xb8\xd0\xb7 \xd0\xbf\xd0\xb8\xd1\x81\xd1\x8c\xd0\xbc\xd0\xb0 (6 \xd1\x86\xd0\xb8\xd1\x84\xd1\x80)"));
    codeEdit->setMaxLength(6);
    codeEdit->setMinimumHeight(38);
    codeEdit->setStyleSheet(inputStyle());
    s3->addWidget(codeEdit);
    connect(codeEdit, &QLineEdit::textChanged, this, &RegWidget::onCodeTextChanged);

    codeErrorLabel = new QLabel(step3Widget);
    codeErrorLabel->setStyleSheet(errorLabelStyle());
    codeErrorLabel->hide();
    s3->addWidget(codeErrorLabel);
    s3->addSpacing(4);

    verifyCodeBtn = new QPushButton(QString::fromUtf8("\xd0\x9f\xd0\xbe\xd0\xb4\xd1\x82\xd0\xb2\xd0\xb5\xd1\x80\xd0\xb4\xd0\xb8\xd1\x82\xd1\x8c \xd0\xba\xd0\xbe\xd0\xb4"), step3Widget);
    verifyCodeBtn->setMinimumHeight(38);
    verifyCodeBtn->setDefault(true);
    verifyCodeBtn->setAutoDefault(true);
    verifyCodeBtn->setStyleSheet(primaryBtnStyle());
    connect(verifyCodeBtn, &QPushButton::clicked, this, &RegWidget::onVerifyCodeClicked);
    connect(codeEdit, &QLineEdit::returnPressed, verifyCodeBtn, &QPushButton::click);
    s3->addWidget(verifyCodeBtn);

    QHBoxLayout *s3Bottom = new QHBoxLayout();
    s3Bottom->setSpacing(8);

    backFromCodeBtn = new QPushButton(QString::fromUtf8("\xe2\x86\x90 \xd0\x98\xd0\xb7\xd0\xbc\xd0\xb5\xd0\xbd\xd0\xb8\xd1\x82\xd1\x8c \xd0\xbf\xd0\xbe\xd1\x87\xd1\x82\xd1\x83"), step3Widget);
    backFromCodeBtn->setMinimumHeight(34);
    backFromCodeBtn->setStyleSheet(secondaryBtnStyle());
    connect(backFromCodeBtn, &QPushButton::clicked, this, &RegWidget::onBackFromCodeClicked);
    s3Bottom->addWidget(backFromCodeBtn);

    confirmEmailBtn = new QPushButton(QString::fromUtf8("\xd0\x9e\xd1\x82\xd0\xbf\xd1\x80\xd0\xb0\xd0\xb2\xd0\xb8\xd1\x82\xd1\x8c \xd0\xbf\xd0\xbe\xd0\xb2\xd1\x82\xd0\xbe\xd1\x80\xd0\xbd\xd0\xbe"), step3Widget);
    confirmEmailBtn->setMinimumHeight(34);
    confirmEmailBtn->setStyleSheet(ghostBtnStyle());
    connect(confirmEmailBtn, &QPushButton::clicked, this, &RegWidget::onConfirmEmailClicked);
    s3Bottom->addWidget(confirmEmailBtn);
    s3->addLayout(s3Bottom);

    mainLayout->addWidget(step3Widget);

    QFrame *line = new QFrame(card);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("QFrame { background: %1; border: none; max-height: 1px; }").arg(GH_BORDER));
    mainLayout->addWidget(line);

    showAuthBtn = new QPushButton(QString::fromUtf8("\xd0\xa3\xd0\xb6\xd0\xb5 \xd0\xb5\xd1\x81\xd1\x82\xd1\x8c \xd0\xb0\xd0\xba\xd0\xba\xd0\xb0\xd1\x83\xd0\xbd\xd1\x82? \xd0\x92\xd0\xbe\xd0\xb9\xd1\x82\xd0\xb8"), card);
    showAuthBtn->setFlat(true);
    showAuthBtn->setStyleSheet(linkBtnStyle());
    connect(showAuthBtn, &QPushButton::clicked, this, &RegWidget::onShowAuthClicked);
    mainLayout->addWidget(showAuthBtn, 0, Qt::AlignCenter);

    outerH->addWidget(card);
    outerH->addStretch(1);
    outerV->addLayout(outerH);
    outerV->addStretch(1);
    setLayout(outerV);
    showStep(1);
}

void RegWidget::showStep(int step)
{
    step1Widget->setVisible(step == 1);
    step2Widget->setVisible(step == 2);
    step3Widget->setVisible(step == 3);
}

bool RegWidget::isEmailValid(const QString &email) const
{
    QRegularExpression re("^[a-zA-Z0-9._%+\\-]+@[a-zA-Z0-9.\\-]+\\.[a-zA-Z]{2,}$");
    return re.match(email).hasMatch();
}

void RegWidget::validateStep1()
{
    bool loginOk   = (loginEdit->text().length() >= 4);
    bool passOk    = (passwordEdit->text().length() >= 8);
    bool confirmOk = (!confirmPasswordEdit->text().isEmpty()
                      && confirmPasswordEdit->text() == passwordEdit->text());
    continueBtn->setEnabled(loginOk && passOk && confirmOk);
}

void RegWidget::onLoginTextChanged(const QString &text)
{
    if (!text.isEmpty() && text.length() < 4) {
        loginErrorLabel->setText(QString::fromUtf8("\xd0\x9c\xd0\xb8\xd0\xbd\xd0\xb8\xd0\xbc\xd1\x83\xd0\xbc 4 \xd1\x81\xd0\xb8\xd0\xbc\xd0\xb2\xd0\xbe\xd0\xbb\xd0\xb0"));
        loginErrorLabel->show();
    } else {
        loginErrorLabel->hide();
    }
    validateStep1();
}

void RegWidget::onPasswordTextChanged(const QString &text)
{
    if (!text.isEmpty() && text.length() < 8) {
        passwordErrorLabel->setText(QString::fromUtf8("\xd0\x9c\xd0\xb8\xd0\xbd\xd0\xb8\xd0\xbc\xd1\x83\xd0\xbc 8 \xd1\x81\xd0\xb8\xd0\xbc\xd0\xb2\xd0\xbe\xd0\xbb\xd0\xbe\xd0\xb2"));
        passwordErrorLabel->show();
    } else {
        passwordErrorLabel->hide();
    }
    if (!confirmPasswordEdit->text().isEmpty()) {
        if (confirmPasswordEdit->text() != text) {
            confirmErrorLabel->setText(QString::fromUtf8("\xd0\x9f\xd0\xb0\xd1\x80\xd0\xbe\xd0\xbb\xd0\xb8 \xd0\xbd\xd0\xb5 \xd1\x81\xd0\xbe\xd0\xb2\xd0\xbf\xd0\xb0\xd0\xb4\xd0\xb0\xd1\x8e\xd1\x82"));
            confirmErrorLabel->show();
        } else {
            confirmErrorLabel->hide();
        }
    }
    validateStep1();
}

void RegWidget::onConfirmPasswordTextChanged(const QString &text)
{
    if (!text.isEmpty() && text != passwordEdit->text()) {
        confirmErrorLabel->setText(QString::fromUtf8("\xd0\x9f\xd0\xb0\xd1\x80\xd0\xbe\xd0\xbb\xd0\xb8 \xd0\xbd\xd0\xb5 \xd1\x81\xd0\xbe\xd0\xb2\xd0\xbf\xd0\xb0\xd0\xb4\xd0\xb0\xd1\x8e\xd1\x82"));
        confirmErrorLabel->show();
    } else {
        confirmErrorLabel->hide();
    }
    validateStep1();
}

void RegWidget::onTogglePassword1()
{
    passwordEdit->setEchoMode(
        passwordEdit->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password);
}

void RegWidget::onTogglePassword2()
{
    confirmPasswordEdit->setEchoMode(
        confirmPasswordEdit->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password);
}

void RegWidget::onContinueClicked()
{
    if (!continueBtn->isEnabled()) return;
    continueBtn->setEnabled(false);
    continueBtn->setText(QString::fromUtf8("\xd0\x9f\xd1\x80\xd0\xbe\xd0\xb2\xd0\xb5\xd1\x80\xd1\x8f\xd0\xb5\xd0\xbc..."));
    loginErrorLabel->hide();
    m_checkingLogin = true;
    ClientSingleton::instance().sendRequestAsync(
        QString("check_login||%1").arg(loginEdit->text().trimmed()));
}

void RegWidget::onEmailTextChanged(const QString &text)
{
    if (!isEmailValid(text)) {
        if (!text.isEmpty()) {
            emailErrorLabel->setText(QString::fromUtf8("\xd0\x9d\xd0\xb5\xd0\xb2\xd0\xb5\xd1\x80\xd0\xbd\xd1\x8b\xd0\xb9 \xd1\x84\xd0\xbe\xd1\x80\xd0\xbc\xd0\xb0\xd1\x82 \xd0\xbf\xd0\xbe\xd1\x87\xd1\x82\xd1\x8b"));
            emailErrorLabel->show();
        } else {
            emailErrorLabel->hide();
        }
        emailNextBtn->setEnabled(false);
    } else {
        emailErrorLabel->hide();
        emailNextBtn->setEnabled(true);
    }
}

void RegWidget::onEmailNextClicked()
{
    if (!emailNextBtn->isEnabled()) return;
    currentEmail = emailEdit->text().trimmed();
    // показываем шаг 3 и сразу отправляем код
    codeSentToLabel->setText(
        QString::fromUtf8("\xd0\x9e\xd1\x82\xd0\xbf\xd1\x80\xd0\xb0\xd0\xb2\xd0\xbb\xd1\x8f\xd0\xb5\xd0\xbc \xd0\xba\xd0\xbe\xd0\xb4 \xd0\xbd\xd0\xb0 ") + currentEmail + "...");
    codeSentToLabel->show();
    codeEdit->clear();
    codeErrorLabel->hide();
    codeStatusLabel->hide();
    codeFailedAttempts = 0;
    codeIsLocked = false;
    m_codeSent = false;
    showStep(3);
    onConfirmEmailClicked();
}

void RegWidget::onBackFromEmailClicked()
{
    loginEdit->setReadOnly(false);
    passwordEdit->setReadOnly(false);
    confirmPasswordEdit->setReadOnly(false);
    continueBtn->setText(QString::fromUtf8("\xd0\x9f\xd1\x80\xd0\xbe\xd0\xb4\xd0\xbe\xd0\xbb\xd0\xb6\xd0\xb8\xd1\x82\xd1\x8c"));
    validateStep1();
    m_checkingLogin = false;
    showStep(1);
}

void RegWidget::onBackFromCodeClicked()
{
    // возврат на шаг 2 для изменения email
    codeEdit->clear();
    codeErrorLabel->hide();
    codeStatusLabel->hide();
    codeSentToLabel->hide();
    confirmEmailBtn->setEnabled(true);
    confirmEmailBtn->setText(QString::fromUtf8("\xd0\x9e\xd1\x82\xd0\xbf\xd1\x80\xd0\xb0\xd0\xb2\xd0\xb8\xd1\x82\xd1\x8c \xd0\xbf\xd0\xbe\xd0\xb2\xd1\x82\xd0\xbe\xd1\x80\xd0\xbd\xd0\xbe"));
    codeFailedAttempts = 0;
    codeIsLocked = false;
    m_codeSent = false;
    if (codeLockTimer->isActive()) codeLockTimer->stop();
    showStep(2);
}

void RegWidget::onConfirmEmailClicked()
{
    if (codeIsLocked) return;
    confirmEmailBtn->setEnabled(false);
    confirmEmailBtn->setText(QString::fromUtf8("\xd0\x9e\xd1\x82\xd0\xbf\xd1\x80\xd0\xb0\xd0\xb2\xd0\xbb\xd1\x8f\xd0\xb5\xd0\xbc..."));
    codeStatusLabel->setText(QString::fromUtf8("\xd0\x9e\xd0\xb6\xd0\xb8\xd0\xb4\xd0\xb0\xd0\xb5\xd0\xbc \xd0\xbe\xd1\x82\xd0\xb2\xd0\xb5\xd1\x82\xd0\xb0 \xd1\x81\xd0\xb5\xd1\x80\xd0\xb2\xd0\xb5\xd1\x80\xd0\xb0..."));
    codeStatusLabel->setStyleSheet(infoLabelStyle());
    codeStatusLabel->show();
    codeErrorLabel->hide();
    m_verifyingCode = false;

    QString login    = currentLogin;
    QString password = passwordEdit->text();
    QString email    = currentEmail;

    QByteArray hashBytes = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString passwordHash = QString::fromLatin1(hashBytes.toHex());
    ClientSingleton::instance().sendRequestAsync(
        QString("registration||%1||%2||%3").arg(login, passwordHash, email));
}

static QString friendlyRegError(const QString &raw)
{
    if (raw.contains("email_exists"))  return QString::fromUtf8("\xd0\x9d\xd0\xb0 \xd1\x8d\xd1\x82\xd1\x83 \xd0\xbf\xd0\xbe\xd1\x87\xd1\x82\xd1\x83 \xd1\x83\xd0\xb6\xd0\xb5 \xd1\x81\xd0\xbe\xd0\xb7\xd0\xb4\xd0\xb0\xd0\xbd \xd0\xb0\xd0\xba\xd0\xba\xd0\xb0\xd1\x83\xd0\xbd\xd1\x82");
    if (raw.contains("user_exists"))   return QString::fromUtf8("\xd0\x9f\xd0\xbe\xd0\xbb\xd1\x8c\xd0\xb7\xd0\xbe\xd0\xb2\xd0\xb0\xd1\x82\xd0\xb5\xd0\xbb\xd1\x8c \xd1\x81 \xd1\x82\xd0\xb0\xd0\xba\xd0\xb8\xd0\xbc \xd0\xbb\xd0\xbe\xd0\xb3\xd0\xb8\xd0\xbd\xd0\xbe\xd0\xbc \xd1\x83\xd0\xb6\xd0\xb5 \xd1\x81\xd1\x83\xd1\x89\xd0\xb5\xd1\x81\xd1\x82\xd0\xb2\xd1\x83\xd0\xb5\xd1\x82");
    if (raw.contains("wrong_code"))    return QString::fromUtf8("\xd0\x9d\xd0\xb5\xd0\xb2\xd0\xb5\xd1\x80\xd0\xbd\xd1\x8b\xd0\xb9 \xd0\xba\xd0\xbe\xd0\xb4 \xd0\xbf\xd0\xbe\xd0\xb4\xd1\x82\xd0\xb2\xd0\xb5\xd1\x80\xd0\xb6\xd0\xb4\xd0\xb5\xd0\xbd\xd0\xb8\xd1\x8f");
    if (raw.contains("db_error"))      return QString::fromUtf8("\xd0\x9e\xd1\x88\xd0\xb8\xd0\xb1\xd0\xba\xd0\xb0 \xd0\xbd\xd0\xb0 \xd1\x81\xd0\xb5\xd1\x80\xd0\xb2\xd0\xb5\xd1\x80\xd0\xb5, \xd0\xbf\xd0\xbe\xd0\xbf\xd1\x80\xd0\xbe\xd0\xb1\xd1\x83\xd0\xb9\xd1\x82\xd0\xb5 \xd0\xbf\xd0\xbe\xd0\xb7\xd0\xb6\xd0\xb5");
    return QString::fromUtf8("\xd0\x9e\xd1\x88\xd0\xb8\xd0\xb1\xd0\xba\xd0\xb0 \xd1\x80\xd0\xb5\xd0\xb3\xd0\xb8\xd1\x81\xd1\x82\xd1\x80\xd0\xb0\xd1\x86\xd0\xb8\xd0\xb8. \xd0\x9f\xd0\xbe\xd0\xbf\xd1\x80\xd0\xbe\xd0\xb1\xd1\x83\xd0\xb9\xd1\x82\xd0\xb5 \xd1\x81\xd0\xbd\xd0\xbe\xd0\xb2\xd0\xb0");
}

void RegWidget::onRegistrationResponseReceived(const QString &response)
{
    QString r = response.trimmed();
    if (r.isEmpty()) return;

    // --- Ответ на check_login ---
    if (m_checkingLogin) {
        m_checkingLogin = false;
        continueBtn->setText(QString::fromUtf8("\xd0\x9f\xd1\x80\xd0\xbe\xd0\xb4\xd0\xbe\xd0\xbb\xd0\xb6\xd0\xb8\xd1\x82\xd1\x8c"));
        if (r == "login_free") {
            loginEdit->setReadOnly(true);
            passwordEdit->setReadOnly(true);
            confirmPasswordEdit->setReadOnly(true);
            currentLogin = loginEdit->text().trimmed();
            emailEdit->clear();
            emailErrorLabel->hide();
            emailNextBtn->setEnabled(false);
            showStep(2);
        } else if (r == "login_taken") {
            loginErrorLabel->setText(QString::fromUtf8("\xd0\x9b\xd0\xbe\xd0\xb3\xd0\xb8\xd0\xbd \xd1\x83\xd0\xb6\xd0\xb5 \xd0\xb7\xd0\xb0\xd0\xbd\xd1\x8f\xd1\x82"));
            loginErrorLabel->show();
            validateStep1();
        } else {
            loginErrorLabel->setText(QString::fromUtf8("\xd0\x9e\xd1\x88\xd0\xb8\xd0\xb1\xd0\xba\xd0\xb0 \xd1\x81\xd0\xbe\xd0\xb5\xd0\xb4\xd0\xb8\xd0\xbd\xd0\xb5\xd0\xbd\xd0\xb8\xd1\x8f \xd1\x81 \xd1\x81\xd0\xb5\xd1\x80\xd0\xb2\xd0\xb5\xd1\x80\xd0\xbe\xd0\xbc"));
            loginErrorLabel->show();
            validateStep1();
        }
        return;
    }

    // --- Ответ на registration (отправка кода) ---
    if (!m_verifyingCode && !m_codeSent) {
        confirmEmailBtn->setText(QString::fromUtf8("\xd0\x9e\xd1\x82\xd0\xbf\xd1\x80\xd0\xb0\xd0\xb2\xd0\xb8\xd1\x82\xd1\x8c \xd0\xbf\xd0\xbe\xd0\xb2\xd1\x82\xd0\xbe\xd1\x80\xd0\xbd\xd0\xbe"));
        if (r == "code_sent" || r.startsWith("code_sent")) {
            m_codeSent = true;
            confirmEmailBtn->setEnabled(true);
            codeSentToLabel->setText(
                QString::fromUtf8("\xd0\x9a\xd0\xbe\xd0\xb4 \xd0\xbe\xd1\x82\xd0\xbf\xd1\x80\xd0\xb0\xd0\xb2\xd0\xbb\xd0\xb5\xd0\xbd \xd0\xbd\xd0\xb0 ") + currentEmail);
            codeSentToLabel->show();
            codeStatusLabel->setText(QString::fromUtf8("\xd0\x9f\xd1\x80\xd0\xbe\xd0\xb2\xd0\xb5\xd1\x80\xd1\x8c\xd1\x82\xd0\xb5 \xd0\xbf\xd0\xbe\xd1\x87\xd1\x82\xd1\x83 \xd0\xb8 \xd0\xb2\xd0\xb2\xd0\xb5\xd0\xb4\xd0\xb8\xd1\x82\xd0\xb5 \xd0\xba\xd0\xbe\xd0\xb4"));
            codeStatusLabel->setStyleSheet(infoLabelStyle());
            codeStatusLabel->show();
        } else {
            // ошибка при отправке
            codeStatusLabel->setText(friendlyRegError(r));
            codeStatusLabel->setStyleSheet(errorLabelStyle());
            codeStatusLabel->show();
            confirmEmailBtn->setEnabled(true);
        }
        return;
    }

    // --- Ответ на verify кода ---
    if (m_verifyingCode) {
        m_verifyingCode = false;
        verifyCodeBtn->setEnabled(true);
        if (r.startsWith("reg+")) {
            codeStatusLabel->setText(QString::fromUtf8("\xd0\xa0\xd0\xb5\xd0\xb3\xd0\xb8\xd1\x81\xd1\x82\xd1\x80\xd0\xb0\xd1\x86\xd0\xb8\xd1\x8f \xd1\x83\xd1\x81\xd0\xbf\xd0\xb5\xd1\x88\xd0\xbd\xd0\xb0! \xd0\x92\xd1\x85\xd0\xbe\xd0\xb4..."));
            codeStatusLabel->setStyleSheet(successLabelStyle());
            codeStatusLabel->show();
            codeErrorLabel->hide();
            verifyCodeBtn->setEnabled(false);
            QTimer::singleShot(800, this, [this]() { emit registrationSuccess(currentLogin); });
            return;
        }
        if (r.startsWith("reg-")) {
            codeFailedAttempts++;
            codeStatusLabel->hide();
            if (codeFailedAttempts < 4) {
                codeErrorLabel->setText(QString::fromUtf8("\xd0\x9d\xd0\xb5\xd0\xb2\xd0\xb5\xd1\x80\xd0\xbd\xd1\x8b\xd0\xb9 \xd0\xba\xd0\xbe\xd0\xb4. \xd0\x9e\xd1\x81\xd1\x82\xd0\xb0\xd0\xbb\xd0\xbe\xd1\x81\xd1\x8c \xd0\xbf\xd0\xbe\xd0\xbf\xd1\x8b\xd1\x82\xd0\xbe\xd0\xba: %1").arg(4 - codeFailedAttempts));
                codeErrorLabel->show();
                return;
            }
            applyCodeLock(1, QString::fromUtf8("\xd0\x9f\xd1\x80\xd0\xb5\xd0\xb2\xd1\x8b\xd1\x88\xd0\xb5\xd0\xbd \xd0\xbb\xd0\xb8\xd0\xbc\xd0\xb8\xd1\x82. \xd0\x91\xd0\xbb\xd0\xbe\xd0\xba\xd0\xb8\xd1\x80\xd0\xbe\xd0\xb2\xd0\xba\xd0\xb0 \xd0\xbd\xd0\xb0 1 \xd0\xbc\xd0\xb8\xd0\xbd."));
            return;
        }
        codeErrorLabel->setText(friendlyRegError(r));
        codeErrorLabel->show();
    }
}

void RegWidget::onCodeTextChanged(const QString &/*text*/)
{
    // ничего дополнительного
}

void RegWidget::onVerifyCodeClicked()
{
    if (codeIsLocked) return;
    QString code = codeEdit->text().trimmed();
    if (code.length() < 6) {
        codeErrorLabel->setText(QString::fromUtf8("\xd0\x9a\xd0\xbe\xd0\xb4 \xd0\xb4\xd0\xbe\xd0\xbb\xd0\xb6\xd0\xb5\xd0\xbd \xd1\x81\xd0\xbe\xd0\xb4\xd0\xb5\xd1\x80\xd0\xb6\xd0\xb0\xd1\x82\xd1\x8c 6 \xd1\x86\xd0\xb8\xd1\x84\xd1\x80"));
        codeErrorLabel->show();
        return;
    }
    verifyCodeBtn->setEnabled(false);
    codeErrorLabel->hide();
    m_verifyingCode = true;
    ClientSingleton::instance().sendRequestAsync(
        QString("verify_code||%1||%2").arg(currentLogin, code));
}

void RegWidget::applyCodeLock(int minutes, const QString &message)
{
    codeIsLocked = true;
    verifyCodeBtn->setEnabled(false);
    confirmEmailBtn->setEnabled(false);
    codeErrorLabel->setText(message);
    codeErrorLabel->show();
    codeLockTimer->start(minutes * 60 * 1000);
}

void RegWidget::onCodeLockTimerFired()
{
    codeIsLocked = false;
    verifyCodeBtn->setEnabled(true);
    confirmEmailBtn->setEnabled(true);
    codeErrorLabel->hide();
    codeFailedAttempts = 0;
}

void RegWidget::onShowAuthClicked() { emit showAuth(); }
