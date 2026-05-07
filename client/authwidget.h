/**
 * @file authwidget.h
 * @brief Виджет авторизации пользователя.
 */

#ifndef AUTHWIDGET_H
#define AUTHWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

/**
 * @class AuthWidget
 * @brief Экран входа в систему.
 *
 * Предоставляет поля ввода логина и пароля, кнопки входа,
 * регистрации и восстановления пароля. Реализует защиту
 * от перебора с прогрессивной блокировкой по времени.
 */
class AuthWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский виджет.
     */
    explicit AuthWidget(QWidget *parent = nullptr);

    /** @brief Деструктор. */
    ~AuthWidget();

    /** @brief Сбрасывает поля ввода и состояние виджета при его показе. */
    void clearFields();

signals:
    /**
     * @brief Испускается при успешной авторизации.
     * @param login Логин вошедшего пользователя.
     */
    void loginSuccess(QString login);

    /** @brief Испускается при переходе на экран регистрации. */
    void showRegister();

    /**
     * @brief Испускается для перехода к экрану подтверждения входа по коду.
     * @param login Логин пользователя, ожидающего верификацию.
     */
    void showVerifyAuth(QString login);

    /** @brief Испускается для перехода на экран восстановления пароля. */
    void showReset();

private slots:
    /** @brief Обрабатывает нажатие кнопки «Войти». */
    void onLoginClicked();

    /** @brief Переключает видимость пароля в поле ввода. */
    void onTogglePassword();

    /** @brief Вызывается по истечении таймера блокировки входа. */
    void onLockTimerFired();

    /** @brief Обрабатывает нажатие кнопки «Регистрация». */
    void onRegisterClicked();

    /** @brief Обрабатывает нажатие кнопки «Забыли пароль?». */
    void onForgotClicked();

    /**
     * @brief Обрабатывает ответ сервера на запрос авторизации.
     * @param response Строка ответа от сервера.
     */
    void onAuthResponseReceived(const QString &response);

private:
    QLineEdit   *loginEdit;         ///< Поле ввода логина.
    QLineEdit   *passwordEdit;      ///< Поле ввода пароля.
    QPushButton *loginBtn;          ///< Кнопка входа.
    QPushButton *registerBtn;       ///< Кнопка перехода к регистрации.
    QPushButton *forgotBtn;         ///< Кнопка восстановления пароля.
    QPushButton *togglePasswordBtn; ///< Кнопка показа/скрытия пароля.
    QLabel      *statusLabel;       ///< Метка для отображения статуса/ошибок.
    QLabel      *attemptsLabel;     ///< Метка оставшихся попыток входа.

    int    failedAttempts; ///< Счётчик неудачных попыток входа.
    int    lockLevel;      ///< Уровень блокировки (0, 1, 2 ...).
    QTimer *lockTimer;     ///< Таймер снятия блокировки.
    bool   isLocked;       ///< Флаг активной блокировки.
    bool   m_waitingForAuth; ///< Флаг ожидания ответа сервера.

    /**
     * @brief Применяет блокировку входа на заданное время.
     * @param minutes Длительность блокировки в минутах (0 — 30 секунд).
     * @param message Сообщение, отображаемое пользователю.
     */
    void applyLock(int minutes, const QString &message);

    /** @brief Инициализирует и компонует элементы интерфейса. */
    void setupUI();
};

#endif // AUTHWIDGET_H
