/**
 * @file regwidget.h
 * @brief Виджет регистрации нового пользователя.
 */

#ifndef REGWIDGET_H
#define REGWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

/**
 * @class RegWidget
 * @brief Экран регистрации нового аккаунта.
 *
 * Реализует трёхшаговый процесс регистрации:
 * - Шаг 1: ввод логина и пароля;
 * - Шаг 2: ввод адреса электронной почты;
 * - Шаг 3: подтверждение кода из письма.
 *
 * Обеспечивает клиентскую валидацию полей, проверку логина
 * на стороне сервера и прогрессивную блокировку при ошибках кода.
 */
class RegWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский виджет.
     */
    explicit RegWidget(QWidget *parent = nullptr);

    /** @brief Деструктор. */
    ~RegWidget();

    /**
     * @brief Сбрасывает все поля и состояние виджета на начальный шаг.
     *
     * Вызывается из MainWindow при переходе на экран регистрации,
     * чтобы предыдущие данные не отображались.
     */
    void clearFields();

signals:
    /**
     * @brief Испускается при успешной регистрации.
     * @param login Логин зарегистрированного пользователя.
     */
    void registrationSuccess(const QString &login);

    /** @brief Испускается при нажатии ссылки «Уже есть аккаунт? Войти». */
    void showAuth();

private slots:
    /** @brief Валидирует поле логина по мере ввода. @param text Текущее значение поля. */
    void onLoginTextChanged(const QString &text);

    /** @brief Валидирует поле пароля по мере ввода. @param text Текущее значение поля. */
    void onPasswordTextChanged(const QString &text);

    /** @brief Валидирует поле подтверждения пароля. @param text Текущее значение поля. */
    void onConfirmPasswordTextChanged(const QString &text);

    /** @brief Переключает видимость основного пароля. */
    void onTogglePassword1();

    /** @brief Переключает видимость пароля подтверждения. */
    void onTogglePassword2();

    /** @brief Инициирует проверку логина на сервере и переход к шагу 2. */
    void onContinueClicked();

    /** @brief Валидирует поле email по мере ввода. @param text Текущее значение поля. */
    void onEmailTextChanged(const QString &text);

    /** @brief Отправляет запрос регистрации на сервер и переходит к шагу 3. */
    void onEmailNextClicked();

    /** @brief Возвращает пользователя с шага 2/3 на шаг 1. */
    void onBackToStep1Clicked();

    /** @brief Возвращает пользователя с шага 3 на шаг 2. */
    void onBackToStep2Clicked();

    /**
     * @brief Обрабатывает все ответы сервера в процессе регистрации.
     * @param response Строка ответа (check_login, login_free, reg+, reg-, wrong_code и др.).
     */
    void onRegistrationResponseReceived(const QString &response);

    /**
     * @brief Активирует кнопку подтверждения кода при вводе ровно 6 символов.
     * @param text Текущее значение поля кода.
     */
    void onCodeTextChanged(const QString &text);

    /** @brief Локально проверяет код и завершает регистрацию через registration_confirm. */
    void onVerifyCodeClicked();

    /** @brief Вызывается по истечении таймера блокировки ввода кода. */
    void onCodeLockTimerFired();

    /** @brief Обрабатывает нажатие ссылки «Войти». */
    void onShowAuthClicked();

private:
    // ═══ Шаг 1: логин + пароль ══════════════════════════════════════
    QWidget     *step1Widget;
    QLineEdit   *loginEdit;
    QLabel      *loginErrorLabel;
    QLineEdit   *passwordEdit;
    QLabel      *passwordErrorLabel;
    QPushButton *togglePassBtn1;
    QLineEdit   *confirmPasswordEdit;
    QLabel      *confirmErrorLabel;
    QPushButton *togglePassBtn2;
    QPushButton *continueBtn;

    // ═══ Шаг 2: ввод email ════════════════════════════════════
    QWidget     *step2Widget;
    QLineEdit   *emailEdit;
    QLabel      *emailErrorLabel;
    QPushButton *emailNextBtn;
    QPushButton *backToStep1Btn;

    // ═══ Шаг 3: ввод кода ═════════════════════════════════════
    QWidget     *step3Widget;
    QLabel      *emailHintLabel;
    QLineEdit   *codeEdit;
    QLabel      *codeErrorLabel;
    QLabel      *codeStatusLabel;
    QPushButton *verifyCodeBtn;
    QPushButton *backToStep2Btn;

    QPushButton *showAuthBtn;

    int     codeFailedAttempts; ///< Счётчик неверных попыток ввода кода.
    int     codeLockLevel;      ///< Уровень блокировки кода.
    QTimer  *codeLockTimer;     ///< Таймер снятия блокировки.
    bool    codeIsLocked;       ///< Флаг активной блокировки ввода кода.
    bool    m_checkingLogin;    ///< Флаг ожидания ответа на check_login.
    bool    m_verifyingCode;    ///< Флаг ожидания ответа на registration_confirm.
    QString m_pendingCodeHash;  ///< SHA-256 хэш кода от сервера для локального сравнения.
    QString m_pendingPassHash;  ///< Хэш пароля для финального registration_confirm.
    QString currentLogin;       ///< Логин, прошедший проверку на шаге 1.
    QString currentEmail;       ///< Email, введённый на шаге 2.

    void setupUI();
    void validateStep1();
    bool isEmailValid(const QString &email) const;
    void applyCodeLock(int minutes, const QString &message);
    void showStep(int step);

    QString primaryBtnStyle() const;
    QString secondaryBtnStyle() const;
    QString ghostBtnStyle() const;
    QString linkBtnStyle() const;
    QString inputStyle() const;
    QString errorLabelStyle() const;
    QString infoLabelStyle() const;
    QString successLabelStyle() const;
};

#endif // REGWIDGET_H
