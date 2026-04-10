#include "resetwidget.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QFont>
#include <QCryptographicHash>
#include <QRegularExpression>

ResetWidget::ResetWidget(QWidget *parent)
    : QWidget(parent),
      failedAttempts(0),
      isLocked(false),
      m_currentStep(StepEmail),
      m_waitingForResponse(false)
{
    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &ResetWidget::onLockTimerFired);

    connect(&ClientSingleton::instance(), &ClientSingleton::responseReceived,
            this, &ResetWidget::onResetResponseReceived);

    setupUI();
}

ResetWidget::~ResetWidget() {}

void ResetWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 20, 40, 20);
    mainLayout->setSpacing(0);

    QLabel *titleLabel = new QLabel("Восстановление пароля", this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(18);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(14);

    // ─────────────────────────────────────────────
    // STEP 1 — Email
    // ─────────────────────────────────────────────
    step1Widget = new QWidget(this);
    QVBoxLayout *s1 = new QVBoxLayout(step1Widget);
    s1->setContentsMargins(0, 0, 0, 0);
    s1->setSpacing(4);

    QLabel *emailHint = new QLabel("Введите почту, привязанную к аккаунту:", step1Widget);
    emailHint->setStyleSheet("QLabel { color: #555555; font-size: 10pt; }");
    s1->addWidget(emailHint);
    s1->addSpacing(4);

    emailEdit = new QLineEdit(step1Widget);
    emailEdit->setPlaceholderText("Email");
    emailEdit->setMinimumHeight(36);
    emailEdit->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #cccccc; border-radius: 4px; }");
    s1->addWidget(emailEdit);
    connect(emailEdit, &QLineEdit::textChanged, this, &ResetWidget::onEmailTextChanged);

    emailErrorLabel = new QLabel(step1Widget);
    emailErrorLabel->setStyleSheet("QLabel { color: red; font-size: 10pt; }");
    emailErrorLabel->hide();
    s1->addWidget(emailErrorLabel);
    s1->addSpacing(10);

    continueBtn = new QPushButton("Продолжить", step1Widget);
    continueBtn->setMinimumHeight(38);
    continueBtn->setEnabled(false);
    continueBtn->setStyleSheet(
        "QPushButton { background-color: #cccccc; color: #666666; border: none; border-radius: 4px; font-size: 13pt; }"
    );
    connect(continueBtn, &QPushButton::clicked, this, &ResetWidget::onContinueClicked);
    s1->addWidget(continueBtn);

    mainLayout->addWidget(step1Widget);

    // ─────────────────────────────────────────────
    // STEP 2 — Code
    // ─────────────────────────────────────────────
    step2Widget = new QWidget(this);
    QVBoxLayout *s2 = new QVBoxLayout(step2Widget);
    s2->setContentsMargins(0, 0, 0, 0);
    s2->setSpacing(6);

    codeStatusLabel = new QLabel(step2Widget);
    codeStatusLabel->setStyleSheet("QLabel { color: #388E3C; font-size: 10pt; }");
    codeStatusLabel->setAlignment(Qt::AlignCenter);
    codeStatusLabel->hide();
    s2->addWidget(codeStatusLabel);

    codeEdit = new QLineEdit(step2Widget);
    codeEdit->setPlaceholderText("Введите код из письма");
    codeEdit->setMaxLength(6);
    codeEdit->setMinimumHeight(36);
    codeEdit->setAlignment(Qt::AlignCenter);
    codeEdit->setStyleSheet(
        "QLineEdit { padding: 4px 8px; border: 1px solid #cccccc; border-radius: 4px; "
        "font-size: 14pt; letter-spacing: 4px; }"
    );
    s2->addWidget(codeEdit);
    connect(codeEdit, &QLineEdit::textChanged, this, &ResetWidget::onCodeTextChanged);

    codeErrorLabel = new QLabel(step2Widget);
    codeErrorLabel->setStyleSheet("QLabel { color: red; font-size: 10pt; }");
    codeErrorLabel->setAlignment(Qt::AlignCenter);
    codeErrorLabel->hide();
    s2->addWidget(codeErrorLabel);

    verifyCodeBtn = new QPushButton("Подтвердить код", step2Widget);
    verifyCodeBtn->setMinimumHeight(38);
    verifyCodeBtn->setEnabled(false);
    verifyCodeBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 4px; font-size: 12pt; }"
        "QPushButton:hover { background-color: #388E3C; }"
        "QPushButton:disabled { background-color: #cccccc; color: #666666; }"
    );
    connect(verifyCodeBtn, &QPushButton::clicked, this, &ResetWidget::onVerifyCodeClicked);
    s2->addWidget(verifyCodeBtn);

    mainLayout->addWidget(step2Widget);

    // ─────────────────────────────────────────────
    // STEP 3 — New password
    // ─────────────────────────────────────────────
    step3Widget = new QWidget(this);
    QVBoxLayout *s3 = new QVBoxLayout(step3Widget);
    s3->setContentsMargins(0, 0, 0, 0);
    s3->setSpacing(4);

    QLabel *passHint = new QLabel("Придумайте новый пароль:", step3Widget);
    passHint->setStyleSheet("QLabel { color: #555555; font-size: 10pt; }");
    s3->addWidget(passHint);
    s3->addSpacing(4);

    // New password row
    QHBoxLayout *pass1Row = new QHBoxLayout();
    pass1Row->setSpacing(6);

    newPasswordEdit = new QLineEdit(step3Widget);
    newPasswordEdit->setPlaceholderText("Новый пароль");
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setMinimumHeight(36);
    newPasswordEdit->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #cccccc; border-radius: 4px; }");
    pass1Row->addWidget(newPasswordEdit);
    connect(newPasswordEdit, &QLineEdit::textChanged, this, &ResetWidget::onNewPasswordTextChanged);

    togglePassBtn1 = new QPushButton("👁", step3Widget);
    togglePassBtn1->setFixedWidth(35);
    togglePassBtn1->setFixedHeight(36);
    togglePassBtn1->setToolTip("Показать/скрыть пароль");
    togglePassBtn1->setStyleSheet(
        "QPushButton { border: 1px solid #cccccc; border-radius: 4px; background: #f5f5f5; }"
        "QPushButton:hover { background: #e0e0e0; }"
    );
    connect(togglePassBtn1, &QPushButton::clicked, this, &ResetWidget::onTogglePassword1);
    pass1Row->addWidget(togglePassBtn1);
    s3->addLayout(pass1Row);

    newPasswordErrorLabel = new QLabel(step3Widget);
    newPasswordErrorLabel->setStyleSheet("QLabel { color: red; font-size: 10pt; }");
    newPasswordErrorLabel->hide();
    s3->addWidget(newPasswordErrorLabel);
    s3->addSpacing(6);

    // Confirm password row
    QHBoxLayout *pass2Row = new QHBoxLayout();
    pass2Row->setSpacing(6);

    confirmPasswordEdit = new QLineEdit(step3Widget);
    confirmPasswordEdit->setPlaceholderText("Подтвердите пароль");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setMinimumHeight(36);
    confirmPasswordEdit->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #cccccc; border-radius: 4px; }");
    pass2Row->addWidget(confirmPasswordEdit);
    connect(confirmPasswordEdit, &QLineEdit::textChanged, this, &ResetWidget::onConfirmPasswordTextChanged);

    togglePassBtn2 = new QPushButton("👁", step3Widget);
    togglePassBtn2->setFixedWidth(35);
    togglePassBtn2->setFixedHeight(36);
    togglePassBtn2->setToolTip("Показать/скрыть пароль");
    togglePassBtn2->setStyleSheet(
        "QPushButton { border: 1px solid #cccccc; border-radius: 4px; background: #f5f5f5; }"
        "QPushButton:hover { background: #e0e0e0; }"
    );
    connect(togglePassBtn2, &QPushButton::clicked, this, &ResetWidget::onTogglePassword2);
    pass2Row->addWidget(togglePassBtn2);
    s3->addLayout(pass2Row);

    confirmErrorLabel = new QLabel(step3Widget);
    confirmErrorLabel->setStyleSheet("QLabel { color: red; font-size: 10pt; }");
    confirmErrorLabel->hide();
    s3->addWidget(confirmErrorLabel);
    s3->addSpacing(10);

    saveBtn = new QPushButton("Сохранить пароль", step3Widget);
    saveBtn->setMinimumHeight(38);
    saveBtn->setEnabled(false);
    saveBtn->setStyleSheet(
        "QPushButton { background-color: #cccccc; color: #666666; border: none; border-radius: 4px; font-size: 13pt; }"
    );
    connect(saveBtn, &QPushButton::clicked, this, &ResetWidget::onSavePasswordClicked);
    s3->addWidget(saveBtn);

    mainLayout->addWidget(step3Widget);
    mainLayout->addSpacing(12);

    // ─────────────────────────────────────────────
    // Back link (always visible)
    // ─────────────────────────────────────────────
    backBtn = new QPushButton("← Назад к входу", this);
    backBtn->setFlat(true);
    backBtn->setStyleSheet(
        "QPushButton { color: #2196F3; border: none; text-decoration: underline; font-size: 11pt; }"
        "QPushButton:hover { color: #1565C0; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &ResetWidget::onBackClicked);
    mainLayout->addWidget(backBtn, 0, Qt::AlignCenter);

    mainLayout->addStretch();

    showStep(StepEmail);
}

void ResetWidget::showStep(Step step)
{
    m_currentStep = step;
    step1Widget->setVisible(step == StepEmail);
    step2Widget->setVisible(step == StepCode);
    step3Widget->setVisible(step == StepPassword);
}

bool ResetWidget::isEmailValid(const QString &email) const
{
    QRegularExpression re("^[a-zA-Z0-9._%+\\-]+@[a-zA-Z0-9.\\-]+\\.[a-zA-Z]{2,}$");
    return re.match(email).hasMatch();
}

void ResetWidget::validatePasswords()
{
    bool passOk    = (newPasswordEdit->text().length() >= 8);
    bool confirmOk = (!confirmPasswordEdit->text().isEmpty()
                      && confirmPasswordEdit->text() == newPasswordEdit->text());

    if (passOk && confirmOk) {
        saveBtn->setEnabled(true);
        saveBtn->setStyleSheet(
            "QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 4px; font-size: 13pt; }"
            "QPushButton:hover { background-color: #388E3C; }"
        );
    } else {
        saveBtn->setEnabled(false);
        saveBtn->setStyleSheet(
            "QPushButton { background-color: #cccccc; color: #666666; border: none; border-radius: 4px; font-size: 13pt; }"
        );
    }
}

void ResetWidget::applyLock(int minutes, const QString &message)
{
    isLocked = true;
    verifyCodeBtn->setEnabled(false);
    codeErrorLabel->setText(message);
    codeErrorLabel->show();
    lockTimer->start(minutes == 0 ? 30 * 1000 : minutes * 60 * 1000);
}

// ─────────────────────────────────────────────
// Slots — Step 1
// ─────────────────────────────────────────────
void ResetWidget::onEmailTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        emailErrorLabel->hide();
        continueBtn->setEnabled(false);
        continueBtn->setStyleSheet(
            "QPushButton { background-color: #cccccc; color: #666666; border: none; border-radius: 4px; font-size: 13pt; }"
        );
        return;
    }
    if (!isEmailValid(text)) {
        emailErrorLabel->setText("Неверный формат почты");
        emailErrorLabel->show();
        continueBtn->setEnabled(false);
        continueBtn->setStyleSheet(
            "QPushButton { background-color: #cccccc; color: #666666; border: none; border-radius: 4px; font-size: 13pt; }"
        );
    } else {
        emailErrorLabel->hide();
        continueBtn->setEnabled(true);
        continueBtn->setStyleSheet(
            "QPushButton { background-color: #2196F3; color: white; border: none; border-radius: 4px; font-size: 13pt; }"
            "QPushButton:hover { background-color: #1976D2; }"
        );
    }
}

void ResetWidget::onContinueClicked()
{
    m_email = emailEdit->text().trimmed();
    continueBtn->setEnabled(false);
    continueBtn->setText("Отправляем...");

    m_waitingForResponse = true;
    QString request = QString("reset_password||%1").arg(m_email);
    ClientSingleton::instance().sendRequestAsync(request);
}

// ─────────────────────────────────────────────
// Slots — Step 2
// ─────────────────────────────────────────────
void ResetWidget::onCodeTextChanged(const QString &text)
{
    verifyCodeBtn->setEnabled(!text.trimmed().isEmpty() && !isLocked);
}

void ResetWidget::onVerifyCodeClicked()
{
    if (isLocked) {
        int remainingSec    = lockTimer->remainingTime() / 1000;
        int remainingMin    = remainingSec / 60;
        int remainingSecMod = remainingSec % 60;
        QString timeStr = remainingMin > 0
            ? QString("Осталось %1 мин %2 сек").arg(remainingMin).arg(remainingSecMod)
            : QString("Осталось %1 сек").arg(remainingSec);
        codeErrorLabel->setText("Заблокировано. " + timeStr);
        codeErrorLabel->show();
        return;
    }

    m_code = codeEdit->text().trimmed();
    if (m_code.isEmpty()) {
        codeErrorLabel->setText("Введите код из письма.");
        codeErrorLabel->show();
        return;
    }

    m_waitingForResponse = true;
    verifyCodeBtn->setEnabled(false);
    codeErrorLabel->hide();
    codeStatusLabel->setText("Проверяем код...");
    codeStatusLabel->setStyleSheet("QLabel { color: #888888; font-size: 10pt; }");
    codeStatusLabel->show();

    QString request = QString("verify_reset||%1||%2").arg(m_email, m_code);
    ClientSingleton::instance().sendRequestAsync(request);
}

void ResetWidget::onLockTimerFired()
{
    isLocked = false;
    verifyCodeBtn->setEnabled(true);
    codeErrorLabel->hide();
}

// ─────────────────────────────────────────────
// Slots — Step 3
// ─────────────────────────────────────────────
void ResetWidget::onNewPasswordTextChanged(const QString &text)
{
    if (text.length() < 8 && !text.isEmpty()) {
        newPasswordErrorLabel->setText("Минимум 8 символов");
        newPasswordErrorLabel->show();
    } else {
        newPasswordErrorLabel->hide();
    }
    validatePasswords();
}

void ResetWidget::onConfirmPasswordTextChanged(const QString &text)
{
    if (!text.isEmpty() && text != newPasswordEdit->text()) {
        confirmErrorLabel->setText("Пароли не совпадают");
        confirmErrorLabel->show();
    } else {
        confirmErrorLabel->hide();
    }
    validatePasswords();
}

void ResetWidget::onTogglePassword1()
{
    newPasswordEdit->setEchoMode(
        newPasswordEdit->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password
    );
}

void ResetWidget::onTogglePassword2()
{
    confirmPasswordEdit->setEchoMode(
        confirmPasswordEdit->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password
    );
}

void ResetWidget::onSavePasswordClicked()
{
    QString newPass = newPasswordEdit->text();
    QByteArray hashBytes = QCryptographicHash::hash(newPass.toUtf8(), QCryptographicHash::Sha256);
    QString passwordHash = QString::fromLatin1(hashBytes.toHex());

    m_waitingForResponse = true;
    saveBtn->setEnabled(false);
    saveBtn->setText("Сохраняем...");

    QString request = QString("set_new_password||%1||%2||%3").arg(m_email, m_code, passwordHash);
    ClientSingleton::instance().sendRequestAsync(request);
}

// ─────────────────────────────────────────────
// Response handler
// ─────────────────────────────────────────────
void ResetWidget::onResetResponseReceived(const QString &response)
{
    if (!m_waitingForResponse) return;
    m_waitingForResponse = false;

    QString r = response.trimmed();
    if (r.isEmpty()) {
        if (m_currentStep == StepEmail) {
            continueBtn->setEnabled(true);
            continueBtn->setText("Продолжить");
        } else if (m_currentStep == StepCode) {
            verifyCodeBtn->setEnabled(true);
            codeStatusLabel->hide();
            codeErrorLabel->setText("Ошибка соединения с сервером.");
            codeErrorLabel->show();
        } else {
            saveBtn->setEnabled(true);
            saveBtn->setText("Сохранить пароль");
        }
        return;
    }

    // ── Step 1 responses ──────────────────────────
    if (m_currentStep == StepEmail) {
        if (r == "reset_code_sent") {
            continueBtn->setText("Код отправлен");
            codeStatusLabel->setText("Код отправлен на почту " + m_email);
            codeStatusLabel->setStyleSheet("QLabel { color: #388E3C; font-size: 10pt; }");
            codeStatusLabel->show();
            codeEdit->clear();
            codeErrorLabel->hide();
            verifyCodeBtn->setEnabled(false);
            failedAttempts = 0;
            isLocked = false;
            showStep(StepCode);
        } else {
            // reset_error или неизвестно
            continueBtn->setEnabled(true);
            continueBtn->setText("Продолжить");
            emailErrorLabel->setText("Почта не найдена или ошибка сервера.");
            emailErrorLabel->show();
        }
        return;
    }

    // ── Step 2 responses ──────────────────────────
    if (m_currentStep == StepCode) {
        if (r == "reset_code_ok") {
            codeStatusLabel->hide();
            codeErrorLabel->hide();
            newPasswordEdit->clear();
            confirmPasswordEdit->clear();
            newPasswordErrorLabel->hide();
            confirmErrorLabel->hide();
            saveBtn->setEnabled(false);
            saveBtn->setText("Сохранить пароль");
            saveBtn->setStyleSheet(
                "QPushButton { background-color: #cccccc; color: #666666; border: none; border-radius: 4px; font-size: 13pt; }"
            );
            showStep(StepPassword);
            return;
        }
        // wrong code
        verifyCodeBtn->setEnabled(true);
        codeStatusLabel->hide();
        failedAttempts++;
        if (failedAttempts < 4) {
            codeErrorLabel->setText(
                QString("Неверный код. Осталось попыток: %1").arg(4 - failedAttempts)
            );
            codeErrorLabel->show();
            return;
        }
        if (failedAttempts == 4) { applyLock(0,    "Слишком много попыток. Заблокировано на 30 секунд"); return; }
        if (failedAttempts == 5) { applyLock(5,    "Слишком много попыток. Заблокировано на 5 минут");   return; }
        if (failedAttempts == 6) { applyLock(10,   "Слишком много попыток. Заблокировано на 10 минут");  return; }
        applyLock(9999, "Слишком много попыток. Заблокировано на длительное время");
        return;
    }

    // ── Step 3 responses ──────────────────────────
    if (m_currentStep == StepPassword) {
        if (r == "password_changed") {
            QTimer::singleShot(500, this, [this]() {
                emit resetSuccess();
            });
        } else {
            saveBtn->setEnabled(true);
            saveBtn->setText("Сохранить пароль");
            saveBtn->setStyleSheet(
                "QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 4px; font-size: 13pt; }"
                "QPushButton:hover { background-color: #388E3C; }"
            );
            confirmErrorLabel->setText("Ошибка сервера. Попробуйте снова.");
            confirmErrorLabel->show();
        }
    }
}

// ─────────────────────────────────────────────
// Back
// ─────────────────────────────────────────────
void ResetWidget::onBackClicked()
{
    // Full reset of state
    m_waitingForResponse = false;
    isLocked = false;
    failedAttempts = 0;
    if (lockTimer->isActive()) lockTimer->stop();

    emailEdit->clear();
    emailErrorLabel->hide();
    continueBtn->setEnabled(false);
    continueBtn->setText("Продолжить");
    continueBtn->setStyleSheet(
        "QPushButton { background-color: #cccccc; color: #666666; border: none; border-radius: 4px; font-size: 13pt; }"
    );

    codeEdit->clear();
    codeErrorLabel->hide();
    codeStatusLabel->hide();
    verifyCodeBtn->setEnabled(false);

    newPasswordEdit->clear();
    confirmPasswordEdit->clear();
    newPasswordErrorLabel->hide();
    confirmErrorLabel->hide();

    showStep(StepEmail);
    emit backToAuth();
}
