#ifndef REGWIDGET_H
#define REGWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QKeyEvent>

class OtpInput;

/**
 * @brief Виджет регистрации нового пользователя
 * 
 * Реализует трёхшаговый процесс регистрации:
 * 1. Ввод логина и пароля
 * 2. Ввод email
 * 3. Ввод OTP-кода подтверждения
 * 
 * Поддерживает валидацию полей, проверку доступности логина,
 * блокировку при превышении лимита попыток ввода кода
 * и навигацию между шагами клавишей Escape.
 */
class RegWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета регистрации
     * 
     * @param parent Родительский виджет
     */
    explicit RegWidget(QWidget *parent = nullptr);

    /**
     * @brief Деструктор виджета регистрации
     */
    ~RegWidget();

    /**
     * @brief Очистить все поля и сбросить состояние виджета
     * 
     * Сбрасывает все поля ввода, статусные метки, флаги состояния
     * и возвращает виджет на первый шаг регистрации.
     */
    void clearFields();

signals:
    /// @brief Сигнал об успешной регистрации
    /// @param login Логин зарегистрированного пользователя
    void registrationSuccess(const QString &login);

    /// @brief Сигнал для переключения на экран авторизации
    void showAuth();

protected:
    /// @brief Обработчик нажатия клавиш для навигации Escape
    /// @param e Событие нажатия клавиши
    void keyPressEvent(QKeyEvent *e) override;

private slots:
    /// @brief Слот для обработки изменения текста в поле логина
    /// @param text Новый текст
    void onLoginTextChanged(const QString &text);

    /// @brief Слот для обработки изменения текста в поле пароля
    /// @param text Новый текст
    void onPasswordTextChanged(const QString &text);

    /// @brief Слот для обработки изменения текста в поле подтверждения пароля
    /// @param text Новый текст
    void onConfirmPasswordTextChanged(const QString &text);

    /// @brief Слот для переключения видимости первого пароля
    void onTogglePassword1();

    /// @brief Слот для переключения видимости второго пароля
    void onTogglePassword2();

    /// @brief Слот для обработки нажатия кнопки "Продолжить"
    void onContinueClicked();

    /// @brief Слот для обработки изменения текста в поле email
    /// @param text Новый текст
    void onEmailTextChanged(const QString &text);

    /// @brief Слот для обработки нажатия кнопки "Далее →"
    void onEmailNextClicked();

    /// @brief Слот для возврата к шагу 1 из шага 2
    void onBackToStep1Clicked();

    /// @brief Слот для возврата к шагу 2 из шага 3
    void onBackToStep2Clicked();

    /// @brief Слот для обработки ответа сервера на запросы регистрации
    /// @param response Ответ сервера
    void onRegistrationResponseReceived(const QString &response);

    /// @brief Слот для обработки завершения ввода OTP-кода
    /// @param code Введённый код
    void onCodeCompleted(const QString &code);

    /// @brief Слот для обработки нажатия кнопки "Подтвердить"
    void onVerifyCodeClicked();

    /// @brief Слот для обработки срабатывания таймера блокировки ввода кода
    void onCodeLockTimerFired();

    /// @brief Слот для обработки нажатия ссылки "Уже есть аккаунт? Войти"
    void onShowAuthClicked();

private:
    // ── Шаг 1 ──────────────────────────────────────────────────────
    QWidget     *step1Widget;        ///< Виджет шага 1 (логин + пароль)
    QLineEdit   *loginEdit;          ///< Поле ввода логина
    QLabel      *loginErrorLabel;    ///< Метка ошибки логина
    QLineEdit   *passwordEdit;       ///< Поле ввода пароля
    QLabel      *passwordErrorLabel; ///< Метка ошибки пароля
    QPushButton *togglePassBtn1;     ///< Кнопка показа/скрытия первого пароля
    QLineEdit   *confirmPasswordEdit; ///< Поле подтверждения пароля
    QLabel      *confirmErrorLabel;  ///< Метка ошибки подтверждения пароля
    QPushButton *togglePassBtn2;     ///< Кнопка показа/скрытия второго пароля
    QPushButton *continueBtn;        ///< Кнопка "Продолжить"

    // ── Шаг 2 ──────────────────────────────────────────────────────
    QWidget     *step2Widget;        ///< Виджет шага 2 (email)
    QLineEdit   *emailEdit;          ///< Поле ввода email
    QLabel      *emailErrorLabel;    ///< Метка ошибки email
    QPushButton *emailNextBtn;       ///< Кнопка "Далее →"
    QPushButton *backToStep1Btn;     ///< Кнопка "← Назад" к шагу 1

    // ── Шаг 3 ──────────────────────────────────────────────────────
    QWidget     *step3Widget;        ///< Виджет шага 3 (OTP-код)
    QLabel      *emailHintLabel;     ///< Метка-подсказка с адресом email
    OtpInput    *otpCode;            ///< Ввод OTP-кода из 6 цифр
    QLabel      *codeErrorLabel;     ///< Метка ошибки кода
    QLabel      *codeStatusLabel;    ///< Метка статуса верификации кода
    QPushButton *verifyCodeBtn;      ///< Кнопка "Подтвердить"
    QPushButton *backToStep2Btn;     ///< Кнопка "← Изменить почту"

    QPushButton *showAuthBtn;        ///< Ссылка "Уже есть аккаунт? Войти"

    int     codeFailedAttempts;      ///< Количество неудачных попыток ввода кода
    int     codeLockLevel;           ///< Текущий уровень блокировки ввода кода
    QTimer  *codeLockTimer;          ///< Таймер блокировки ввода кода
    bool    codeIsLocked;            ///< Флаг: ввод кода заблокирован
    bool    m_checkingLogin;         ///< Флаг: ожидание проверки доступности логина
    bool    m_waitingForRegCode;     ///< Флаг: ожидание кода регистрации от сервера
    bool    m_verifyingCode;         ///< Флаг: ожидание подтверждения кода
    QString m_pendingCodeHash;       ///< Хэш ожидаемого OTP-кода
    QString m_pendingPassHash;       ///< Хэш пароля для отправки на сервер
    QString currentLogin;            ///< Текущий логин (сохранён после проверки)
    QString currentEmail;            ///< Текущий email

    /**
     * @brief Настройка пользовательского интерфейса
     * 
     * Создаёт все три шага регистрации с полями ввода и кнопками.
     */
    void setupUI();

    /**
     * @brief Валидация полей шага 1
     * 
     * Проверяет минимальную длину логина (4), пароля (8)
     * и совпадение паролей. Включает/выключает кнопку "Продолжить".
     */
    void validateStep1();

    /**
     * @brief Проверить корректность формата email
     * 
     * @param email Адрес электронной почты для проверки
     * @return true если email соответствует формату, false в противном случае
     */
    bool isEmailValid(const QString &email) const;

    /**
     * @brief Применить блокировку ввода OTP-кода
     * 
     * @param minutes Длительность блокировки в минутах (0 = 30 секунд)
     * @param message Сообщение для отображения пользователю
     */
    void applyCodeLock(int minutes, const QString &message);

    /**
     * @brief Показать указанный шаг регистрации
     * 
     * @param step Номер шага (1, 2 или 3)
     */
    void showStep(int step);

    /**
     * @brief Обновить состояние кнопки "Подтвердить"
     * 
     * Кнопка активна только если OTP-код введён полностью,
     * блокировка не действует и хэш кода получен от сервера.
     */
    void updateVerifyCodeBtn();

    /// @return CSS-стиль для основной кнопки (зелёная)
    QString primaryBtnStyle() const;

    /// @return CSS-стиль для вторичной кнопки
    QString secondaryBtnStyle() const;

    /// @return CSS-стиль для призрачной кнопки
    QString ghostBtnStyle() const;

    /// @return CSS-стиль для кнопки-ссылки
    QString linkBtnStyle() const;

    /// @return CSS-стиль для поля ввода
    QString inputStyle() const;

    /// @return CSS-стиль для метки ошибки
    QString errorLabelStyle() const;

    /// @return CSS-стиль для информационной метки
    QString infoLabelStyle() const;

    /// @return CSS-стиль для метки успешного действия
    QString successLabelStyle() const;
};

#endif // REGWIDGET_H
