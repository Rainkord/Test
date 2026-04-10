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

// ── GitHub dark palette ────────────────────────────────────────────────────
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
      m_verifyingCode(false)
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

// ── Style helpers (same as authwidget) ───────────────────────────────────
static QString inputStyle()
{
    return QString(
        "QLineEdit {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 6px;"
        "  padding: 6px 10px;"
        "  font-family: '%4';"
        "  font-size: %5pt;"
        "}"
        "QLineEdit:focus { border-color: %6; }"
        "QLineEdit:read-only { color: %7; }"
    ).arg(GH_INPUT_BG).arg(GH_TEXT).arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT).arg(GH_BLUE).arg(GH_MUTED);
}

static QString primaryBtnStyle()
{
    return QString(
        "QPushButton {"
        "  background-color: %1;"
        "  color: #ffffff;"
        "  border: 1px solid rgba(240,246,252,0.1);"
        "  border-radius: 6px;"
        "  padding: 6px 16px;"
        "  font-family: '%3';"
        "  font-size: %4pt;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:disabled { background-color: rgba(35,134,54,0.4); color: rgba(255,255,255,0.4); }"
    ).arg(GH_GREEN).arg(GH_GREEN_H).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString secondaryBtnStyle()
{
    return QString(
        "QPushButton {"
        "  background-color: %1;"
        "  color: %3;"
        "  border: 1px solid %4;"
        "  border-radius: 6px;"
        "  padding: 6px 16px;"
        "  font-family: '%5';"
        "  font-size: %6pt;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:disabled { background-color: rgba(33,38,45,0.5); color: rgba(139,148,158,0.5); }"
    ).arg(GH_BTN_GHOST).arg(GH_BTN_GHOST_H).arg(GH_TEXT).arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString ghostBtnStyle()
{
    return QString(
        "QPushButton {"
        "  background-color: %1;"
        "  color: %3;"
        "  border: 1px solid %4;"
        "  border-radius: 6px;"
        "  padding: 5px 14px;"
        "  font-family: '%5';"
        "  font-size: %6pt;"
        "}"
        "QPushButton:hover { background-color: %2; }"
    ).arg(GH_BTN_GHOST).arg(GH_BTN_GHOST_H).arg(GH_TEXT).arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

static QString linkBtnStyle()
{
    return QString(
        "QPushButton {"
        "  color: %1;"
        "  border: none;"
        "  background: transparent;"
        "  font-family: '%2';"
        "  font-size: %3pt;"
        "}"
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
    // Внешний layout — центрирует карточку
    QVBoxLayout *outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->addStretch(1);

    QHBoxLayout *outerH = new QHBoxLayout();
    outerH->addStretch(1);

    // Карточка
    QWidget *card = new QWidget(this);
    card->setFixedWidth(360);
    card->setStyleSheet(QString(
        "QWidget {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 10px;"
        "}"
    ).arg(GH_CARD).arg(GH_BORDER));

    QVBoxLayout *mainLayout = new QVBoxLayout(card);
    mainLayout->setContentsMargins(28, 28, 28, 28);
    mainLayout->setSpacing(8);

    // Заголовок
    QLabel *titleLabel = new QLabel("Регистрация", card);
    QFont titleFont(FONT_FAMILY, FONT_SIZE_TITLE, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString("QLabel { color: %1; border: none; }").arg(GH_TEXT));
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(6);

    // ── STEP 1 ────────────────────────────────────────────────────────────
    step1Widget = new QWidget(card);
    step1Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *step1Layout = new QVBoxLayout(step1Widget);
    step1Layout->setContentsMargins(0, 0, 0, 0);
    step1Layout->setSpacing(6);

    loginEdit = new QLineEdit(step1Widget);
    loginEdit->setPlaceholderText("Логин (мин. 4 символа)");
    loginEdit->setMinimumHeight(38);
    loginEdit->setStyleSheet(inputStyle());
    step1Layout->addWidget(loginEdit);
    connect(loginEdit, &QLineEdit::textChanged, this, &RegWidget::onLoginTextChanged);

    loginErrorLabel = new QLabel(step1Widget);
    loginErrorLabel->setStyleSheet(errorLabelStyle());
    loginErrorLabel->hide();
    step1Layout->addWidget(loginErrorLabel);

    // Пароль 1
    QHBoxLayout *pass1Row = new QHBoxLayout();
    pass1Row->setSpacing(6);
    passwordEdit = new QLineEdit(step1Widget);
    passwordEdit->setPlaceholderText("Пароль (мин. 8 символов)");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(38);
    passwordEdit->setStyleSheet(inputStyle());
    pass1Row->addWidget(passwordEdit);
    connect(passwordEdit, &QLineEdit::textChanged, this, &RegWidget::onPasswordTextChanged);

    togglePassBtn1 = new QPushButton("👁", step1Widget);
    togglePassBtn1->setFixedSize(38, 38);
    togglePassBtn1->setStyleSheet(ghostBtnStyle());
    connect(togglePassBtn1, &QPushButton::clicked, this, &RegWidget::onTogglePassword1);
    pass1Row->addWidget(togglePassBtn1);
    step1Layout->addLayout(pass1Row);

    passwordErrorLabel = new QLabel(step1Widget);
    passwordErrorLabel->setStyleSheet(errorLabelStyle());
    passwordErrorLabel->hide();
    step1Layout->addWidget(passwordErrorLabel);

    // Пароль 2
    QHBoxLayout *pass2Row = new QHBoxLayout();
    pass2Row->setSpacing(6);
    confirmPasswordEdit = new QLineEdit(step1Widget);
    confirmPasswordEdit->setPlaceholderText("Подтвердите пароль");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setMinimumHeight(38);
    confirmPasswordEdit->setStyleSheet(inputStyle());
    pass2Row->addWidget(confirmPasswordEdit);
    connect(confirmPasswordEdit, &QLineEdit::textChanged, this, &RegWidget::onConfirmPasswordTextChanged);

    togglePassBtn2 = new QPushButton("👁", step1Widget);
    togglePassBtn2->setFixedSize(38, 38);
    togglePassBtn2->setStyleSheet(ghostBtnStyle());
    connect(togglePassBtn2, &QPushButton::clicked, this, &RegWidget::onTogglePassword2);
    pass2Row->addWidget(togglePassBtn2);
    step1Layout->addLayout(pass2Row);

    confirmErrorLabel = new QLabel(step1Widget);
    confirmErrorLabel->setStyleSheet(errorLabelStyle());
    confirmErrorLabel->hide();
    step1Layout->addWidget(confirmErrorLabel);
    step1Layout->addSpacing(4);

    continueBtn = new QPushButton("Продолжить", step1Widget);
    continueBtn->setMinimumHeight(38);
    continueBtn->setEnabled(false);
    continueBtn->setStyleSheet(primaryBtnStyle());
    connect(continueBtn, &QPushButton::clicked, this, &RegWidget::onContinueClicked);
    step1Layout->addWidget(continueBtn);

    mainLayout->addWidget(step1Widget);

    // ── STEP 2 ────────────────────────────────────────────────────────────
    step2Widget = new QWidget(card);
    step2Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *step2Layout = new QVBoxLayout(step2Widget);
    step2Layout->setContentsMargins(0, 0, 0, 0);
    step2Layout->setSpacing(6);

    emailEdit = new QLineEdit(step2Widget);
    emailEdit->setPlaceholderText("Email");
    emailEdit->setMinimumHeight(38);
    emailEdit->setStyleSheet(inputStyle());
    step2Layout->addWidget(emailEdit);
    connect(emailEdit, &QLineEdit::textChanged, this, &RegWidget::onEmailTextChanged);

    emailErrorLabel = new QLabel(step2Widget);
    emailErrorLabel->setStyleSheet(errorLabelStyle());
    emailErrorLabel->hide();
    step2Layout->addWidget(emailErrorLabel);
    step2Layout->addSpacing(4);

    backBtn = new QPushButton("← Назад", step2Widget);
    backBtn->setMinimumHeight(38);
    backBtn->setStyleSheet(secondaryBtnStyle());
    connect(backBtn, &QPushButton::clicked, this, &RegWidget::onBackClicked);
    step2Layout->addWidget(backBtn);

    mainLayout->addWidget(step2Widget);

    // ── STEP 3 ────────────────────────────────────────────────────────────
    step3Widget = new QWidget(card);
    step3Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *step3Layout = new QVBoxLayout(step3Widget);
    step3Layout->setContentsMargins(0, 0, 0, 0);
    step3Layout->setSpacing(6);

    confirmEmailBtn = new QPushButton("Отправить код на почту", step3Widget);
    confirmEmailBtn->setMinimumHeight(38);
    confirmEmailBtn->setStyleSheet(primaryBtnStyle());
    connect(confirmEmailBtn, &QPushButton::clicked, this, &RegWidget::onConfirmEmailClicked);
    step3Layout->addWidget(confirmEmailBtn);

    codeStatusLabel = new QLabel(step3Widget);
    codeStatusLabel->setStyleSheet(successLabelStyle());
    codeStatusLabel->setAlignment(Qt::AlignCenter);
    codeStatusLabel->hide();
    step3Layout->addWidget(codeStatusLabel);

    codeEdit = new QLineEdit(step3Widget);
    codeEdit->setPlaceholderText("Код из письма (6 цифр)");
    codeEdit->setMaxLength(6);
    codeEdit->setMinimumHeight(38);
    codeEdit->setStyleSheet(inputStyle());
    codeEdit->hide();
    step3Layout->addWidget(codeEdit);
    connect(codeEdit, &QLineEdit::textChanged, this, &RegWidget::onCodeTextChanged);

    codeErrorLabel = new QLabel(step3Widget);
    codeErrorLabel->setStyleSheet(errorLabelStyle());
    codeErrorLabel->hide();
    step3Layout->addWidget(codeErrorLabel);

    verifyCodeBtn = new QPushButton("Подтвердить код", step3Widget);
    verifyCodeBtn->setMinimumHeight(38);
    verifyCodeBtn->setStyleSheet(primaryBtnStyle());
    verifyCodeBtn->hide();
    connect(verifyCodeBtn, &QPushButton::clicked, this, &RegWidget::onVerifyCodeClicked);
    step3Layout->addWidget(verifyCodeBtn);

    mainLayout->addWidget(step3Widget);

    // Разделитель + ссылка
    QFrame *line = new QFrame(card);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("QFrame { background: %1; border: none; max-height: 1px; }").arg(GH_BORDER));
    mainLayout->addWidget(line);

    showAuthBtn = new QPushButton("Уже есть аккаунт? Войти", card);
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
    step2Widget->setVisible(step == 2 || step == 3);
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
        loginErrorLabel->setText("Минимум 4 символа");
        loginErrorLabel->show();
    } else {
        loginErrorLabel->hide();
    }
    validateStep1();
}

void RegWidget::onPasswordTextChanged(const QString &text)
{
    if (!text.isEmpty() && text.length() < 8) {
        passwordErrorLabel->setText("Минимум 8 символов");
        passwordErrorLabel->show();
    } else {
        passwordErrorLabel->hide();
    }
    QString confirm = confirmPasswordEdit->text();
    if (!confirm.isEmpty()) {
        if (confirm != text) {
            confirmErrorLabel->setText("Пароли не совпадают");
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
        confirmErrorLabel->setText("Пароли не совпадают");
        confirmErrorLabel->show();
    } else {
        confirmErrorLabel->hide();
    }
    validateStep1();
}

void RegWidget::onTogglePassword1()
{
    passwordEdit->setEchoMode(
        passwordEdit->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password
    );
}

void RegWidget::onTogglePassword2()
{
    confirmPasswordEdit->setEchoMode(
        confirmPasswordEdit->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password
    );
}

void RegWidget::onContinueClicked()
{
    continueBtn->setEnabled(false);
    continueBtn->setText("Проверяем...");
    loginErrorLabel->hide();
    m_checkingLogin = true;
    ClientSingleton::instance().sendRequestAsync(
        QString("check_login||%1").arg(loginEdit->text().trimmed()));
}

void RegWidget::onEmailTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        emailErrorLabel->hide();
        confirmEmailBtn->hide();
        step3Widget->hide();
        return;
    }
    if (!isEmailValid(text)) {
        emailErrorLabel->setText("Неверный формат почты");
        emailErrorLabel->show();
        confirmEmailBtn->hide();
        step3Widget->hide();
    } else {
        emailErrorLabel->hide();
        step3Widget->show();
        confirmEmailBtn->show();
    }
}

void RegWidget::onBackClicked()
{
    loginEdit->setReadOnly(false);
    passwordEdit->setReadOnly(false);
    confirmPasswordEdit->setReadOnly(false);

    continueBtn->setText("Продолжить");
    validateStep1();

    confirmEmailBtn->setEnabled(true);
    confirmEmailBtn->setText("Отправить код на почту");
    confirmEmailBtn->setStyleSheet(primaryBtnStyle());

    codeFailedAttempts = 0;
    codeLockLevel = 0;
    codeIsLocked = false;
    m_checkingLogin = false;
    m_verifyingCode = false;
    if (codeLockTimer->isActive()) codeLockTimer->stop();

    showStep(1);
}

void RegWidget::onConfirmEmailClicked()
{
    confirmEmailBtn->setEnabled(false);
    confirmEmailBtn->setText("Отправляем...");
    codeStatusLabel->setText("Ожидаем ответа сервера...");
    codeStatusLabel->setStyleSheet(infoLabelStyle());
    codeStatusLabel->show();
    codeErrorLabel->hide();
    m_verifyingCode = false;

    QString login    = loginEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString email    = emailEdit->text().trimmed();

    QByteArray hashBytes = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString passwordHash = QString::fromLatin1(hashBytes.toHex());
    ClientSingleton::instance().sendRequestAsync(
        QString("registration||%1||%2||%3").arg(login, passwordHash, email));
}

static QString friendlyRegError(const QString &raw)
{
    if (raw.contains("email_exists"))  return "На эту почту уже создан аккаунт";
    if (raw.contains("user_exists"))   return "Пользователь с таким логином уже существует";
    if (raw.contains("wrong_code"))    return "Неверный код подтверждения";
    if (raw.contains("db_error"))      return "Ошибка на сервере, попробуйте позже";
    return "Ошибка регистрации. Попробуйте снова";
}

void RegWidget::onRegistrationResponseReceived(const QString &response)
{
    QString r = response.trimmed();
    if (r.isEmpty()) return;

    if (m_checkingLogin) {
        m_checkingLogin = false;
        continueBtn->setText("Продолжить");
        if (r == "login_free") {
            loginEdit->setReadOnly(true);
            passwordEdit->setReadOnly(true);
            confirmPasswordEdit->setReadOnly(true);
            currentLogin = loginEdit->text().trimmed();
            showStep(2);
            emailEdit->clear();
            emailErrorLabel->hide();
            confirmEmailBtn->hide();
            codeStatusLabel->hide();
            codeEdit->hide();
            codeErrorLabel->hide();
            verifyCodeBtn->hide();
        } else if (r == "login_taken") {
            loginErrorLabel->setText("Логин уже занят");
            loginErrorLabel->show();
            validateStep1();
        } else {
            loginErrorLabel->setText("Ошибка соединения с сервером");
            loginErrorLabel->show();
            validateStep1();
        }
        return;
    }

    if (m_verifyingCode) {
        m_verifyingCode = false;
        verifyCodeBtn->setEnabled(true);
        if (r.startsWith("reg+")) {
            codeStatusLabel->setText("Регистрация успешна! Выполняется вход...");
            codeStatusLabel->setStyleSheet(successLabelStyle());
            codeStatusLabel->show();
            codeErrorLabel->hide();
            verifyCodeBtn->setEnabled(false);
            QTimer::singleShot(2000, this, [this]() { emit registrationSuccess(); });
            return;
        }
        if (r.startsWith("reg-")) {
            codeFailedAttempts++;
            codeStatusLabel->hide();
            if (codeFailedAttempts < 4) {
                codeErrorLabel->setText(QString("Неверный код. Осталось попыток: %1").arg(4 - codeFailedAttempts));
                codeErrorLabel->show();
                return;
            }
            if (codeFailedAttempts == 4) { applyCodeLock(0,    "Слишком много попыток. Блокировка 30 сек");  return; }
            if (codeFailedAttempts == 5) { applyCodeLock(5,    "Слишком много попыток. Блокировка 5 мин");   return; }
            if (codeFailedAttempts == 6) { applyCodeLock(10,   "Слишком много попыток. Блокировка 10 мин");  return; }
            applyCodeLock(9999, "Аккаунт заблокирован на длительное время");
            return;
        }
        codeErrorLabel->setText("Ошибка соединения с сервером.");
        codeErrorLabel->show();
        codeStatusLabel->hide();
        return;
    }

    if (r == "reg_code_sent") {
        codeStatusLabel->setText("Код отправлен на почту ✓");
        codeStatusLabel->setStyleSheet(successLabelStyle());
        codeStatusLabel->show();
        codeEdit->show();
        verifyCodeBtn->show();
        codeErrorLabel->hide();
        codeFailedAttempts = 0;
        codeIsLocked = false;
        confirmEmailBtn->setText("Код отправлен");
    } else if (r.startsWith("reg-")) {
        codeStatusLabel->hide();
        codeErrorLabel->setText(friendlyRegError(r));
        codeErrorLabel->show();
        confirmEmailBtn->setEnabled(true);
        confirmEmailBtn->setText("Отправить код на почту");
    }
}

void RegWidget::onCodeTextChanged(const QString &text) { Q_UNUSED(text) }

void RegWidget::applyCodeLock(int minutes, const QString &message)
{
    codeIsLocked = true;
    verifyCodeBtn->setEnabled(false);
    codeErrorLabel->setText(message);
    codeErrorLabel->show();
    codeLockTimer->start(minutes == 0 ? 30 * 1000 : minutes * 60 * 1000);
}

void RegWidget::onCodeLockTimerFired()
{
    codeIsLocked = false;
    verifyCodeBtn->setEnabled(true);
    codeErrorLabel->hide();
}

void RegWidget::onVerifyCodeClicked()
{
    if (codeIsLocked) {
        int remainingSec    = codeLockTimer->remainingTime() / 1000;
        int remainingMin    = remainingSec / 60;
        int remainingSecMod = remainingSec % 60;
        codeErrorLabel->setText(remainingMin > 0
            ? QString("Заблокировано. Осталось %1 мин %2 сек").arg(remainingMin).arg(remainingSecMod)
            : QString("Заблокировано. Осталось %1 сек").arg(remainingSec));
        codeErrorLabel->show();
        return;
    }
    QString code = codeEdit->text().trimmed();
    if (code.isEmpty()) {
        codeErrorLabel->setText("Введите код из письма.");
        codeErrorLabel->show();
        return;
    }
    m_verifyingCode = true;
    verifyCodeBtn->setEnabled(false);
    codeErrorLabel->hide();
    codeStatusLabel->setText("Проверяем код...");
    codeStatusLabel->setStyleSheet(infoLabelStyle());
    codeStatusLabel->show();
    ClientSingleton::instance().sendRequestAsync(
        QString("verify_reg||%1||%2").arg(loginEdit->text().trimmed(), code));
}

void RegWidget::onShowAuthClicked() { emit showAuth(); }
