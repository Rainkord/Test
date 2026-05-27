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

/**
 * @brief Генерирует стиль CSS для полей ввода
 * @return строка со стилем CSS
 */
static QString inputStyleR()
{
    return QString(
        "QLineEdit { background: %1; color: %2; border: 1px solid %3;"
        " border-radius: 6px; padding: 6px 10px;"
        " font-family: '%4'; font-size: %5pt; }"
        "QLineEdit:focus { border-color: %6; }"
    ).arg(GH_INPUT_BG, GH_TEXT, GH_BORDER, FONT_FAMILY).arg(FS_INPUT).arg(GH_BLUE);
}

/**
 * @brief Генерирует стиль CSS для основной зелёной кнопки
 * @return строка со стилем CSS
 */
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

/**
 * @brief Генерирует стиль CSS для второстепенной кнопки
 * @return строка со стилем CSS
 */
static QString secondaryR()
{
    return QString(
        "QPushButton { background: %1; color: %3;"
        " border: 1px solid %4; border-radius: 6px;"
        " padding: 6px 16px; font-family: '%5'; font-size: %6pt; font-weight: bold; }"
        "QPushButton:hover { background: %2; }"
    ).arg(GH_BTN_GHOST, GH_BTN_GHOST_H, GH_TEXT, GH_BORDER, FONT_FAMILY).arg(FS_BTN);
}

/**
 * @brief Генерирует стиль CSS для прозрачной кнопки-призрака
 * @return строка со стилем CSS
 */
static QString ghostR()
{
    return QString(
        "QPushButton { background: %1; color: %3;"
        " border: 1px solid %4; border-radius: 6px;"
        " padding: 5px 14px; font-family: '%5'; font-size: %6pt; }"
        "QPushButton:hover { background: %2; }"
    ).arg(GH_BTN_GHOST, GH_BTN_GHOST_H, GH_TEXT, GH_BORDER, FONT_FAMILY).arg(FS_BTN);
}

/**
 * @brief Генерирует стиль CSS для кнопки-ссылки
 * @return строка со стилем CSS
 */
static QString linkR()
{
    return QString(
        "QPushButton { color: %1; border: none; background: transparent;"
        " font-family: '%2'; font-size: %3pt; }"
        "QPushButton:hover { color: %4; text-decoration: underline; }"
    ).arg(GH_BLUE, FONT_FAMILY).arg(FS_BTN).arg(GH_BLUE_H);
}

/**
 * @brief Генерирует стиль CSS для метки ошибки
 * @return строка со стилем CSS
 */
static QString errorR()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_RED, FONT_FAMILY).arg(FS_SMALL);
}

/**
 * @brief Генерирует стиль CSS для информационной метки
 * @return строка со стилем CSS
 */
static QString infoR()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_MUTED, FONT_FAMILY).arg(FS_SMALL);
}

/**
 * @brief Генерирует стиль CSS для метки успешного результата
 * @return строка со стилем CSS
 */
static QString successR()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_GREEN_H, FONT_FAMILY).arg(FS_SMALL);
}

// ── Constructor ──────────────────────────────────────────────────────────────────
/**
 * @brief Конструктор виджета сброса пароля
 *
 * Инициализирует таймер блокировки, подключает сигнал ответа сервера,
 * устанавливает стили и создаёт пользовательский интерфейс.
 *
 * @param parent родительский виджет
 */
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

/// Деструктор виджета сброса пароля
ResetWidget::~ResetWidget() {}

// ── clearFields ───────────────────────────────────────────────────────────────
// Полный сброс состояния виджета. Вызывается из MainWindow перед каждым
// открытием экрана сброса пароля и при любом возврате на экран авторизации.
// Это предотвращает ситуацию, когда зависший флаг m_waitingForCodeHash
// или m_waitingForSave заставляет onResetResponseReceived обрабатывать
// следующий ответ сервера (например, от auth) как ответ на сброс пароля.
/**
 * @brief Полный сброс состояния виджета
 *
 * Сбрасывает флаги ожидания, останавливает таймер блокировки,
 * очищает все поля и возвращает виджет на шаг ввода email.
 * Вызывается перед каждым открытием экрана сброса пароля.
 */
void ResetWidget::clearFields()
{
    // Сброс флагов ожидания — ПЕРВЫМ ДЕЛОМ, до любых UI-изменений
    m_waitingForCodeHash = false;
    m_waitingForSave     = false;

    // Остановить таймер блокировки если запущен
    if (lockTimer->isActive())
        lockTimer->stop();

    // Сброс состояния блокировки
    isLocked       = false;
    failedAttempts = 0;
    lockLevel      = 0;

    // Сброс данных
    m_email.clear();
    m_pendingCodeHash.clear();

    // Шаг 1: email
    emailEdit->clear();
    emailErrorLabel->hide();
    continueBtn->setEnabled(false);
    continueBtn->setText(QString::fromUtf8("Отправить код"));

    // Шаг 2: OTP
    codeEdit->clear();
    codeEdit->setEnabled(true);
    codeErrorLabel->hide();
    codeStatusLabel->hide();
    verifyCodeBtn->setEnabled(false);

    // Шаг 3: новый пароль
    newPasswordEdit->clear();
    confirmPasswordEdit->clear();
    newPasswordErrorLabel->hide();
    confirmErrorLabel->hide();
    saveBtn->setEnabled(false);
    saveBtn->setText(QString::fromUtf8("Сохранить пароль"));

    // Вернуться на шаг 1
    showStep(StepEmail);
}

/**
 * @brief Инициализация пользовательского интерфейса
 *
 * Создаёт три шага: ввод email, OTP-код и новый пароль.
 * Настраивает все сигналы и слоты, компонует элементы управления.
 */
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

    auto *titleLbl = new QLabel(QString::fromUtf8("Восстановление пароля"), card);
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

    // ── Шаг 1: email ────────────────────────────────────────────────────────
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

    backBtn = new QPushButton(QString::fromUtf8("\u2190 Назад"), step1Widget);
    backBtn->setMinimumHeight(38);
    backBtn->setStyleSheet(secondaryR());
    connect(backBtn, &QPushButton::clicked, this, &ResetWidget::onBackClicked);
    s1Btns->addWidget(backBtn);

    continueBtn = new QPushButton(QString::fromUtf8("Отправить код"), step1Widget);
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

    // ── Шаг 2: OTP-ввод кода ─────────────────────────────────────────────────────
    step2Widget = new QWidget(card);
    step2Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    auto *s2 = new QVBoxLayout(step2Widget);
    s2->setContentsMargins(0, 0, 0, 0);
    s2->setSpacing(8);

    auto *codeHintLbl = new QLabel(
        QString::fromUtf8("Введите код из письма:"),
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

    verifyCodeBtn = new QPushButton(QString::fromUtf8("Подтвердить"), step2Widget);
    verifyCodeBtn->setMinimumHeight(38);
    verifyCodeBtn->setEnabled(false);
    verifyCodeBtn->setDefault(true);
    verifyCodeBtn->setAutoDefault(true);
    verifyCodeBtn->setStyleSheet(primaryR());
    connect(verifyCodeBtn, &QPushButton::clicked, this, &ResetWidget::onVerifyCodeClicked);
    s2->addWidget(verifyCodeBtn);

    mainL->addWidget(step2Widget);

    // ── Шаг 3: новый пароль ──────────────────────────────────────────────
    step3Widget = new QWidget(card);
    step3Widget->setStyleSheet("QWidget { background: transparent; border: none; }");
    auto *s3 = new QVBoxLayout(step3Widget);
    s3->setContentsMargins(0, 0, 0, 0);
    s3->setSpacing(6);

    auto *newPassHintLbl = new QLabel(
        QString::fromUtf8("Введите новый пароль:"),
        step3Widget);
    newPassHintLbl->setAlignment(Qt::AlignCenter);
    newPassHintLbl->setStyleSheet(infoR());
    s3->addWidget(newPassHintLbl);

    auto *np1Row = new QHBoxLayout();
    np1Row->setSpacing(6);
    newPasswordEdit = new QLineEdit(step3Widget);
    newPasswordEdit->setPlaceholderText(QString::fromUtf8("Новый пароль (мин. 8 симв.)"));
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
    confirmPasswordEdit->setPlaceholderText(QString::fromUtf8("Подтвердите пароль"));
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

    saveBtn = new QPushButton(QString::fromUtf8("Сохранить пароль"), step3Widget);
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

/**
 * @brief Переключает отображаемый шаг процесса сброса пароля
 * @param step номер шага для отображения
 */
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

/**
 * @brief Обработчик нажатия клавиш
 *
 * При нажатии Escape: на шаге кода — возврат к email,
 * на шаге пароля — возврат к коду, на шаге email — выход на авторизацию.
 *
 * @param e событие нажатия клавиши
 */
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

/**
 * @brief Проверяет корректность формата email
 * @param email адрес электронной почты для проверки
 * @return true если email соответствует регулярному выражению
 */
bool ResetWidget::isEmailValid(const QString &email) const
{
    QRegularExpression re("^[a-zA-Z0-9._%+\\-]+@[a-zA-Z0-9.\\-]+\\.[a-zA-Z]{2,}$");
    return re.match(email).hasMatch();
}

/// Проверяет совпадение и корректность паролей, обновляет состояние кнопки «Сохранить»
void ResetWidget::validatePasswords()
{
    const QString np = newPasswordEdit->text();
    const QString cp = confirmPasswordEdit->text();
    bool ok = np.length() >= 8 && !cp.isEmpty() && np == cp;
    saveBtn->setEnabled(ok);
}

/**
 * @brief Обработчик изменения текста в поле email
 *
 * Проверяет формат email и показывает ошибку при неверном формате.
 * Активирует кнопку «Отправить код» при корректном email.
 *
 * @param text текущее содержимое поля email
 */
void ResetWidget::onEmailTextChanged(const QString &text)
{
    if (text.isEmpty()) { emailErrorLabel->hide(); continueBtn->setEnabled(false); return; }
    if (!isEmailValid(text)) {
        emailErrorLabel->setText(QString::fromUtf8("Неверный формат"));
        emailErrorLabel->show();
        continueBtn->setEnabled(false);
    } else {
        emailErrorLabel->hide();
        continueBtn->setEnabled(true);
    }
}

/// Обработчик нажатия кнопки «Отправить код»: отправляет запрос на сброс пароля серверу
void ResetWidget::onContinueClicked()
{
    if (!continueBtn->isEnabled()) return;
    m_email = emailEdit->text().trimmed();
    continueBtn->setEnabled(false);
    continueBtn->setText(QString::fromUtf8("Отправка..."));
    emailErrorLabel->hide();
    m_waitingForCodeHash = true;
    ClientSingleton::instance().sendRequestAsync(
        QString("reset_password||%1").arg(m_email));
}

/**
 * @brief Обработчик изменения OTP-кода
 *
 * Активирует кнопку «Подтвердить» при заполнении всех полей.
 *
 * @param text текущее содержимое OTP-ввода (не используется)
 */
void ResetWidget::onCodeTextChanged(const QString &)
{
    verifyCodeBtn->setEnabled(!isLocked && codeEdit->isComplete() && !m_pendingCodeHash.isEmpty());
}

/**
 * @brief Обработчик нажатия кнопки «Подтвердить код»
 *
 * Хэширует введённый код и сравнивает с полученным от сервера.
 * При совпадении переход к шагу установки нового пароля.
 * При несовпадении увеличивает счётчик попыток с блокировкой.
 */
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
        codeStatusLabel->setText(QString::fromUtf8("Код верен!"));
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
            QString::fromUtf8("Неверный код. Осталось: %1.").arg(4 - failedAttempts));
        codeErrorLabel->show();
        codeEdit->clear();
        verifyCodeBtn->setEnabled(false);
    } else {
        lockLevel++;
        failedAttempts = 0;
        const int lockMin = (lockLevel == 1) ? 0 : (lockLevel == 2) ? 5 : 30;
        applyLock(lockMin,
            lockMin == 0
                ? QString::fromUtf8("Превышен лимит. Блокировка 30 сек.")
                : QString::fromUtf8("Превышен лимит. Блокировка %1 мин.").arg(lockMin));
    }
}

/**
 * @brief Обработчик изменения поля «Новый пароль»
 *
 * Проверяет минимальную длину пароля (8 символов) и совпадение
 * с полем подтверждения.
 *
 * @param text текущее значение нового пароля
 */
void ResetWidget::onNewPasswordTextChanged(const QString &text)
{
    if (!text.isEmpty() && text.length() < 8) {
        newPasswordErrorLabel->setText(QString::fromUtf8("Минимум 8 симв."));
        newPasswordErrorLabel->show();
    } else {
        newPasswordErrorLabel->hide();
    }
    if (!confirmPasswordEdit->text().isEmpty() && confirmPasswordEdit->text() != text) {
        confirmErrorLabel->setText(QString::fromUtf8("Пароли не совпадают"));
        confirmErrorLabel->show();
    } else {
        confirmErrorLabel->hide();
    }
    validatePasswords();
}

/**
 * @brief Обработчик изменения поля «Подтвердите пароль»
 *
 * Проверяет совпадение с полем нового пароля.
 *
 * @param text текущее значение подтверждения пароля
 */
void ResetWidget::onConfirmPasswordTextChanged(const QString &text)
{
    if (!text.isEmpty() && text != newPasswordEdit->text()) {
        confirmErrorLabel->setText(QString::fromUtf8("Пароли не совпадают"));
        confirmErrorLabel->show();
    } else {
        confirmErrorLabel->hide();
    }
    validatePasswords();
}

/// Переключает видимость нового пароля между режимами Password и Normal
void ResetWidget::onTogglePassword1()
{
    newPasswordEdit->setEchoMode(newPasswordEdit->echoMode() == QLineEdit::Password
                                     ? QLineEdit::Normal : QLineEdit::Password);
}

/// Переключает видимость подтверждения пароля между режимами Password и Normal
void ResetWidget::onTogglePassword2()
{
    confirmPasswordEdit->setEchoMode(confirmPasswordEdit->echoMode() == QLineEdit::Password
                                         ? QLineEdit::Normal : QLineEdit::Password);
}

/// Обработчик нажатия кнопки «Сохранить пароль»: хэширует пароль и отправляет на сервер
void ResetWidget::onSavePasswordClicked()
{
    if (!saveBtn->isEnabled()) return;
    saveBtn->setEnabled(false);
    saveBtn->setText(QString::fromUtf8("Сохранение..."));

    const QString hash = QString::fromLatin1(
        QCryptographicHash::hash(
            newPasswordEdit->text().toUtf8(), QCryptographicHash::Sha256).toHex());

    m_waitingForSave = true;
    ClientSingleton::instance().sendRequestAsync(
        QString("set_new_password||%1||%2").arg(m_email, hash));
}

/// Обработчик нажатия кнопки «Назад»: сбрасывает флаги ожидания и испускает сигнал backToAuth
void ResetWidget::onBackClicked()
{
    // Сбрасываем флаги ожидания перед уходом — если сервер ещё не ответил,
    // его ответ не должен быть обработан как ответ на сброс пароля
    m_waitingForCodeHash = false;
    m_waitingForSave     = false;
    continueBtn->setEnabled(isEmailValid(emailEdit->text()));
    continueBtn->setText(QString::fromUtf8("Отправить код"));
    emit backToAuth();
}

/// Обработчик срабатывания таймера блокировки: снимает блокировку и очищает OTP-ввод
void ResetWidget::onLockTimerFired()
{
    isLocked = false;
    codeEdit->setEnabled(true);
    codeEdit->clear();
    verifyCodeBtn->setEnabled(false);
    codeErrorLabel->hide();
}

/**
 * @brief Блокирует ввод OTP-кода на указанное время
 * @param minutes количество минут блокировки (0 = 30 секунд)
 * @param message сообщение при блокировке
 */
void ResetWidget::applyLock(int minutes, const QString &message)
{
    isLocked = true;
    codeEdit->setEnabled(false);
    verifyCodeBtn->setEnabled(false);
    codeErrorLabel->setText(message);
    codeErrorLabel->show();
    lockTimer->start(minutes == 0 ? 30 * 1000 : minutes * 60 * 1000);
}

/**
 * @brief Обработчик ответа сервера на запросы сброса пароля
 *
 * Обрабатывает два типа ответов: получение хэша кода (m_waitingForCodeHash)
 * и результат сохранения нового пароля (m_waitingForSave).
 *
 * @param response строка ответа от сервера
 */
void ResetWidget::onResetResponseReceived(const QString &response)
{
    QString r = response.trimmed();
    if (r.isEmpty()) return;

    if (m_waitingForCodeHash) {
        m_waitingForCodeHash = false;
        continueBtn->setText(QString::fromUtf8("Отправить код"));
        continueBtn->setEnabled(isEmailValid(emailEdit->text()));

        if (r.startsWith("reset_code_sent")) {
            const QStringList parts = r.split("||");
            m_pendingCodeHash = (parts.size() >= 2) ? parts[1].trimmed() : QString();
            failedAttempts = 0;
            isLocked = false;
            showStep(StepCode);
        } else if (r == "reset-") {
            emailErrorLabel->setText(QString::fromUtf8("Пользователь с таким email не найден"));
            emailErrorLabel->show();
        } else {
            emailErrorLabel->setText(QString::fromUtf8("Ошибка сервера. Попробуйте снова."));
            emailErrorLabel->show();
        }
        return;
    }

    if (m_waitingForSave) {
        m_waitingForSave = false;
        if (r == "set_password+") {
            emit resetSuccess();
        } else {
            saveBtn->setEnabled(true);
            saveBtn->setText(QString::fromUtf8("Сохранить пароль"));
            confirmErrorLabel->setText(QString::fromUtf8("Ошибка сервера. Попробуйте снова."));
            confirmErrorLabel->show();
        }
    }
}
