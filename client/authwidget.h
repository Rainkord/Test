#pragma once

#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;
class QTimer;

/**
 * @brief Виджет авторизации пользователя
 * 
 * Предоставляет интерфейс для входа в аккаунт: ввод логина и пароля,
 * отправка запроса на сервер, обработка ответа с блокировкой
 * при превышении лимита попыток. Поддерживает переход к регистрации
 * и восстановлению пароля.
 */
class AuthWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета авторизации
     * 
     * @param parent Родительский виджет
     */
    explicit AuthWidget(QWidget *parent = nullptr);

    /**
     * @brief Деструктор виджета авторизации
     */
    ~AuthWidget();

    /**
     * @brief Очистить все поля ввода и сбросить состояние
     * 
     * Сбрасывает логин, пароль, статусные метки, флаги ожидания
     * и включает кнопку входа.
     */
    void clearFields();

signals:
    /// @brief Сигнал для переключения на экран регистрации
    void showRegister();

    /// @brief Сигнал для переключения на экран восстановления пароля
    void showReset();

    /// @brief Сигнал для переключения на экран верификации при входе
    /// @param login Логин пользователя
    /// @param codeHash Хэш кода подтверждения
    void showVerifyAuth(const QString &login, const QString &codeHash);

private slots:
    /// @brief Слот для переключения видимости пароля
    void onTogglePassword();

    /// @brief Слот для обработки нажатия кнопки "Войти"
    void onLoginClicked();

    /// @brief Слот для обработки нажатия ссылки "Регистрация"
    void onRegisterClicked();

    /// @brief Слот для обработки нажатия ссылки "Забыл пароль"
    void onForgotClicked();

    /// @brief Слот для обработки ответа сервера на запрос авторизации
    /// @param response Ответ сервера
    void onAuthResponseReceived(const QString &response);

    /// @brief Слот для обработки срабатывания таймера блокировки
    void onLockTimerFired();

private:
    /**
     * @brief Настройка пользовательского интерфейса
     * 
     * Создаёт карточку с полями ввода, кнопками и статусными метками.
     */
    void setupUI();

    /**
     * @brief Применить блокировку входа на указанное время
     * 
     * @param durationSec Длительность блокировки в секундах
     * @param message Сообщение для отображения пользователю
     */
    void applyLock(int durationSec, const QString &message);

    /// Поле ввода логина
    QLineEdit   *loginEdit;

    /// Поле ввода пароля
    QLineEdit   *passwordEdit;

    /// Кнопка показа/скрытия пароля
    QPushButton *togglePasswordBtn;

    /// Кнопка "Войти"
    QPushButton *loginBtn;

    /// Ссылка "Регистрация"
    QPushButton *registerBtn;

    /// Ссылка "Забыл пароль"
    QPushButton *forgotBtn;

    /// Метка статуса/ошибок
    QLabel      *statusLabel;

    /// Метка оставшихся попыток
    QLabel      *attemptsLabel;

    /// Таймер блокировки при превышении лимита попыток
    QTimer      *lockTimer;

    /// Количество неудачных попыток входа подряд
    int     failedAttempts;

    /// Текущий уровень блокировки (0 — без блокировки)
    int     lockLevel;

    /// Флаг: вход заблокирован из-за превышения лимита попыток
    bool    isLocked;

    /// Флаг: ожидание ответа сервера на запрос авторизации
    bool    m_waitingForAuth;

    /// Логин текущей попытки входа
    QString m_pendingLogin;

    /// SHA-256 хэш кода, полученный от сервера для верификации
    QString m_pendingCodeHash;
};
