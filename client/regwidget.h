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

    /** @brief Отправляет введённый код на сервер для верификации. */
    void onVerifyCodeClicked();

    /** @brief Вызывается по истечении таймера блокировки ввода кода. */
    void onCodeLockTimerFired();

    /** @brief Обрабатывает нажатие ссылки «Войти». */
    void onShowAuthClicked();

private:
    // ── Шаг 1: логин + пароль ──────────────────────────────────────────────
    QWidget     *step1Widget;          ///< Контейнер элементов шага 1.
    QLineEdit   *loginEdit;            ///< Поле ввода логина.
    QLabel      *loginErrorLabel;      ///< Ошибка валидации логина.
    QLineEdit   *passwordEdit;         ///< Поле ввода пароля.
    QLabel      *passwordErrorLabel;   ///< Ошибка валидации пароля.
    QPushButton *togglePassBtn1;       ///< Кнопка показа/скрытия пароля.
    QLineEdit   *confirmPasswordEdit;  ///< Поле подтверждения пароля.
    QLabel      *confirmErrorLabel;    ///< Ошибка несовпадения паролей.
    QPushButton *togglePassBtn2;       ///< Кнопка показа/скрытия пароля подтверждения.
    QPushButton *continueBtn;          ///< Кнопка «Продолжить».

    // ── Шаг 2: ввод email ──────────────────────────────────────────────────
    QWidget     *step2Widget;    ///< Контейнер элементов шага 2.
    QLineEdit   *emailEdit;      ///< Поле ввода email.
    QLabel      *emailErrorLabel;///< Ошибка валидации email.
    QPushButton *emailNextBtn;   ///< Кнопка «Далее →».
    QPushButton *backToStep1Btn; ///< Кнопка «← Назад».

    // ── Шаг 3: ввод кода ───────────────────────────────────────────────────
    QWidget     *step3Widget;      ///< Контейнер элементов шага 3.
    QLabel      *emailHintLabel;   ///< Подсказка «Код отправлен на ...».
    QLineEdit   *codeEdit;         ///< Поле ввода кода (6 цифр).
    QLabel      *codeErrorLabel;   ///< Ошибка неверного кода.
    QLabel      *codeStatusLabel;  ///< Статус проверки кода.
    QPushButton *verifyCodeBtn;    ///< Кнопка «Подтвердить».
    QPushButton *backToStep2Btn;   ///< Кнопка «← Изменить почту».

    QPushButton *showAuthBtn; ///< Ссылка «Уже есть аккаунт? Войти».

    int     codeFailedAttempts; ///< Счётчик неверных попыток ввода кода.
    int     codeLockLevel;      ///< Уровень блокировки кода.
    QTimer  *codeLockTimer;     ///< Таймер снятия блокировки.
    bool    codeIsLocked;       ///< Флаг активной блокировки ввода кода.
    bool    m_verifyingCode;    ///< Флаг ожидания ответа на verify_code.
    bool    m_checkingLogin;    ///< Флаг ожидания ответа на check_login.
    QString currentLogin;       ///< Логин, прошедший проверку на шаге 1.
    QString currentEmail;       ///< Email, введённый на шаге 2.

    /** @brief Инициализирует и компонует все элементы интерфейса. */
    void setupUI();

    /** @brief Обновляет состояние кнопки «Продолжить» по результатам валидации шага 1. */
    void validateStep1();

    /**
     * @brief Проверяет корректность формата email.
     * @param email Строка для проверки.
     * @return true, если формат валиден.
     */
    bool isEmailValid(const QString &email) const;

    /**
     * @brief Блокирует ввод кода на заданное время.
     * @param minutes Длительность блокировки (0 — 30 секунд).
     * @param message Сообщение для отображения пользователю.
     */
    void applyCodeLock(int minutes, const QString &message);

    /**
     * @brief Показывает нужный шаг и скрывает остальные.
     * @param step Номер шага (1, 2 или 3).
     */
    void showStep(int step);

    /** @brief @return CSS-стиль поля ввода. */
    QString primaryBtnStyle() const;
    /** @brief @return CSS-стиль вторичной кнопки. */
    QString secondaryBtnStyle() const;
    /** @brief @return CSS-стиль кнопки-призрака. */
    QString ghostBtnStyle() const;
    /** @brief @return CSS-стиль кнопки-ссылки. */
    QString linkBtnStyle() const;
    /** @brief @return CSS-стиль поля ввода. */
    QString inputStyle() const;
    /** @brief @return CSS-стиль метки ошибки. */
    QString errorLabelStyle() const;
    /** @brief @return CSS-стиль информационной метки. */
    QString infoLabelStyle() const;
    /** @brief @return CSS-стиль метки успеха. */
    QString successLabelStyle() const;
};

#endif // REGWIDGET_H
