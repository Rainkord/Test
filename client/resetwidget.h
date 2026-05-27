#ifndef RESETWIDGET_H
#define RESETWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QKeyEvent>

class OtpInput;

/**
 * @brief Виджет восстановления пароля через email
 *
 * Пошаговый виджет сброса пароля: ввод email → проверка OTP-кода →
 * установка нового пароля. Поддерживает блокировку при превышении
 * лимита попыток ввода кода.
 */
class ResetWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета сброса пароля
     * @param parent родительский виджет
     */
    explicit ResetWidget(QWidget *parent = nullptr);

    /// Деструктор виджета
    ~ResetWidget();

    /// Полностью очищает все поля и сбрасывает внутреннее состояние
    void clearFields();

signals:
    /// Испускается при успешном сохранении нового пароля
    void resetSuccess();

    /// Испускается при возврате на экран авторизации
    void backToAuth();

protected:
    /**
     * @brief Обработчик нажатия клавиш (Escape — назад на предыдущий шаг)
     * @param e событие нажатия клавиши
     */
    void keyPressEvent(QKeyEvent *e) override;

private slots:
    /**
     * @brief Обработчик изменения текста в поле email
     * @param text текущее содержимое поля email
     */
    void onEmailTextChanged(const QString &text);

    /// Обработчик нажатия кнопки «Отправить код»
    void onContinueClicked();

    /**
     * @brief Обработчик изменения OTP-кода
     * @param text текущее содержимое OTP-ввода
     */
    void onCodeTextChanged(const QString &text);

    /// Обработчик нажатия кнопки «Подтвердить код»
    void onVerifyCodeClicked();

    /**
     * @brief Обработчик изменения поля «Новый пароль»
     * @param text текущее значение нового пароля
     */
    void onNewPasswordTextChanged(const QString &text);

    /**
     * @brief Обработчик изменения поля «Подтвердите пароль»
     * @param text текущее значение подтверждения пароля
     */
    void onConfirmPasswordTextChanged(const QString &text);

    /// Переключение видимости нового пароля (показать/скрыть)
    void onTogglePassword1();

    /// Переключение видимости подтверждения пароля (показать/скрыть)
    void onTogglePassword2();

    /// Обработчик нажатия кнопки «Сохранить пароль»
    void onSavePasswordClicked();

    /// Обработчик нажатия кнопки «Назад»
    void onBackClicked();

    /// Обработчик срабатывания таймера блокировки
    void onLockTimerFired();

    /**
     * @brief Обработчик ответа сервера на запросы сброса пароля
     * @param response строка ответа от сервера
     */
    void onResetResponseReceived(const QString &response);

private:
    /// Перечисление шагов процесса восстановления пароля
    enum Step { StepEmail, StepCode, StepPassword };

    // ── Шаг 1: email ──────────────────────────────────────────────────────
    QWidget     *step1Widget;     ///< Виджет шага ввода email
    QLineEdit   *emailEdit;       ///< Поле ввода email
    QLabel      *emailErrorLabel; ///< Метка ошибки ввода email
    QPushButton *continueBtn;     ///< Кнопка «Отправить код»
    QPushButton *backBtn;         ///< Кнопка «Назад»

    // ── Шаг 2: код ────────────────────────────────────────────────────────
    QWidget     *step2Widget;     ///< Виджет шага ввода OTP-кода
    OtpInput    *codeEdit;        ///< Виджет ввода OTP-кода
    QLabel      *codeErrorLabel;  ///< Метка ошибки ввода кода
    QLabel      *codeStatusLabel; ///< Метка статуса верификации кода
    QPushButton *verifyCodeBtn;   ///< Кнопка «Подтвердить код»

    // ── Шаг 3: новый пароль ───────────────────────────────────────────────
    QWidget     *step3Widget;          ///< Виджет шага установки нового пароля
    QLineEdit   *newPasswordEdit;      ///< Поле нового пароля
    QLineEdit   *confirmPasswordEdit;  ///< Поле подтверждения пароля
    QLabel      *newPasswordErrorLabel; ///< Метка ошибки нового пароля
    QLabel      *confirmErrorLabel;    ///< Метка ошибки подтверждения пароля
    QPushButton *togglePassBtn1;       ///< Кнопка показа/скрытия нового пароля
    QPushButton *togglePassBtn2;       ///< Кнопка показа/скрытия подтверждения
    QPushButton *saveBtn;              ///< Кнопка «Сохранить пароль»

    // ── Состояние ─────────────────────────────────────────────────────────
    Step    m_currentStep;          ///< Текущий шаг процесса
    QString m_email;                ///< Адрес email пользователя
    QString m_pendingCodeHash;      ///< Хэш полученного от сервера OTP-кода
    int     failedAttempts;         ///< Количество неудачных попыток ввода кода
    int     lockLevel;              ///< Уровень блокировки (нарастающий)
    bool    isLocked;               ///< Флаг: ввод заблокирован
    QTimer *lockTimer;              ///< Таймер блокировки
    bool    m_waitingForCodeHash;   ///< Флаг: ожидание ответа сервера на запрос кода
    bool    m_waitingForSave;       ///< Флаг: ожидание ответа сервера на сохранение пароля

    /// Инициализация пользовательского интерфейса
    void setupUI();

    /**
     * @brief Переключает отображаемый шаг
     * @param step номер шага для отображения
     */
    void showStep(Step step);

    /**
     * @brief Проверяет корректность формата email
     * @param email адрес электронной почты для проверки
     * @return true если email соответствует регулярному выражению
     */
    bool isEmailValid(const QString &email) const;

    /// Проверяет совпадение и корректность паролей, обновляет состояние кнопки
    void validatePasswords();

    /**
     * @brief Блокирует ввод на указанное время
     * @param minutes количество минут блокировки (0 = 30 секунд)
     * @param message сообщение при блокировке
     */
    void applyLock(int minutes, const QString &message);
};

#endif // RESETWIDGET_H
