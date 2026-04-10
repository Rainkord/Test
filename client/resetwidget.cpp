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

// ── GitHub dark palette ────────────────────────────────────────────────────
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

// ── Shared style helpers ───────────────────────────────────────────────────
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
    ).arg(GH_INPUT_BG).arg(GH_TEXT).arg(GH_BORDER)
     .arg(FONT_FAMILY).arg(FONT_SIZE_INPUT).arg(GH_BLUE);
}

static QString primaryBtnStyle(bool enabled)
{
    if (enabled) {
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
        ).arg(GH_GREEN).arg(GH_GREEN_H).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
    } else {
        return QString(
            "QPushButton {"
            "  background-color: rgba(35,134,54,0.35);"
            "  color: rgba(255,255,255,0.4);"
            "  border: 1px solid rgba(240,246,252,0.05);"
            "  border-radius: 6px;"
            "  padding: 6px 16px;"
            "  font-family: '%1';"
            "  font-size: %2pt;"
            "  font-weight: bold;"
            "}"
        ).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
    }
}

static QString blueBtnStyle(bool enabled)
{
    if (enabled) {
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
        ).arg(GH_BLUE).arg(GH_BLUE_H).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
    } else {
        return QString(
            "QPushButton {"
            "  background-color: rgba(56,139,253,0.3);"
            "  color: rgba(255,255,255,0.4);"
            "  border: 1px solid rgba(240,246,252,0.05);"
            "  border-radius: 6px;"
            "  padding: 6px 16px;"
            "  font-family: '%1';"
            "  font-size: %2pt;"
            "  font-weight: bold;"
            "}"
        ).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
    }
}

static QString ghostBtnStyle()
{
    return QString(
        "QPushButton {"
        "  background-color: %1;"
        "  color: %3;"
        "  border: 1px solid %4;"
        "  border-radius: 6px;"
        "  padding: 5px 10px;"
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

static QString successLabelStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_GREEN_H).arg(FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

// ── Constructor ───────────────────────────────────────────────────────────
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

    // Фон виджета — тёмный GitHub
    setStyleSheet(QString(
        "QWidget { background-color: %1; color: %2; font-family: '%3'; font-size: %4pt; }")
        .arg(GH_BG).arg(GH_TEXT).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT));

    setupUI();
}

ResetWidget::~ResetWidget() {}

// ── setupUI ────────────────────────────────────────────────────────────────
void ResetWidget::setupUI()
{
    // Внешний layout — центрирует карточку
    QVBoxLayout *outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->addStretch(1);

    QHBoxLayout *outerH = new QHBoxLayout();
    outerH->addStretch(1);

    // Карточка
    QWidget *card = new QWidget(this);
    card->setFixedWidth(380);
    card->setStyleSheet(QString(
        "QWidget {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 10px;"
        "}"
    ).arg(GH_CARD).arg(GH_BORDER));

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 24, 28, 24);
    cardLayout->setSpacing(8);

    // Заголовок
    QLabel *titleLabel = new QLabel("Восстановление пароля", card);
    QFont titleFont(FONT_FAMILY, FONT_SIZE_TITLE, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString("QLabel { color: %1; border: none; }").arg(GH_TEXT));
    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(8);

    // ── STEP 1 — Email ─────────────────────────────
    step1Widget = new QWidget(card);
    step1Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *s1 = new QVBoxLayout(step1Widget);
    s1->setContentsMargins(0, 0, 0, 0);
    s1->setSpacing(6);

    QLabel *emailHint = new QLabel("Введите почту, привязанную к аккаунту:", step1Widget);
    emailHint->setStyleSheet(hintLabelStyle());
    s1->addWidget(emailHint);

    emailEdit = new QLineEdit(step1Widget);
    emailEdit->setPlaceholderText("Email");
    emailEdit->setMinimumHeight(38);
    emailEdit->setStyleSheet(inputStyle());
    s1->addWidget(emailEdit);
    connect(emailEdit, &QLineEdit::textChanged, this, &ResetWidget::onEmailTextChanged);

    emailErrorLabel = new QLabel(step1Widget);
    emailErrorLabel->setStyleSheet(errorLabelStyle());
    emailErrorLabel->hide();
    s1->addWidget(emailErrorLabel);

    s1->addSpacing(4);
    continueBtn = new QPushButton("Продолжить", step1Widget);
    continueBtn->setMinimumHeight(38);
    continueBtn->setEnabled(false);
    continueBtn->setStyleSheet(primaryBtnStyle(false));
    connect(continueBtn, &QPushButton::clicked, this, &ResetWidget::onContinueClicked);
    s1->addWidget(continueBtn);

    cardLayout->addWidget(step1Widget);

    // ── STEP 2 — Code ──────────────────────────────
    step2Widget = new QWidget(card);
    step2Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *s2 = new QVBoxLayout(step2Widget);
    s2->setContentsMargins(0, 0, 0, 0);
    s2->setSpacing(6);

    codeStatusLabel = new QLabel(step2Widget);
    codeStatusLabel->setStyleSheet(successLabelStyle());
    codeStatusLabel->setAlignment(Qt::AlignCenter);
    codeStatusLabel->hide();
    s2->addWidget(codeStatusLabel);

    codeEdit = new QLineEdit(step2Widget);
    codeEdit->setPlaceholderText("Введите код из письма");
    codeEdit->setMaxLength(6);
    codeEdit->setMinimumHeight(38);
    codeEdit->setAlignment(Qt::AlignCenter);
    codeEdit->setStyleSheet(
        inputStyle() +
        "QLineEdit { font-size: 15pt; letter-spacing: 4px; }"
    );
    s2->addWidget(codeEdit);
    connect(codeEdit, &QLineEdit::textChanged, this, &ResetWidget::onCodeTextChanged);

    codeErrorLabel = new QLabel(step2Widget);
    codeErrorLabel->setStyleSheet(errorLabelStyle());
    codeErrorLabel->setAlignment(Qt::AlignCenter);
    codeErrorLabel->hide();
    s2->addWidget(codeErrorLabel);

    s2->addSpacing(4);
    verifyCodeBtn = new QPushButton("Подтвердить код", step2Widget);
    verifyCodeBtn->setMinimumHeight(38);
    verifyCodeBtn->setEnabled(false);
    verifyCodeBtn->setStyleSheet(blueBtnStyle(false));
    connect(verifyCodeBtn, &QPushButton::clicked, this, &ResetWidget::onVerifyCodeClicked);
    s2->addWidget(verifyCodeBtn);

    cardLayout->addWidget(step2Widget);

    // ── STEP 3 — New password ──────────────────────
    step3Widget = new QWidget(card);
    step3Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *s3 = new QVBoxLayout(step3Widget);
    s3->setContentsMargins(0, 0, 0, 0);
    s3->setSpacing(6);

    QLabel *passHint = new QLabel("Придумайте новый пароль:", step3Widget);
    passHint->setStyleSheet(hintLabelStyle());
    s3->addWidget(passHint);

    // Новый пароль
    QHBoxLayout *pass1Row = new QHBoxLayout();
    pass1Row->setSpacing(6);
    newPasswordEdit = new QLineEdit(step3Widget);
    newPasswordEdit->setPlaceholderText("Новый пароль");
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setMinimumHeight(38);
    newPasswordEdit->setStyleSheet(inputStyle());
    pass1Row->addWidget(newPasswordEdit);
    connect(newPasswordEdit, &QLineEdit::textChanged, this, &ResetWidget::onNewPasswordTextChanged);

    togglePassBtn1 = new QPushButton("\xF0\x9F\x91\x81", step3Widget);
    togglePassBtn1->setFixedSize(38, 38);
    togglePassBtn1->setToolTip("Показать/скрыть пароль");
    togglePassBtn1->setStyleSheet(ghostBtnStyle());
    connect(togglePassBtn1, &QPushButton::clicked, this, &ResetWidget::onTogglePassword1);
    pass1Row->addWidget(togglePassBtn1);
    s3->addLayout(pass1Row);

    newPasswordErrorLabel = new QLabel(step3Widget);
    newPasswordErrorLabel->setStyleSheet(errorLabelStyle());
    newPasswordErrorLabel->hide();
    s3->addWidget(newPasswordErrorLabel);

    // Подтвердить пароль
    QHBoxLayout *pass2Row = new QHBoxLayout();
    pass2Row->setSpacing(6);
    confirmPasswordEdit = new QLineEdit(step3Widget);
    confirmPasswordEdit->setPlaceholderText("Подтвердите пароль");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setMinimumHeight(38);
    confirmPasswordEdit->setStyleSheet(inputStyle());
    pass2Row->addWidget(confirmPasswordEdit);
    connect(confirmPasswordEdit, &QLineEdit::textChanged, this, &ResetWidget::onConfirmPasswordTextChanged);

    togglePassBtn2 = new QPushButton("\xF0\x9F\x91\x81", step3Widget);
    togglePassBtn2->setFixedSize(38, 38);
    togglePassBtn2->setToolTip("Показать/скрыть пароль");
    togglePassBtn2->setStyleSheet(ghostBtnStyle());
    connect(togglePassBtn2, &QPushButton::clicked, this, &ResetWidget::onTogglePassword2);
    pass2Row->addWidget(togglePassBtn2);
    s3->addLayout(pass2Row);

    confirmErrorLabel = new QLabel(step3Widget);
    confirmErrorLabel->setStyleSheet(errorLabelStyle());
    confirmErrorLabel->hide();
    s3->addWidget(confirmErrorLabel);

    s3->addSpacing(6);
    saveBtn = new QPushButton("Сохранить пароль", step3Widget);
    saveBtn->setMinimumHeight(38);
    saveBtn->setEnabled(false);
    saveBtn->setStyleSheet(primaryBtnStyle(false));
    connect(saveBtn, &QPushButton::clicked, this, &ResetWidget::onSavePasswordClicked);
    s3->addWidget(saveBtn);

    cardLayout->addWidget(step3Widget);

    // Разделитель + кнопка назад
    cardLayout->addSpacing(8);
    QFrame *line = new QFrame(card);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("QFrame { background: %1; border: none; max-height: 1px; }").arg(GH_BORDER));
    cardLayout->addWidget(line);
    cardLayout->addSpacing(4);

    backBtn = new QPushButton("\u2190 Назад к входу", card);
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

// ── showStep ───────────────────────────────────────────────────────────────
void ResetWidget::showStep(Step step)
{
    m_currentStep = step;
    step1Widget->setVisible(step == StepEmail);
    step2Widget->setVisible(step == StepCode);
    step3Widget->setVisible(step == StepPassword);
}

// ── Helpers ────────────────────────────────────────────────────────────────
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
    bool ok = passOk && confirmOk;
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

// ── Slots — Step 1 ─────────────────────────────────────────────────────────
void ResetWidget::onEmailTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        emailErrorLabel->hide();
        continueBtn->setEnabled(false);
        continueBtn->setStyleSheet(primaryBtnStyle(false));
        return;
    }
    if (!isEmailValid(text)) {
        emailErrorLabel->setText("Неверный формат почты");
        emailErrorLabel->show();
        continueBtn->setEnabled(false);
        continueBtn->setStyleSheet(primaryBtnStyle(false));
    } else {
        emailErrorLabel->hide();
        continueBtn->setEnabled(true);
        continueBtn->setStyleSheet(primaryBtnStyle(true));
    }
}

void ResetWidget::onContinueClicked()
{
    m_email = emailEdit->text().trimmed();
    continueBtn->setEnabled(false);
    continueBtn->setText("Отправляем...");
    continueBtn->setStyleSheet(primaryBtnStyle(false));

    m_waitingForResponse = true;
    ClientSingleton::instance().sendRequestAsync(QString("reset_password||%1").arg(m_email));
}

// ── Slots — Step 2 ─────────────────────────────────────────────────────────
void ResetWidget::onCodeTextChanged(const QString &text)
{
    bool en = !text.trimmed().isEmpty() && !isLocked;
    verifyCodeBtn->setEnabled(en);
    verifyCodeBtn->setStyleSheet(blueBtnStyle(en));
}

void ResetWidget::onVerifyCodeClicked()
{
    if (isLocked) {
        int remainingSec    = lockTimer->remainingTime() / 1000;
        int remainingMin    = remainingSec / 60;
        int remainingSecMod = remainingSec % 60;
        codeErrorLabel->setText(remainingMin > 0
            ? QString("Заблокировано. Осталось %1 мин %2 сек").arg(remainingMin).arg(remainingSecMod)
            : QString("Заблокировано. Осталось %1 сек").arg(remainingSec));
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
    verifyCodeBtn->setStyleSheet(blueBtnStyle(false));
    codeErrorLabel->hide();
    codeStatusLabel->setText("Проверяем код...");
    codeStatusLabel->setStyleSheet(hintLabelStyle());
    codeStatusLabel->show();

    ClientSingleton::instance().sendRequestAsync(
        QString("verify_reset||%1||%2").arg(m_email, m_code));
}

void ResetWidget::onLockTimerFired()
{
    isLocked = false;
    if (!codeEdit->text().trimmed().isEmpty()) {
        verifyCodeBtn->setEnabled(true);
        verifyCodeBtn->setStyleSheet(blueBtnStyle(true));
    }
    codeErrorLabel->hide();
}

// ── Slots — Step 3 ─────────────────────────────────────────────────────────
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
        newPasswordEdit->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password);
}

void ResetWidget::onTogglePassword2()
{
    confirmPasswordEdit->setEchoMode(
        confirmPasswordEdit->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password);
}

void ResetWidget::onSavePasswordClicked()
{
    QByteArray hashBytes = QCryptographicHash::hash(
        newPasswordEdit->text().toUtf8(), QCryptographicHash::Sha256);
    QString passwordHash = QString::fromLatin1(hashBytes.toHex());

    m_waitingForResponse = true;
    saveBtn->setEnabled(false);
    saveBtn->setText("Сохраняем...");
    saveBtn->setStyleSheet(primaryBtnStyle(false));

    ClientSingleton::instance().sendRequestAsync(
        QString("set_new_password||%1||%2||%3").arg(m_email, m_code, passwordHash));
}

// ── Response handler ───────────────────────────────────────────────────────
void ResetWidget::onResetResponseReceived(const QString &response)
{
    if (!m_waitingForResponse) return;
    m_waitingForResponse = false;

    QString r = response.trimmed();

    if (r.isEmpty()) {
        if (m_currentStep == StepEmail) {
            continueBtn->setEnabled(true);
            continueBtn->setText("Продолжить");
            continueBtn->setStyleSheet(primaryBtnStyle(true));
        } else if (m_currentStep == StepCode) {
            verifyCodeBtn->setEnabled(true);
            verifyCodeBtn->setStyleSheet(blueBtnStyle(true));
            codeStatusLabel->hide();
            codeErrorLabel->setText("Ошибка соединения с сервером.");
            codeErrorLabel->show();
        } else {
            saveBtn->setEnabled(true);
            saveBtn->setText("Сохранить пароль");
            saveBtn->setStyleSheet(primaryBtnStyle(true));
        }
        return;
    }

    // Step 1
    if (m_currentStep == StepEmail) {
        if (r == "reset_code_sent") {
            continueBtn->setText("Код отправлен");
            codeStatusLabel->setText("Код отправлен на почту " + m_email);
            codeStatusLabel->setStyleSheet(successLabelStyle());
            codeStatusLabel->show();
            codeEdit->clear();
            codeErrorLabel->hide();
            verifyCodeBtn->setEnabled(false);
            verifyCodeBtn->setStyleSheet(blueBtnStyle(false));
            failedAttempts = 0;
            isLocked = false;
            showStep(StepCode);
        } else {
            continueBtn->setEnabled(true);
            continueBtn->setText("Продолжить");
            continueBtn->setStyleSheet(primaryBtnStyle(true));
            emailErrorLabel->setText("Почта не найдена или ошибка сервера.");
            emailErrorLabel->show();
        }
        return;
    }

    // Step 2
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
            saveBtn->setStyleSheet(primaryBtnStyle(false));
            showStep(StepPassword);
            return;
        }
        verifyCodeBtn->setEnabled(true);
        verifyCodeBtn->setStyleSheet(blueBtnStyle(true));
        codeStatusLabel->hide();
        failedAttempts++;
        if (failedAttempts < 4) {
            codeErrorLabel->setText(
                QString("Неверный код. Осталось попыток: %1").arg(4 - failedAttempts));
            codeErrorLabel->show();
            return;
        }
        if (failedAttempts == 4) { applyLock(0,    "Слишком много попыток. Заблокировано на 30 секунд"); return; }
        if (failedAttempts == 5) { applyLock(5,    "Слишком много попыток. Заблокировано на 5 минут");   return; }
        if (failedAttempts == 6) { applyLock(10,   "Слишком много попыток. Заблокировано на 10 минут");  return; }
        applyLock(9999, "Слишком много попыток. Заблокировано навсегда");
        return;
    }

    // Step 3
    if (m_currentStep == StepPassword) {
        if (r == "password_changed") {
            QTimer::singleShot(500, this, [this]() { emit resetSuccess(); });
        } else {
            saveBtn->setEnabled(true);
            saveBtn->setText("Сохранить пароль");
            saveBtn->setStyleSheet(primaryBtnStyle(true));
            confirmErrorLabel->setText("Ошибка сервера. Попробуйте снова.");
            confirmErrorLabel->show();
        }
    }
}

// ── Back ───────────────────────────────────────────────────────────────────
void ResetWidget::onBackClicked()
{
    m_waitingForResponse = false;
    isLocked = false;
    failedAttempts = 0;
    if (lockTimer->isActive()) lockTimer->stop();

    emailEdit->clear();
    emailErrorLabel->hide();
    continueBtn->setEnabled(false);
    continueBtn->setText("Продолжить");
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
