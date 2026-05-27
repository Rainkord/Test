#include "regwidget.h"
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
#include <QString>
#include <QKeyEvent>

// ── GitHub dark palette ───────────────────────────────────────────────────────────────────
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
#define GH_BTN_GHOST    "#21262d"
#define GH_BTN_GHOST_H  "#30363d"

#define FONT_FAMILY      "Segoe UI"
#define FONT_SIZE_TITLE  16
#define FONT_SIZE_BTN    11
#define FONT_SIZE_INPUT  11
#define FONT_SIZE_SMALL   9

/**
 * @brief Генератор стиля для полей ввода
 * 
 * @return Строка CSS-стиля для QLineEdit в стиле GitHub dark
 */
QString RegWidget::inputStyle() const
{
    return QString(
        "QLineEdit {"
        "  background-color: %1; color: %2;"
        "  border: 1px solid %3; border-radius: 6px;"
        "  padding: 6px 10px; font-family: '%4'; font-size: %5pt;"
        "}"
        "QLineEdit:focus { border-color: %6; }"
        "QLineEdit:read-only { color: %7; }"
    ).arg(GH_INPUT_BG).arg(GH_TEXT).arg(GH_BORDER)
     .arg(FONT_FAMILY).arg(FONT_SIZE_INPUT).arg(GH_BLUE).arg(GH_MUTED);
}

/**
 * @brief Генератор стиля для основной кнопки (зелёная)
 * 
 * @return Строка CSS-стиля для QPushButton в стиле GitHub green
 */
QString RegWidget::primaryBtnStyle() const
{
    return QString(
        "QPushButton {"
        "  background-color: %1; color: #ffffff;"
        "  border: 1px solid rgba(240,246,252,0.1); border-radius: 6px;"
        "  padding: 6px 16px; font-family: '%3'; font-size: %4pt; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:disabled { background-color: rgba(35,134,54,0.4); color: rgba(255,255,255,0.4); }"
    ).arg(GH_GREEN).arg(GH_GREEN_H).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

/**
 * @brief Генератор стиля для вторичной кнопки
 * 
 * @return Строка CSS-стиля для QPushButton с тёмным фоном
 */
QString RegWidget::secondaryBtnStyle() const
{
    return QString(
        "QPushButton {"
        "  background-color: %1; color: %3;"
        "  border: 1px solid %4; border-radius: 6px;"
        "  padding: 6px 16px; font-family: '%5'; font-size: %6pt; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: %2; }"
    ).arg(GH_BTN_GHOST).arg(GH_BTN_GHOST_H).arg(GH_TEXT)
     .arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

/**
 * @brief Генератор стиля для призрачной кнопки
 * 
 * @return Строка CSS-стиля для QPushButton с прозрачным фоном
 */
QString RegWidget::ghostBtnStyle() const
{
    return QString(
        "QPushButton {"
        "  background-color: %1; color: %3;"
        "  border: 1px solid %4; border-radius: 6px;"
        "  padding: 5px 14px; font-family: '%5'; font-size: %6pt;"
        "}"
        "QPushButton:hover { background-color: %2; }"
    ).arg(GH_BTN_GHOST).arg(GH_BTN_GHOST_H).arg(GH_TEXT)
     .arg(GH_BORDER).arg(FONT_FAMILY).arg(FONT_SIZE_BTN);
}

/**
 * @brief Генератор стиля для кнопки-ссылки
 * 
 * @return Строка CSS-стиля для QPushButton в виде ссылки
 */
QString RegWidget::linkBtnStyle() const
{
    return QString(
        "QPushButton { color: %1; border: none; background: transparent;"
        " font-family: '%2'; font-size: %3pt; }"
        "QPushButton:hover { color: %4; text-decoration: underline; }"
    ).arg(GH_BLUE).arg(FONT_FAMILY).arg(FONT_SIZE_BTN).arg(GH_BLUE_H);
}

/**
 * @brief Генератор стиля для метки ошибки
 * 
 * @return Строка CSS-стиля для QLabel с красным цветом текста
 */
QString RegWidget::errorLabelStyle() const
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_RED).arg(FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

/**
 * @brief Генератор стиля для информационной метки
 * 
 * @return Строка CSS-стиля для QLabel с серым цветом текста
 */
QString RegWidget::infoLabelStyle() const
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_MUTED).arg(FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

/**
 * @brief Генератор стиля для метки успешного действия
 * 
 * @return Строка CSS-стиля для QLabel с зелёным цветом текста
 */
QString RegWidget::successLabelStyle() const
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_GREEN_H).arg(FONT_FAMILY).arg(FONT_SIZE_SMALL);
}

/**
 * @brief Конструктор виджета регистрации
 * 
 * Инициализирует переменные состояния, создаёт таймер блокировки,
 * подключает сигнал ответа сервера и настраивает UI.
 * 
 * @param parent Родительский виджет
 */
RegWidget::RegWidget(QWidget *parent)
    : QWidget(parent),
      codeFailedAttempts(0), codeLockLevel(0),
      codeIsLocked(false), m_checkingLogin(false),
      m_waitingForRegCode(false), m_verifyingCode(false)
{
    codeLockTimer = new QTimer(this);
    codeLockTimer->setSingleShot(true);
    connect(codeLockTimer, &QTimer::timeout, this, &RegWidget::onCodeLockTimerFired);

    connect(&ClientSingleton::instance(), &ClientSingleton::responseReceived,
            this, &RegWidget::onRegistrationResponseReceived);

    setStyleSheet(QString(
        "QWidget { background-color: %1; color: %2; font-family: '%3'; font-size: %4pt; }")
        .arg(GH_BG).arg(GH_TEXT).arg(FONT_FAMILY).arg(FONT_SIZE_INPUT));

    setupUI();
}

/**
 * @brief Деструктор виджета регистрации
 */
RegWidget::~RegWidget() {}

/**
 * @brief Очистить все поля и сбросить состояние виджета
 * 
 * Очищает все поля ввода, скрывает статусные метки,
 * сбрасывает флаги состояния и возвращает виджет на шаг 1.
 */
void RegWidget::clearFields()
{
    loginEdit->clear();
    passwordEdit->clear();
    confirmPasswordEdit->clear();
    emailEdit->clear();
    otpCode->clear();

    loginErrorLabel->hide();
    passwordErrorLabel->hide();
    confirmErrorLabel->hide();
    emailErrorLabel->hide();
    codeErrorLabel->hide();
    codeStatusLabel->hide();
    emailHintLabel->hide();

    loginEdit->setReadOnly(false);
    passwordEdit->setReadOnly(false);
    confirmPasswordEdit->setReadOnly(false);

    continueBtn->setEnabled(false);
    continueBtn->setText(QString::fromUtf8("Продолжить"));
    emailNextBtn->setEnabled(false);
    emailNextBtn->setText(QString::fromUtf8("Далее →"));

    codeFailedAttempts = 0;
    codeLockLevel      = 0;
    codeIsLocked       = false;
    m_verifyingCode    = false;
    m_waitingForRegCode = false;
    m_pendingPassHash.clear();
    m_pendingCodeHash.clear();
    m_checkingLogin    = false;
    currentLogin.clear();
    currentEmail.clear();

    if (codeLockTimer->isActive())
        codeLockTimer->stop();

    showStep(1);
}

/**
 * @brief Обработчик нажатия клавиш для навигации
 * 
 * При нажатии Escape:
 * - На шаге 3: возврат к шагу 2
 * - На шаге 2: возврат к шагу 1
 * - На шаге 1: переход на экран авторизации
 * 
 * @param e Событие нажатия клавиши
 */
void RegWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        if (step3Widget->isVisible()) {
            onBackToStep2Clicked();
        } else if (step2Widget->isVisible()) {
            onBackToStep1Clicked();
        } else {
            emit showAuth();
        }
    } else {
        QWidget::keyPressEvent(e);
    }
}

/**
 * @brief Настройка пользовательского интерфейса
 * 
 * Создаёт трёхшаговую форму регистрации с карточкой, содержащей:
 * - Шаг 1: поля логина и пароля с валидацией
 * - Шаг 2: поле email с проверкой формата
 * - Шаг 3: OTP-ввод кода подтверждения
 * А также ссылку для перехода к авторизации.
 */
void RegWidget::setupUI()
{
    QVBoxLayout *outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->addStretch(1);

    QHBoxLayout *outerH = new QHBoxLayout();
    outerH->addStretch(1);

    QWidget *card = new QWidget(this);
    card->setFixedWidth(420);
    card->setStyleSheet(QString(
        "QWidget { background-color: %1; border: 1px solid %2; border-radius: 10px; }"
    ).arg(GH_CARD).arg(GH_BORDER));

    QVBoxLayout *mainLayout = new QVBoxLayout(card);
    mainLayout->setContentsMargins(28, 28, 28, 28);
    mainLayout->setSpacing(8);

    QLabel *titleLabel = new QLabel(QString::fromUtf8("Регистрация"), card);
    QFont titleFont(FONT_FAMILY, FONT_SIZE_TITLE, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString("QLabel { color: %1; border: none; }").arg(GH_TEXT));
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(6);

    // ── Шаг 1: логин + пароль ──────────────────────────────────────────────────────
    step1Widget = new QWidget(card);
    step1Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *s1 = new QVBoxLayout(step1Widget);
    s1->setContentsMargins(0, 0, 0, 0);
    s1->setSpacing(6);

    loginEdit = new QLineEdit(step1Widget);
    loginEdit->setPlaceholderText(QString::fromUtf8("Логин (мин. 4 символа)"));
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
    passwordEdit->setPlaceholderText(QString::fromUtf8("Пароль (мин. 8 символов)"));
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(38);
    passwordEdit->setStyleSheet(inputStyle());
    pass1Row->addWidget(passwordEdit);
    connect(passwordEdit, &QLineEdit::textChanged, this, &RegWidget::onPasswordTextChanged);

    togglePassBtn1 = new QPushButton("\xF0\x9F\x91\x81", step1Widget);
    togglePassBtn1->setFixedSize(38, 38);
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
    confirmPasswordEdit->setPlaceholderText(QString::fromUtf8("Подтвердите пароль"));
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setMinimumHeight(38);
    confirmPasswordEdit->setStyleSheet(inputStyle());
    pass2Row->addWidget(confirmPasswordEdit);
    connect(confirmPasswordEdit, &QLineEdit::textChanged,
            this, &RegWidget::onConfirmPasswordTextChanged);

    togglePassBtn2 = new QPushButton("\xF0\x9F\x91\x81", step1Widget);
    togglePassBtn2->setFixedSize(38, 38);
    togglePassBtn2->setStyleSheet(ghostBtnStyle());
    connect(togglePassBtn2, &QPushButton::clicked, this, &RegWidget::onTogglePassword2);
    pass2Row->addWidget(togglePassBtn2);
    s1->addLayout(pass2Row);

    confirmErrorLabel = new QLabel(step1Widget);
    confirmErrorLabel->setStyleSheet(errorLabelStyle());
    confirmErrorLabel->hide();
    s1->addWidget(confirmErrorLabel);
    s1->addSpacing(4);

    continueBtn = new QPushButton(QString::fromUtf8("Продолжить"), step1Widget);
    continueBtn->setMinimumHeight(38);
    continueBtn->setEnabled(false);
    continueBtn->setDefault(true);
    continueBtn->setAutoDefault(true);
    continueBtn->setStyleSheet(primaryBtnStyle());
    connect(continueBtn, &QPushButton::clicked, this, &RegWidget::onContinueClicked);
    connect(loginEdit, &QLineEdit::returnPressed, continueBtn, &QPushButton::click);
    connect(passwordEdit, &QLineEdit::returnPressed, continueBtn, &QPushButton::click);
    connect(confirmPasswordEdit, &QLineEdit::returnPressed, continueBtn, &QPushButton::click);
    s1->addWidget(continueBtn);

    mainLayout->addWidget(step1Widget);

    // ── Шаг 2: email ───────────────────────────────────────────────────────────────
    step2Widget = new QWidget(card);
    step2Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *s2 = new QVBoxLayout(step2Widget);
    s2->setContentsMargins(0, 0, 0, 0);
    s2->setSpacing(6);

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

    backToStep1Btn = new QPushButton(QString::fromUtf8("← Назад"), step2Widget);
    backToStep1Btn->setMinimumHeight(38);
    backToStep1Btn->setStyleSheet(secondaryBtnStyle());
    connect(backToStep1Btn, &QPushButton::clicked, this, &RegWidget::onBackToStep1Clicked);
    s2Btns->addWidget(backToStep1Btn);

    emailNextBtn = new QPushButton(QString::fromUtf8("Далее →"), step2Widget);
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

    // ── Шаг 3: OTP-ввод кода ─────────────────────────────────────────────────────
    step3Widget = new QWidget(card);
    step3Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *s3 = new QVBoxLayout(step3Widget);
    s3->setContentsMargins(0, 0, 0, 0);
    s3->setSpacing(8);

    emailHintLabel = new QLabel(step3Widget);
    emailHintLabel->setStyleSheet(infoLabelStyle());
    emailHintLabel->setAlignment(Qt::AlignCenter);
    emailHintLabel->setWordWrap(true);
    emailHintLabel->hide();
    s3->addWidget(emailHintLabel);

    otpCode = new OtpInput(step3Widget);
    s3->addWidget(otpCode, 0, Qt::AlignCenter);
    connect(otpCode, &OtpInput::completed,  this, &RegWidget::onCodeCompleted);
    connect(otpCode, &OtpInput::escPressed, this, &RegWidget::onBackToStep2Clicked);

    codeStatusLabel = new QLabel(step3Widget);
    codeStatusLabel->setStyleSheet(successLabelStyle());
    codeStatusLabel->setAlignment(Qt::AlignCenter);
    codeStatusLabel->hide();
    s3->addWidget(codeStatusLabel);

    codeErrorLabel = new QLabel(step3Widget);
    codeErrorLabel->setStyleSheet(errorLabelStyle());
    codeErrorLabel->setAlignment(Qt::AlignCenter);
    codeErrorLabel->hide();
    s3->addWidget(codeErrorLabel);
    s3->addSpacing(4);

    QHBoxLayout *s3Btns = new QHBoxLayout();
    s3Btns->setSpacing(8);

    backToStep2Btn = new QPushButton(QString::fromUtf8("← Изменить почту"), step3Widget);
    backToStep2Btn->setMinimumHeight(38);
    backToStep2Btn->setStyleSheet(secondaryBtnStyle());
    connect(backToStep2Btn, &QPushButton::clicked, this, &RegWidget::onBackToStep2Clicked);
    s3Btns->addWidget(backToStep2Btn);

    verifyCodeBtn = new QPushButton(QString::fromUtf8("Подтвердить"), step3Widget);
    verifyCodeBtn->setMinimumHeight(38);
    verifyCodeBtn->setEnabled(false);
    verifyCodeBtn->setDefault(true);
    verifyCodeBtn->setAutoDefault(true);
    verifyCodeBtn->setStyleSheet(primaryBtnStyle());
    connect(verifyCodeBtn, &QPushButton::clicked, this, &RegWidget::onVerifyCodeClicked);
    s3Btns->addWidget(verifyCodeBtn);

    s3->addLayout(s3Btns);
    mainLayout->addWidget(step3Widget);

    QFrame *line = new QFrame(card);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("QFrame { background: %1; border: none; max-height: 1px; }").arg(GH_BORDER));
    mainLayout->addWidget(line);

    showAuthBtn = new QPushButton(QString::fromUtf8("Уже есть аккаунт? Войти"), card);
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

/**
 * @brief Показать указанный шаг регистрации
 * 
 * Делает видимым только виджет указанного шага,
 * скрывая остальные.
 * 
 * @param step Номер шага (1, 2 или 3)
 */
void RegWidget::showStep(int step)
{
    step1Widget->setVisible(step == 1);
    step2Widget->setVisible(step == 2);
    step3Widget->setVisible(step == 3);
}

/**
 * @brief Проверить корректность формата email
 * 
 * Использует регулярное выражение для проверки формата
 * адреса электронной почты.
 * 
 * @param email Адрес электронной почты для проверки
 * @return true если email соответствует формату, false в противном случае
 */
bool RegWidget::isEmailValid(const QString &email) const
{
    QRegularExpression re("^[a-zA-Z0-9._%+\\-]+@[a-zA-Z0-9.\\-]+\\.[a-zA-Z]{2,}$");
    return re.match(email).hasMatch();
}

/**
 * @brief Валидация полей шага 1
 * 
 * Проверяет минимальную длину логина (4 символа), пароля (8 символов),
 * заполненность поля подтверждения и совпадение паролей.
 * Активирует/деактивирует кнопку "Продолжить".
 */
void RegWidget::validateStep1()
{
    bool ok = loginEdit->text().length() >= 4
           && passwordEdit->text().length() >= 8
           && !confirmPasswordEdit->text().isEmpty()
           && confirmPasswordEdit->text() == passwordEdit->text();
    continueBtn->setEnabled(ok);
}

/**
 * @brief Обновить состояние кнопки "Подтвердить"
 * 
 * Кнопка активна только если OTP-код введён полностью,
 * блокировка не действует и хэш кода получен от сервера.
 */
void RegWidget::updateVerifyCodeBtn()
{
    verifyCodeBtn->setEnabled(
        otpCode->isComplete() && !codeIsLocked && !m_pendingCodeHash.isEmpty());
}

/**
 * @brief Слот для обработки изменения текста в поле логина
 * 
 * Проверяет минимальную длину логина и отображает ошибку
 * при необходимости. Запускает валидацию шага 1.
 * 
 * @param text Новый текст логина
 */
void RegWidget::onLoginTextChanged(const QString &text)
{
    if (!text.isEmpty() && text.length() < 4) {
        loginErrorLabel->setText(QString::fromUtf8("Минимум 4 символа"));
        loginErrorLabel->show();
    } else {
        loginErrorLabel->hide();
    }
    validateStep1();
}

/**
 * @brief Слот для обработки изменения текста в поле пароля
 * 
 * Проверяет минимальную длину пароля и совпадение с подтверждением.
 * Отображает соответствующие ошибки.
 * 
 * @param text Новый текст пароля
 */
void RegWidget::onPasswordTextChanged(const QString &text)
{
    if (!text.isEmpty() && text.length() < 8) {
        passwordErrorLabel->setText(QString::fromUtf8("Минимум 8 символов"));
        passwordErrorLabel->show();
    } else {
        passwordErrorLabel->hide();
    }
    if (!confirmPasswordEdit->text().isEmpty()) {
        if (confirmPasswordEdit->text() != text) {
            confirmErrorLabel->setText(QString::fromUtf8("Пароли не совпадают"));
            confirmErrorLabel->show();
        } else confirmErrorLabel->hide();
    }
    validateStep1();
}

/**
 * @brief Слот для обработки изменения текста в поле подтверждения пароля
 * 
 * Проверяет совпадение с введённым паролем и отображает ошибку
 * при несовпадении.
 * 
 * @param text Новый текст подтверждения пароля
 */
void RegWidget::onConfirmPasswordTextChanged(const QString &text)
{
    if (!text.isEmpty() && text != passwordEdit->text()) {
        confirmErrorLabel->setText(QString::fromUtf8("Пароли не совпадают"));
        confirmErrorLabel->show();
    } else {
        confirmErrorLabel->hide();
    }
    validateStep1();
}

/**
 * @brief Слот для переключения видимости первого пароля
 * 
 * Переключает режим отображения поля пароля между
 * скрытым (Password) и видимым (Normal).
 */
void RegWidget::onTogglePassword1()
{
    passwordEdit->setEchoMode(passwordEdit->echoMode() == QLineEdit::Password
                                  ? QLineEdit::Normal : QLineEdit::Password);
}

/**
 * @brief Слот для переключения видимости второго пароля
 * 
 * Переключает режим отображения поля подтверждения пароля между
 * скрытым (Password) и видимым (Normal).
 */
void RegWidget::onTogglePassword2()
{
    confirmPasswordEdit->setEchoMode(confirmPasswordEdit->echoMode() == QLineEdit::Password
                                         ? QLineEdit::Normal : QLineEdit::Password);
}

/**
 * @brief Слот для обработки нажатия кнопки "Продолжить"
 * 
 * Отправляет асинхронный запрос на проверку доступности логина
 * на сервер. Блокирует кнопку на время проверки.
 */
void RegWidget::onContinueClicked()
{
    if (!continueBtn->isEnabled()) return;
    continueBtn->setEnabled(false);
    continueBtn->setText(QString::fromUtf8("Проверяем..."));
    loginErrorLabel->hide();
    m_checkingLogin = true;
    ClientSingleton::instance().sendRequestAsync(
        QString("check_login||%1").arg(loginEdit->text().trimmed()));
}

/**
 * @brief Слот для обработки изменения текста в поле email
 * 
 * Проверяет формат email и отображает ошибку при неверном формате.
 * Активирует/деактивирует кнопку "Далее →".
 * 
 * @param text Новый текст email
 */
void RegWidget::onEmailTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        emailErrorLabel->hide();
        emailNextBtn->setEnabled(false);
        return;
    }
    if (!isEmailValid(text)) {
        emailErrorLabel->setText(QString::fromUtf8("Неверный формат почты"));
        emailErrorLabel->show();
        emailNextBtn->setEnabled(false);
    } else {
        emailErrorLabel->hide();
        emailNextBtn->setEnabled(true);
    }
}

/**
 * @brief Слот для обработки нажатия кнопки "Далее →"
 * 
 * Хэширует пароль (SHA-256) и отправляет асинхронный запрос
 * регистрации на сервер с логином, хэшем пароля и email.
 * Переходит к ожиданию OTP-кода.
 */
void RegWidget::onEmailNextClicked()
{
    if (!emailNextBtn->isEnabled()) return;
    currentEmail = emailEdit->text().trimmed();

    QString login    = loginEdit->text().trimmed();
    QString password = passwordEdit->text();
    QByteArray hash  = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    m_pendingPassHash = QString::fromLatin1(hash.toHex());

    emailNextBtn->setEnabled(false);
    emailNextBtn->setText(QString::fromUtf8("Отправка..."));
    emailErrorLabel->hide();
    m_waitingForRegCode = true;
    m_verifyingCode = false;

    ClientSingleton::instance().sendRequestAsync(
        QString("registration||%1||%2||%3").arg(login, m_pendingPassHash, currentEmail));
}

/**
 * @brief Слот для возврата к шагу 1 из шага 2
 * 
 * Сбрасывает состояние полей шага 1, включает их для редактирования
 * и переключает видимость на шаг 1.
 */
void RegWidget::onBackToStep1Clicked()
{
    loginEdit->setReadOnly(false);
    passwordEdit->setReadOnly(false);
    confirmPasswordEdit->setReadOnly(false);
    continueBtn->setText(QString::fromUtf8("Продолжить"));
    validateStep1();
    emailNextBtn->setEnabled(false);
    emailNextBtn->setText(QString::fromUtf8("Далее →"));
    showStep(1);
}

/**
 * @brief Слот для возврата к шагу 2 из шага 3
 * 
 * Очищает OTP-ввод, скрывает статусные метки и сбрасывает флаги
 * состояния верификации кода.
 */
void RegWidget::onBackToStep2Clicked()
{
    otpCode->clear();
    codeErrorLabel->hide();
    codeStatusLabel->hide();
    emailHintLabel->hide();
    verifyCodeBtn->setEnabled(false);
    emailNextBtn->setEnabled(isEmailValid(emailEdit->text()));
    emailNextBtn->setText(QString::fromUtf8("Далее →"));
    m_verifyingCode = false;
    m_waitingForRegCode = false;
    showStep(2);
}

/**
 * @brief Слот для обработки завершения ввода OTP-кода
 * 
 * Обновляет состояние кнопки "Подтвердить" после ввода кода.
 * 
 * @param code Введённый код (не используется)
 */
void RegWidget::onCodeCompleted(const QString &)
{
    updateVerifyCodeBtn();
}

/**
 * @brief Слот для обработки нажатия кнопки "Подтвердить"
 * 
 * Проверяет введённый OTP-код: вычисляет его хэш и сравнивает
 * с полученным от сервера. При совпадении отправляет запрос
 * подтверждения регистрации. При несовпадении увеличивает
 * счётчик попыток с блокировкой при превышении лимита.
 */
void RegWidget::onVerifyCodeClicked()
{
    if (codeIsLocked || !verifyCodeBtn->isEnabled()) return;
    verifyCodeBtn->setEnabled(false);

    const QString code = otpCode->code();
    if (code.length() != 6) {
        codeErrorLabel->setText(QString::fromUtf8("Введите 6-значный код."));
        codeErrorLabel->show();
        verifyCodeBtn->setEnabled(true);
        return;
    }

    const QString enteredHash = QString::fromLatin1(
        QCryptographicHash::hash(code.toUtf8(), QCryptographicHash::Sha256).toHex());

    if (enteredHash == m_pendingCodeHash) {
        codeErrorLabel->hide();
        codeStatusLabel->setText(QString::fromUtf8("Подтверждение..."));
        codeStatusLabel->setStyleSheet(infoLabelStyle());
        codeStatusLabel->show();
        m_verifyingCode = true;
        ClientSingleton::instance().sendRequestAsync(
            QString("registration_confirm||%1").arg(loginEdit->text().trimmed()));
        return;
    }

    verifyCodeBtn->setEnabled(true);
    codeFailedAttempts++;
    if (codeFailedAttempts < 4) {
        codeStatusLabel->hide();
        codeErrorLabel->setText(
            QString::fromUtf8("Неверный код. Осталось попыток: %1.").arg(4 - codeFailedAttempts));
        codeErrorLabel->show();
        otpCode->clear();
    } else {
        codeLockLevel++;
        codeFailedAttempts = 0;
        const int lockMin = (codeLockLevel == 1) ? 0 : (codeLockLevel == 2) ? 5 : 30;
        applyCodeLock(lockMin,
            lockMin == 0
                ? QString::fromUtf8("Превышен лимит. Блокировка на 30 сек.")
                : QString::fromUtf8("Превышен лимит. Блокировка на %1 мин.").arg(lockMin));
        verifyCodeBtn->setEnabled(false);
    }
}

/**
 * @brief Слот для обработки срабатывания таймера блокировки ввода кода
 * 
 * Снимает блокировку OTP-ввода, очищает поле кода
 * и обновляет состояние кнопки "Подтвердить".
 */
void RegWidget::onCodeLockTimerFired()
{
    codeIsLocked = false;
    otpCode->setEnabled(true);
    otpCode->clear();
    updateVerifyCodeBtn();
    codeStatusLabel->hide();
    codeErrorLabel->hide();
}

/**
 * @brief Слот для обработки нажатия ссылки "Уже есть аккаунт? Войти"
 * 
 * Испускает сигнал showAuth для переключения на экран авторизации.
 */
void RegWidget::onShowAuthClicked() { emit showAuth(); }

/**
 * @brief Применить блокировку ввода OTP-кода
 * 
 * Блокирует OTP-ввод и кнопку "Подтвердить", отображает сообщение
 * о блокировке и запускает таймер.
 * 
 * @param minutes Длительность блокировки в минутах (0 = 30 секунд)
 * @param message Сообщение для отображения пользователю
 */
void RegWidget::applyCodeLock(int minutes, const QString &message)
{
    codeIsLocked = true;
    otpCode->setEnabled(false);
    verifyCodeBtn->setEnabled(false);
    codeErrorLabel->setText(message);
    codeErrorLabel->show();
    codeStatusLabel->hide();
    codeLockTimer->start(minutes == 0 ? 30 * 1000 : minutes * 60 * 1000);
}

/**
 * @brief Слот для обработки ответа сервера на запросы регистрации
 * 
 * Обрабатывает три сценария:
 * 1. Проверка логина (m_checkingLogin): переход к шагу 2 или отображение ошибки
 * 2. Ожидание OTP-кода (m_waitingForRegCode): переход к шагу 3 или отображение ошибки
 * 3. Подтверждение кода (m_verifyingCode): успешная регистрация или ошибка
 * 
 * @param response Ответ сервера
 */
void RegWidget::onRegistrationResponseReceived(const QString &response)
{
    QString r = response.trimmed();
    if (r.isEmpty()) return;

    if (m_checkingLogin) {
        m_checkingLogin = false;
        continueBtn->setText(QString::fromUtf8("Продолжить"));
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
            loginErrorLabel->setText(QString::fromUtf8("Логин уже занят"));
            loginErrorLabel->show();
            validateStep1();
        } else {
            loginErrorLabel->setText(QString::fromUtf8("Ошибка соединения с сервером"));
            loginErrorLabel->show();
            validateStep1();
        }
        return;
    }

    if (m_waitingForRegCode) {
        m_waitingForRegCode = false;
        emailNextBtn->setText(QString::fromUtf8("Далее →"));

        if (r.startsWith("reg_code_sent")) {
            const QStringList parts = r.split("||");
            m_pendingCodeHash = (parts.size() >= 2) ? parts[1].trimmed() : QString();

            emailHintLabel->setText(
                QString::fromUtf8("Код отправлен на: ") + currentEmail);
            emailHintLabel->show();
            otpCode->clear();
            otpCode->setEnabled(true);
            codeErrorLabel->hide();
            codeStatusLabel->hide();
            verifyCodeBtn->setEnabled(false);
            showStep(3);
            return;
        }

        emailNextBtn->setEnabled(isEmailValid(emailEdit->text()));

        if (r == "email_taken" || r.contains("email_exists") || r.contains("email_taken")) {
            emailErrorLabel->setText(QString::fromUtf8("На эту почту уже зарегистрирован аккаунт"));
            emailErrorLabel->show();
        } else if (r.startsWith("reg-") && r.contains("user_exists")) {
            showStep(1);
            loginErrorLabel->setText(QString::fromUtf8("Логин уже занят."));
            loginErrorLabel->show();
        } else {
            emailErrorLabel->setText(QString::fromUtf8("Ошибка сервера. Попробуйте снова."));
            emailErrorLabel->show();
        }
        return;
    }

    if (m_verifyingCode) {
        m_verifyingCode = false;

        if (r.startsWith("reg+")) {
            codeStatusLabel->setText(QString::fromUtf8("Регистрация успешна!"));
            codeStatusLabel->setStyleSheet(successLabelStyle());
            codeStatusLabel->show();
            codeErrorLabel->hide();
            verifyCodeBtn->setEnabled(false);
            QTimer::singleShot(800, this, [this]() { emit registrationSuccess(currentLogin); });
            return;
        }

        codeErrorLabel->setText(QString::fromUtf8("Ошибка. Попробуйте снова."));
        codeErrorLabel->show();
        verifyCodeBtn->setEnabled(true);
    }
}
