/**
 * @file resetwidget.h
 * @brief Виджет восстановления пароля.
 */

#ifndef RESETWIDGET_H
#define RESETWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

/**
 * @class ResetWidget
 * @brief Экран восстановления забытого пароля.
 *
 * Реализует трёхшаговый процесс сброса пароля:
 * - Шаг 1: ввод email и запрос кода;
 * - Шаг 2: ввод кода подтверждения;
 * - Шаг 3: ввод нового пароля.
 *
 * Поддерживает валидацию полей и прогрессивную блокировку
 * при многократном вводе неверного кода.
 */
class ResetWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский виджет.
     */
    explicit ResetWidget(QWidget *parent = nullptr);

    /** @brief Деструктор. */
    ~ResetWidget();

signals:
    /** @brief Испускается после успешного сброса пароля. */
    void resetSuccess();

    /** @brief Испускается при нажатии кнопки «Назад» на шаге 1. */
    void backToAuth();

private slots:
    /** @brief Валидирует поле email и активирует кнопку «Далее». */
    void onEmailTextChanged(const QString &text);

    /** @brief Отправляет запрос на получение кода на email (шаг 1 → шаг 2). */
    void onContinueClicked();

    /** @brief Валидирует код по мере ввода. */
    void onCodeTextChanged(const QString &text);

    /** @brief Отправляет запрос верификации кода (шаг 2 → шаг 3). */
    void onVerifyCodeClicked();

    /** @brief Валидирует поле нового пароля. */
    void onNewPasswordTextChanged(const QString &text);

    /** @brief Валидирует поле подтверждения пароля. */
    void onConfirmPasswordTextChanged(const QString &text);

    /** @brief Переключает видимость нового пароля. */
    void onTogglePassword1();

    /** @brief Переключает видимость подтверждения пароля. */
    void onTogglePassword2();

    /** @brief Отправляет запрос сохранения нового пароля. */
    void onSavePasswordClicked();

    /** @brief Возвращает на экран авторизации. */
    void onBackClicked();

    /** @brief Вызывается по истечении таймера блокировки. */
    void onLockTimerFired();

    /**
     * @brief Обрабатывает ответы сервера в процессе восстановления пароля.
     * @param response Строка ответа от сервера.
     */
    void onResetResponseReceived(const QString &response);

private:
    /** @brief Шаги экрана восстановления. */
    enum Step { StepEmail, StepCode, StepPassword };

    // ── Шаг 1: email ───────────────────────────────────────────────────────
    QWidget     *step1Widget;      ///< Контейнер элементов шага 1.
    QLineEdit   *emailEdit;        ///< Поле ввода email.
    QLabel      *emailErrorLabel;  ///< Ошибка валидации email.
    QPushButton *continueBtn;      ///< Кнопка «Продолжить».
    QPushButton *backBtn;          ///< Кнопка «Назад».

    // ── Шаг 2: код ─────────────────────────────────────────────────────────
    QWidget     *step2Widget;      ///< Контейнер элементов шага 2.
    QLineEdit   *codeEdit;         ///< Поле ввода кода.
    QLabel      *codeErrorLabel;   ///< Ошибка кода / сообщение блокировки.
    QLabel      *codeStatusLabel;  ///< Статус проверки кода.
    QPushButton *verifyCodeBtn;    ///< Кнопка «Подтвердить код».
    QPushButton *sendCodeBtn;      ///< Кнопка повторной отправки кода.

    // ── Шаг 3: новый пароль ────────────────────────────────────────────────
    QWidget     *step3Widget;           ///< Контейнер элементов шага 3.
    QLineEdit   *newPasswordEdit;       ///< Поле нового пароля.
    QLineEdit   *confirmPasswordEdit;   ///< Поле подтверждения пароля.
    QLabel      *newPasswordErrorLabel; ///< Ошибка нового пароля.
    QLabel      *confirmErrorLabel;     ///< Ошибка подтверждения пароля.
    QPushButton *togglePassBtn1;        ///< Кнопка показа нового пароля.
    QPushButton *togglePassBtn2;        ///< Кнопка показа подтверждения.
    QPushButton *saveBtn;               ///< Кнопка «Сохранить пароль».

    // ── Состояние ──────────────────────────────────────────────────────────
    Step    m_currentStep;       ///< Текущий активный шаг.
    int     failedAttempts;      ///< Счётчик неверных попыток ввода кода.
    int     lockLevel;           ///< Уровень блокировки.
    QTimer *lockTimer;           ///< Таймер снятия блокировки.
    bool    isLocked;            ///< Флаг активной блокировки.
    bool    m_waitingForReset;   ///< Флаг ожидания ответа сервера (смена пароля).
    bool    m_waitingForCode;    ///< Флаг ожидания ответа на отправку кода.
    bool    m_waitingForResponse;///< Общий флаг ожидания ответа сервера.
    QString currentEmail;        ///< Email, введённый на шаге 1 (устар., см. m_email).
    QString m_email;             ///< Email пользователя, сохранённый для передачи на шаги 2/3.
    QString m_code;              ///< Код подтверждения, сохранённый для передачи на шаг 3.

    /** @brief Инициализирует и компонует элементы интерфейса. */
    void setupUI();

    /**
     * @brief Показывает нужный шаг и скрывает остальные.
     * @param step Шаг для отображения.
     */
    void showStep(Step step);

    /** @brief Проверяет совпадение и сложность паролей, активирует saveBtn. */
    void validatePasswords();

    /**
     * @brief Проверяет корректность формата email.
     * @param email Строка для проверки.
     * @return true, если формат валиден.
     */
    bool isEmailValid(const QString &email) const;

    /**
     * @brief Блокирует ввод кода на заданное время.
     * @param minutes Длительность блокировки (0 — 30 секунд).
     * @param message Сообщение для отображения.
     */
    void applyCodeLock(int minutes, const QString &message);

    /**
     * @brief Псевдоним applyCodeLock, используемый в cpp.
     * @param minutes Длительность блокировки.
     * @param message Сообщение.
     */
    void applyLock(int minutes, const QString &message);
};

#endif // RESETWIDGET_H
