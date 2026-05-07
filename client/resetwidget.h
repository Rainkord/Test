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
 * Реализует двухшаговый процесс сброса пароля:
 * - Шаг 1: ввод email для получения кода;
 * - Шаг 2: ввод кода и нового пароля.
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
    /** @brief Валидирует поле email и активирует кнопку «Далее». @param text Текущее значение. */
    void onEmailTextChanged(const QString &text);

    /** @brief Отправляет запрос на отправку кода восстановления. */
    void onSendCodeClicked();

    /** @brief Возвращает на экран авторизации. */
    void onBackClicked();

    /** @brief Валидирует поля кода и нового пароля. @param text Текущее значение. */
    void onCodeOrPasswordChanged(const QString &text);

    /** @brief Отправляет запрос сброса пароля с кодом и новым паролем. */
    void onResetClicked();

    /** @brief Возвращает пользователя с шага 2 на шаг 1. */
    void onBackToEmailClicked();

    /** @brief Вызывается по истечении таймера блокировки. */
    void onLockTimerFired();

    /**
     * @brief Обрабатывает ответы сервера в процессе восстановления пароля.
     * @param response Строка ответа от сервера.
     */
    void onResetResponseReceived(const QString &response);

private:
    // ── Шаг 1: email ───────────────────────────────────────────────────────
    QWidget     *step1Widget;      ///< Контейнер элементов шага 1.
    QLineEdit   *emailEdit;        ///< Поле ввода email.
    QLabel      *emailErrorLabel;  ///< Ошибка валидации email.
    QPushButton *sendCodeBtn;      ///< Кнопка «Отправить код».
    QPushButton *backBtn;          ///< Кнопка «Назад».

    // ── Шаг 2: код + новый пароль ──────────────────────────────────────────
    QWidget     *step2Widget;         ///< Контейнер элементов шага 2.
    QLineEdit   *codeEdit;            ///< Поле ввода кода (6 символов).
    QLineEdit   *newPasswordEdit;     ///< Поле нового пароля.
    QLineEdit   *confirmPasswordEdit; ///< Поле подтверждения нового пароля.
    QLabel      *codeErrorLabel;      ///< Ошибка кода/блокировки.
    QLabel      *codeStatusLabel;     ///< Статус проверки кода.
    QPushButton *resetBtn;            ///< Кнопка «Сменить пароль».
    QPushButton *backToEmailBtn;      ///< Кнопка «← Изменить email».

    int     lockLevel;          ///< Уровень блокировки.
    QTimer  *lockTimer;         ///< Таймер снятия блокировки.
    bool    isLocked;           ///< Флаг активной блокировки.
    bool    m_waitingForReset;  ///< Флаг ожидания ответа сервера.
    bool    m_waitingForCode;   ///< Флаг ожидания ответа на отправку кода.
    QString currentEmail;       ///< Email, введённый на шаге 1.

    /** @brief Инициализирует и компонует элементы интерфейса. */
    void setupUI();

    /**
     * @brief Показывает нужный шаг и скрывает остальные.
     * @param step Номер шага (1 или 2).
     */
    void showStep(int step);

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
};

#endif // RESETWIDGET_H
