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
#include <QString>

RegWidget::RegWidget(QWidget *parent)
    : QWidget(parent),
      codeFailedAttempts(0),
      codeLockLevel(0),
      codeIsLocked(false),
      m_verifyingCode(false)
{
    codeLockTimer = new QTimer(this);
    codeLockTimer->setSingleShot(true);
    connect(codeLockTimer, &QTimer::timeout, this, &RegWidget::onCodeLockTimerFired);

    connect(&ClientSingleton::instance(), &ClientSingleton::responseReceived,
            this, &RegWidget::onRegistrationResponseReceived);

    setupUI();
}

RegWidget::~RegWidget()
{
}

void RegWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 20, 40, 20);
    mainLayout->setSpacing(0);

    QLabel *titleLabel = new QLabel("Регистрация", this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(18);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(14);

    // =====================
    // STEP 1 WIDGET
    // =====================
    step1Widget = new QWidget(this);
    QVBoxLayout *step1Layout = new QVBoxLayout(step1Widget);
    step1Layout->setContentsMargins(0, 0, 0, 0);
    step1Layout->setSpacing(4);

    loginEdit = new QLineEdit(step1Widget);
    loginEdit->setPlaceholderText("Логин");
    loginEdit->setMinimumHeight(36);
    loginEdit->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #cccccc; border-radius: 4px; }");
    step1Layout->addWidget(loginEdit);
    connect(loginEdit, &QLineEdit::textChanged, this, &RegWidget::onLoginTextChanged);

    loginErrorLabel = new QLabel(step1Widget);
    loginErrorLabel->setStyleSheet("QLabel { color: red; font-size: 10pt; }");
    loginErrorLabel->hide();
    step1Layout->addWidget(loginErrorLabel);
    step1Layout->addSpacing(6);

    QHBoxLayout *pass1Layout = new QHBoxLayout();
    pass1Layout->setSpacing(6);
    passwordEdit = new QLineEdit(step1Widget);
    passwordEdit->setPlaceholderText("Пароль");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(36);
    passwordEdit->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #cccccc; border-radius: 4px; }");
    pass1Layout->addWidget(passwordEdit);
    connect(passwordEdit, &QLineEdit::textChanged, this, &RegWidget::onPasswordTextChanged);

    togglePassBtn1 = new QPushButton("👁", step1Widget);
    togglePassBtn1->setFixedWidth(35);
    togglePassBtn1->setFixedHeight(36);
    togglePassBtn1->setToolTip("Показать/скрыть пароль");
    togglePassBtn1->setStyleSheet(
        "QPushButton { border: 1px solid #cccccc; border-radius: 4px; background: #f5f5f5; }"
        "QPushButton:hover { background: #e0e0e0; }"
    );
    connect(togglePassBtn1, &QPushButton::clicked, this, &RegWidget::onTogglePassword1);
    pass1Layout->addWidget(togglePassBtn1);
    step1Layout->addLayout(pass1Layout);

    passwordErrorLabel = new QLabel(step1Widget);
    passwordErrorLabel->setStyleSheet("QLabel { color: red; font-size: 10pt; }");
    passwordErrorLabel->hide();
    step1Layout->addWidget(passwordErrorLabel);
    step1Layout->addSpacing(6);

    QHBoxLayout *pass2Layout = new QHBoxLayout();
    pass2Layout->setSpacing(6);
    confirmPasswordEdit = new QLineEdit(step1Widget);
    confirmPasswordEdit->setPlaceholderText("Подтвердите пароль");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setMinimumHeight(36);
    confirmPasswordEdit->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #cccccc; border-radius: 4px; }");
    pass2Layout->addWidget(confirmPasswordEdit);
    connect(confirmPasswordEdit, &QLineEdit::textChanged, this, &RegWidget::onConfirmPasswordTextChanged);

    togglePassBtn2 = new QPushButton("👁", step1Widget);
    togglePassBtn2->setFixedWidth(35);
    togglePassBtn2->setFixedHeight(36);
    togglePassBtn2->setToolTip("Показать/скрыть пароль");
    togglePassBtn2->setStyleSheet(
        "QPushButton { border: 1px solid #cccccc; border-radius: 4px; background: #f5f5f5; }"
        "QPushButton:hover { background: #e0e0e0; }"
    );
    connect(togglePassBtn2, &QPushButton::clicked, this, &RegWidget::onTogglePassword2);
    pass2Layout->addWidget(togglePassBtn2);
    step1Layout->addLayout(pass2Layout);

    confirmErrorLabel = new QLabel(step1Widget);
    confirmErrorLabel->setStyleSheet("QLabel { color: red; font-size: 10pt; }");
    confirmErrorLabel->hide();
    step1Layout->addWidget(confirmErrorLabel);
    step1Layout->addSpacing(10);

    continueBtn = new QPushButton("Продолжить", step1Widget);
    continueBtn->setMinimumHeight(38);
    continueBtn->setEnabled(false);
    continueBtn->setStyleSheet(
        "QPushButton { background-color: #cccccc; color: #666666; border: none; border-radius: 4px; font-size: 13pt; }"
    );
    connect(continueBtn, &QPushButton::clicked, this, &RegWidget::onContinueClicked);
    step1Layout->addWidget(continueBtn);

    mainLayout->addWidget(step1Widget);

    // =====================
    // STEP 2 WIDGET
    // =====================
    step2Widget = new QWidget(this);
    QVBoxLayout *step2Layout = new QVBoxLayout(step2Widget);
    step2Layout->setContentsMargins(0, 0, 0, 0);
    step2Layout->setSpacing(4);

    emailEdit = new QLineEdit(step2Widget);
    emailEdit->setPlaceholderText("Email");
    emailEdit->setMinimumHeight(36);
    emailEdit->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #cccccc; border-radius: 4px; }");
    step2Layout->addWidget(emailEdit);
    connect(emailEdit, &QLineEdit::textChanged, this, &RegWidget::onEmailTextChanged);

    emailErrorLabel = new QLabel(step2Widget);
    emailErrorLabel->setStyleSheet("QLabel { color: red; font-size: 10pt; }");
    emailErrorLabel->hide();
    step2Layout->addWidget(emailErrorLabel);
    step2Layout->addSpacing(16);

    backBtn = new QPushButton("Назад", step2Widget);
    backBtn->setMinimumHeight(36);
    backBtn->setStyleSheet(
        "QPushButton { background-color: #9E9E9E; color: white; border: none; border-radius: 4px; font-size: 12pt; }"
        "QPushButton:hover { background-color: #757575; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &RegWidget::onBackClicked);
    step2Layout->addWidget(backBtn);
    step2Layout->addSpacing(20);

    mainLayout->addWidget(step2Widget);

    // =====================
    // STEP 3 WIDGET
    // =====================
    step3Widget = new QWidget(this);
    QVBoxLayout *step3Layout = new QVBoxLayout(step3Widget);
    step3Layout->setContentsMargins(0, 0, 0, 0);
    step3Layout->setSpacing(6);

    confirmEmailBtn = new QPushButton("Подтвердить почту", step3Widget);
    confirmEmailBtn->setMinimumHeight(38);
    confirmEmailBtn->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; border: none; border-radius: 4px; font-size: 12pt; }"
        "QPushButton:hover { background-color: #1976D2; }"
        "QPushButton:disabled { background-color: #cccccc; color: #666666; }"
    );
    connect(confirmEmailBtn, &QPushButton::clicked, this, &RegWidget::onConfirmEmailClicked);
    step3Layout->addWidget(confirmEmailBtn);

    codeStatusLabel = new QLabel(step3Widget);
    codeStatusLabel->setStyleSheet("QLabel { color: #388E3C; font-size: 10pt; }");
    codeStatusLabel->setAlignment(Qt::AlignCenter);
    codeStatusLabel->hide();
    step3Layout->addWidget(codeStatusLabel);

    codeEdit = new QLineEdit(step3Widget);
    codeEdit->setPlaceholderText("Введите код из письма");
    codeEdit->setMaxLength(6);
    codeEdit->setMinimumHeight(36);
    codeEdit->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #cccccc; border-radius: 4px; }");
    codeEdit->hide();
    step3Layout->addWidget(codeEdit);
    connect(codeEdit, &QLineEdit::textChanged, this, &RegWidget::onCodeTextChanged);

    codeErrorLabel = new QLabel(step3Widget);
    codeErrorLabel->setStyleSheet("QLabel { color: red; font-size: 10pt; }");
    codeErrorLabel->hide();
    step3Layout->addWidget(codeErrorLabel);

    verifyCodeBtn = new QPushButton("Подтвердить код", step3Widget);
    verifyCodeBtn->setMinimumHeight(38);
    verifyCodeBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 4px; font-size: 12pt; }"
        "QPushButton:hover { background-color: #388E3C; }"
        "QPushButton:disabled { background-color: #cccccc; color: #666666; }"
    );
    verifyCodeBtn->hide();
    connect(verifyCodeBtn, &QPushButton::clicked, this, &RegWidget::onVerifyCodeClicked);
    step3Layout->addWidget(verifyCodeBtn);

    mainLayout->addWidget(step3Widget);
    mainLayout->addSpacing(12);

    showAuthBtn = new QPushButton("Уже есть аккаунт? Войти", this);
    showAuthBtn->setFlat(true);
    showAuthBtn->setStyleSheet(
        "QPushButton { color: #2196F3; border: none; text-decoration: underline; font-size: 11pt; }"
        "QPushButton:hover { color: #1565C0; }"
    );
    connect(showAuthBtn, &QPushButton::clicked, this, &RegWidget::onShowAuthClicked);
    mainLayout->addWidget(showAuthBtn, 0, Qt::AlignCenter);

    mainLayout->addStretch();

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
    bool loginOk    = (loginEdit->text().length() >= 4);
    bool passwordOk = (passwordEdit->text().length() >= 8);
    bool confirmOk  = (!confirmPasswordEdit->text().isEmpty()
                       && confirmPasswordEdit->text() == passwordEdit->text());

    if (loginOk && passwordOk && confirmOk) {
        continueBtn->setEnabled(true);
        continueBtn->setStyleSheet(
            "QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 4px; font-size: 13pt; }"
            "QPushButton:hover { background-color: #388E3C; }"
        );
    } else {
        continueBtn->setEnabled(false);
        continueBtn->setStyleSheet(
            "QPushButton { background-color: #cccccc; color: #666666; border: none; border-radius: 4px; font-size: 13pt; }"
        );
    }
}

void RegWidget::onLoginTextChanged(const QString &text)
{
    if (text.isEmpty() || text.length() < 4) {
        loginErrorLabel->setText("Минимум 4 символа");
        loginErrorLabel->show();
    } else {
        loginErrorLabel->hide();
    }
    validateStep1();
}

void RegWidget::onPasswordTextChanged(const QString &text)
{
    if (text.isEmpty() || text.length() < 8) {
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
    if (text.isEmpty() || text != passwordEdit->text()) {
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

    confirmEmailBtn->setEnabled(true);
    confirmEmailBtn->setText("Подтвердить почту");
    confirmEmailBtn->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; border: none; border-radius: 4px; font-size: 12pt; }"
        "QPushButton:hover { background-color: #1976D2; }"
        "QPushButton:disabled { background-color: #cccccc; color: #666666; }"
    );

    codeFailedAttempts = 0;
    codeLockLevel = 0;
    codeIsLocked = false;
    m_verifyingCode = false;
    if (codeLockTimer->isActive()) codeLockTimer->stop();

    showStep(1);
}

void RegWidget::onConfirmEmailClicked()
{
    confirmEmailBtn->setEnabled(false);
    confirmEmailBtn->setText("Отправляем...");
    codeStatusLabel->setText("Ожидаем ответа сервера...");
    codeStatusLabel->setStyleSheet("QLabel { color: #888888; font-size: 10pt; }");
    codeStatusLabel->show();
    codeErrorLabel->hide();

    m_verifyingCode = false;

    QString login    = loginEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString email    = emailEdit->text().trimmed();

    QByteArray hashBytes = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString passwordHash = QString::fromLatin1(hashBytes.toHex());
    QString request      = QString("registration||%1||%2||%3").arg(login, passwordHash, email);

    ClientSingleton::instance().sendRequestAsync(request);
}

void RegWidget::onRegistrationResponseReceived(const QString &response)
{
    QString r = response.trimmed();
    if (r.isEmpty()) return;

    if (m_verifyingCode) {
        // Ответ на verify_reg
        m_verifyingCode = false;
        verifyCodeBtn->setEnabled(true);

        if (r.startsWith("reg+")) {
            codeStatusLabel->setText("Регистрация успешна! Выполняется вход...");
            codeStatusLabel->setStyleSheet("QLabel { color: #388E3C; font-size: 10pt; }");
            codeStatusLabel->show();
            codeErrorLabel->hide();
            verifyCodeBtn->setEnabled(false);
            QTimer::singleShot(2000, this, [this]() {
                emit registrationSuccess();
            });
            return;
        }

        if (r.startsWith("reg-")) {
            codeFailedAttempts++;
            codeStatusLabel->hide();
            if (codeFailedAttempts < 4) {
                codeErrorLabel->setText(
                    QString("Неверный код. Осталось попыток: %1").arg(4 - codeFailedAttempts)
                );
                codeErrorLabel->show();
                return;
            }
            if (codeFailedAttempts == 4) { applyCodeLock(0,    "Слишком много попыток. Заблокировано на 30 секунд"); return; }
            if (codeFailedAttempts == 5) { applyCodeLock(5,    "Слишком много попыток. Заблокировано на 5 минут");   return; }
            if (codeFailedAttempts == 6) { applyCodeLock(10,   "Слишком много попыток. Заблокировано на 10 минут");  return; }
            applyCodeLock(9999, "Слишком много попыток. Аккаунт заблокирован на длительное время");
            return;
        }

        codeErrorLabel->setText("Ошибка соединения с сервером.");
        codeErrorLabel->show();
        codeStatusLabel->hide();
        return;
    }

    // Ответ на registration
    if (r == "reg_code_sent") {
        codeStatusLabel->setText("Код отправлен на почту");
        codeStatusLabel->setStyleSheet("QLabel { color: #388E3C; font-size: 10pt; }");
        codeStatusLabel->show();
        codeEdit->show();
        verifyCodeBtn->show();
        codeErrorLabel->hide();
        codeFailedAttempts = 0;
        codeIsLocked = false;
        confirmEmailBtn->setText("Код отправлен");
    } else if (r.startsWith("reg-")) {
        codeStatusLabel->hide();
        codeErrorLabel->setText("Ошибка регистрации: " + r);
        codeErrorLabel->show();
        confirmEmailBtn->setEnabled(true);
        confirmEmailBtn->setText("Подтвердить почту");
    }
}

void RegWidget::onCodeTextChanged(const QString &text)
{
    Q_UNUSED(text)
}

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
        QString timeStr = remainingMin > 0
            ? QString("Осталось %1 мин %2 сек").arg(remainingMin).arg(remainingSecMod)
            : QString("Осталось %1 сек").arg(remainingSec);
        codeErrorLabel->setText("Аккаунт заблокирован. " + timeStr);
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
    codeStatusLabel->setStyleSheet("QLabel { color: #888888; font-size: 10pt; }");
    codeStatusLabel->show();

    QString login   = loginEdit->text().trimmed();
    QString request = QString("verify_reg||%1||%2").arg(login, code);
    ClientSingleton::instance().sendRequestAsync(request);
}

void RegWidget::onShowAuthClicked()
{
    emit showAuth();
}
