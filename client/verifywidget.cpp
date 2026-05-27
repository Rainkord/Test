#include "verifywidget.h"
#include "otpinput.h"
#include "clientsingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QCryptographicHash>
#include <QFont>
#include <QFrame>

#define GH_BG       "#0d1117"
#define GH_CARD     "#161b22"
#define GH_BORDER   "#30363d"
#define GH_TEXT     "#e6edf3"
#define GH_MUTED    "#8b949e"
#define GH_GREEN    "#238636"
#define GH_GREEN_H  "#2ea043"
#define GH_BLUE     "#388bfd"
#define GH_BLUE_H   "#58a6ff"
#define GH_RED      "#f85149"
#define FONT_FAMILY "Segoe UI"
#define FS_TITLE    16
#define FS_BTN      11
#define FS_SMALL     9

/**
 * @brief Генерирует стиль CSS для основной кнопки
 * @param enabled состояние кнопки (включена/выключена)
 * @return строка со стилем CSS
 */
static QString primaryBtnStyle(bool enabled)
{
    if (enabled)
        return QString(
            "QPushButton {"
            "  background-color: %1; color: #fff;"
            "  border: 1px solid rgba(240,246,252,.1); border-radius: 6px;"
            "  padding: 6px 16px; font-family: '%3'; font-size: %4pt; font-weight: bold;"
            "}"
            "QPushButton:hover { background-color: %2; }"
        ).arg(GH_GREEN, GH_GREEN_H, FONT_FAMILY).arg(FS_BTN);
    return QString(
        "QPushButton {"
        "  background-color: rgba(35,134,54,.35); color: rgba(255,255,255,.4);"
        "  border: 1px solid rgba(240,246,252,.05); border-radius: 6px;"
        "  padding: 6px 16px; font-family: '%1'; font-size: %2pt; font-weight: bold;"
        "}"
    ).arg(FONT_FAMILY).arg(FS_BTN);
}

/**
 * @brief Генерирует стиль CSS для кнопки-ссылки
 * @return строка со стилем CSS
 */
static QString linkBtnStyle()
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
static QString errorStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_RED, FONT_FAMILY).arg(FS_SMALL);
}

/**
 * @brief Генерирует стиль CSS для метки-подсказки
 * @return строка со стилем CSS
 */
static QString hintStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_MUTED, FONT_FAMILY).arg(FS_SMALL);
}

/**
 * @brief Генерирует стиль CSS для метки успешного результата
 * @return строка со стилем CSS
 */
static QString successStyle()
{
    return QString("QLabel { color: %1; border: none; font-family: '%2'; font-size: %3pt; }")
           .arg(GH_GREEN_H, FONT_FAMILY).arg(FS_SMALL);
}

// ── Constructor ──────────────────────────────────────────────────────────────
/**
 * @brief Конструктор виджета верификации
 *
 * Устанавливает стили, создаёт интерфейс и таймер блокировки.
 *
 * @param parent родительский виджет
 */
VerifyWidget::VerifyWidget(QWidget *parent)
    : QWidget(parent), isLocked(false), m_attemptsLeft(3)
{
    setStyleSheet(QString(
        "QWidget { background-color: %1; color: %2; font-family: '%3'; font-size: 11pt; }")
        .arg(GH_BG, GH_TEXT, FONT_FAMILY));

    setupUI();

    lockTimer = new QTimer(this);
    lockTimer->setSingleShot(true);
    connect(lockTimer, &QTimer::timeout, this, &VerifyWidget::onLockTimerFired);
}

/// Деструктор виджета верификации
VerifyWidget::~VerifyWidget() {}

/**
 * @brief Инициализация пользовательского интерфейса
 *
 * Создаёт карточку с полем OTP-ввода, кнопками «Подтвердить» и «Назад»,
 * а также метками статуса и подсказок.
 */
void VerifyWidget::setupUI()
{
    auto *outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->addStretch(1);

    auto *outerH = new QHBoxLayout();
    outerH->addStretch(1);

    QWidget *card = new QWidget(this);
    card->setFixedWidth(420);
    card->setStyleSheet(QString(
        "QWidget { background-color: %1; border: 1px solid %2; border-radius: 10px; }")
        .arg(GH_CARD, GH_BORDER));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(8);

    // Заголовок
    auto *title = new QLabel(QString::fromUtf8("Подтверждение входа"), card);
    QFont tf(FONT_FAMILY, FS_TITLE, QFont::Bold);
    title->setFont(tf);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QString("QLabel { color: %1; border: none; background: transparent; }").arg(GH_TEXT));
    layout->addWidget(title);

    auto *sep = new QFrame(card);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(QString("QFrame { background: %1; border: none; max-height: 1px; }").arg(GH_BORDER));
    layout->addWidget(sep);
    layout->addSpacing(4);

    hintLabel = new QLabel(QString::fromUtf8("Введите код из письма:"), card);
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setWordWrap(true);
    hintLabel->setStyleSheet(hintStyle());
    layout->addWidget(hintLabel);
    layout->addSpacing(4);

    // OTP-ввод
    otpInput = new OtpInput(card);
    layout->addWidget(otpInput, 0, Qt::AlignCenter);
    // FIX: только активируем кнопку при заполнении, НЕ вызываем verify автоматически
    connect(otpInput, &OtpInput::completed,  this, &VerifyWidget::onCodeCompleted);
    // ESC из OtpInput → назад
    connect(otpInput, &OtpInput::escPressed, this, &VerifyWidget::onBackClicked);

    statusLabel = new QLabel(card);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    statusLabel->setStyleSheet(errorStyle());
    statusLabel->hide();
    layout->addWidget(statusLabel);
    layout->addSpacing(4);

    verifyBtn = new QPushButton(QString::fromUtf8("Подтвердить"), card);
    verifyBtn->setEnabled(false);
    verifyBtn->setDefault(true);
    verifyBtn->setAutoDefault(true);
    verifyBtn->setMinimumHeight(38);
    verifyBtn->setStyleSheet(primaryBtnStyle(false));
    connect(verifyBtn, &QPushButton::clicked, this, &VerifyWidget::onVerifyClicked);
    layout->addWidget(verifyBtn);

    backBtn = new QPushButton(QString::fromUtf8("Назад"), card);
    backBtn->setStyleSheet(linkBtnStyle());
    connect(backBtn, &QPushButton::clicked, this, &VerifyWidget::onBackClicked);
    layout->addWidget(backBtn, 0, Qt::AlignCenter);

    outerH->addWidget(card);
    outerH->addStretch(1);
    outerV->addLayout(outerH);
    outerV->addStretch(1);
}

/**
 * @brief Устанавливает логин и хэш кода для верификации
 *
 * Сбрасывает состояние: очищает OTP-ввод, восстанавливает лимит попыток
 * и скрывает сообщения об ошибках.
 *
 * @param login логин пользователя
 * @param codeHash SHA-256 хэш ожидаемого OTP-кода
 */
void VerifyWidget::setLogin(const QString &login, const QString &codeHash)
{
    m_login        = login;
    m_codeHash     = codeHash;
    m_attemptsLeft = 3;
    isLocked       = false;
    otpInput->clear();
    otpInput->setEnabled(true);
    updateVerifyBtn();
    statusLabel->hide();
    hintLabel->setText(QString::fromUtf8("Введите код из письма:"));
    hintLabel->setStyleSheet(hintStyle());
}

/// Полностью очищает все поля и сбрасывает внутреннее состояние виджета
void VerifyWidget::clearFields()
{
    otpInput->clear();
    otpInput->setError(false);
    statusLabel->hide();
    updateVerifyBtn();
    m_codeHash.clear();
    m_login.clear();
    m_attemptsLeft = 3;
    isLocked       = false;
}

/// Обновляет состояние и стиль кнопки «Подтвердить» в зависимости от заполненности OTP-кода
void VerifyWidget::updateVerifyBtn()
{
    const bool ok = !isLocked && otpInput->isComplete();
    verifyBtn->setEnabled(ok);
    verifyBtn->setStyleSheet(primaryBtnStyle(ok));
}

// FIX: убран автоматический вызов onVerifyClicked() при заполнении кода.
// Это устраняло бесконечную петлю: неверный код → 600мс пауза → clear() →
// completed() снова → onVerifyClicked() снова → и так до блокировки.
// Теперь пользователь должен явно нажать кнопку «Подтвердить».
/**
 * @brief Обработчик завершения ввода OTP-кода
 *
 * Сбрасывает флаг ошибки и обновляет состояние кнопки подтверждения.
 * Автоматическая верификация не вызывается — пользователь должен
 * явно нажать кнопку.
 *
 * @param code введённый шестизначный код (не используется)
 */
void VerifyWidget::onCodeCompleted(const QString &)
{
    otpInput->setError(false);
    updateVerifyBtn();
}

/**
 * @brief Обработчик нажатия кнопки «Подтвердить»
 *
 * Хэширует введённый код и сравнивает с ожидаемым. При совпадении
 * испускает сигнал verificationSuccess. При несовпадении уменьшает
 * счётчик попыток и при достижении нуля блокирует ввод на 30 секунд.
 */
void VerifyWidget::onVerifyClicked()
{
    if (isLocked) return;
    const QString c = otpInput->code();
    if (c.length() != 6) return;

    verifyBtn->setEnabled(false);
    verifyBtn->setStyleSheet(primaryBtnStyle(false));

    const QString entered = QString::fromLatin1(
        QCryptographicHash::hash(c.toUtf8(), QCryptographicHash::Sha256).toHex());

    if (entered == m_codeHash) {
        otpInput->setError(false);
        statusLabel->setStyleSheet(successStyle());
        statusLabel->setText(QString::fromUtf8("Код верен!"));
        statusLabel->show();
        otpInput->setEnabled(false);
        emit verificationSuccess(m_login);
        return;
    }

    // Неверный код
    otpInput->setError(true);
    m_attemptsLeft--;
    if (m_attemptsLeft > 0) {
        statusLabel->setStyleSheet(errorStyle());
        statusLabel->setText(
            QString::fromUtf8("Неверный код. Осталось попыток: %1.").arg(m_attemptsLeft));
        statusLabel->show();
        QTimer::singleShot(600, this, [this]() {
            otpInput->clear();
            updateVerifyBtn();
        });
    } else {
        applyLock(30, QString::fromUtf8("Превышен лимит попыток. Блокировка на 30 сек."));
    }
}

/// Обработчик нажатия кнопки «Назад»: очищает поля и испускает сигнал backToAuth
void VerifyWidget::onBackClicked()
{
    clearFields();
    emit backToAuth();
}

/**
 * @brief Обработчик нажатия клавиш
 *
 * При нажатии Escape вызывает переход на экран авторизации.
 *
 * @param e событие нажатия клавиши
 */
void VerifyWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        onBackClicked();
    else
        QWidget::keyPressEvent(e);
}

/**
 * @brief Блокирует ввод на указанное время
 *
 * Отключает OTP-ввод и кнопку подтверждения, отображает сообщение
 * и запускает таймер блокировки.
 *
 * @param seconds количество секунд блокировки
 * @param message сообщение, отображаемое при блокировке
 */
void VerifyWidget::applyLock(int seconds, const QString &message)
{
    isLocked = true;
    otpInput->setEnabled(false);
    verifyBtn->setEnabled(false);
    verifyBtn->setStyleSheet(primaryBtnStyle(false));
    statusLabel->setStyleSheet(errorStyle());
    statusLabel->setText(message);
    statusLabel->show();
    lockTimer->start(seconds * 1000);
}

/// Обработчик срабатывания таймера блокировки: снимает блокировку и сбрасывает попытки
void VerifyWidget::onLockTimerFired()
{
    isLocked       = false;
    m_attemptsLeft = 3;
    otpInput->setEnabled(true);
    otpInput->clear();
    statusLabel->hide();
    updateVerifyBtn();
}

/// Пустой обработчик ответа сервера (зарезервирован для будущей реализации)
void VerifyWidget::onVerifyResponseReceived(const QString &) {}
