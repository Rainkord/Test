#ifndef FUNCTIONSFORSERVER_H
#define FUNCTIONSFORSERVER_H

#include <QString>
#include <QMap>

/**
 * @file functionsforserver.h
 * @brief Парсер входящих команд и обработчики запросов клиента.
 */

/**
 * @struct TempRegData
 * @brief Временные данные регистрации, ожидающие подтверждения по email.
 */
struct TempRegData {
    QString name;         ///< Логин пользователя.
    QString passwordHash; ///< Хэш пароля (SHA-256).
    QString email;        ///< Email-адрес пользователя.
    QString code;         ///< Шестизначный код подтверждения.
};

/**
 * @struct TempResetData
 * @brief Временные данные сброса пароля, ожидающие подтверждения по email.
 */
struct TempResetData {
    QString email; ///< Email-адрес пользователя.
    QString code;  ///< Шестизначный код подтверждения.
};

/**
 * @class FunctionsForServer
 * @brief Статический класс-диспетчер: разбирает входящее сообщение от клиента
 *        и маршрутизирует его к соответствующему обработчику.
 *
 * Протокол команд использует разделитель @c "||".
 *
 * ## Архитектура верификации (клиентская)
 *
 * Для всех операций с кодом подтверждения (вход, регистрация, сброс пароля)
 * используется единая схема:
 *  1. Сервер генерирует код и немедленно возвращает его SHA-256 хэш клиенту.
 *  2. Письмо с кодом отправляется на почту в фоне (QtConcurrent).
 *  3. Клиент хэширует введённый пользователем код и сравнивает хэши ЛОКАЛЬНО.
 *  4. Сетевой запрос для проверки кода НЕ выполняется.
 *
 * Поддерживаемые команды:
 * | Команда                | Параметры                  | Ответ при успехе              |
 * |------------------------|----------------------------|-------------------------------|
 * | check_login            | login                      | login_free / login_taken      |
 * | registration           | login, hash, email         | reg_code_sent||codeHash       |
 * | registration_confirm   | login, passwordHash, email | reg+||login                   |
 * | auth                   | login, hash                | auth_code_sent||codeHash      |
 * | get_graph              | xMin, xMax, step, a, b, c  | graph||x;y||...               |
 * | get_task               | —                          | task||название||описание      |
 * | reset_password         | email                      | reset_code_sent||codeHash     |
 * | set_new_password       | email, newPasswordHash     | password_changed              |
 */
class FunctionsForServer
{
public:
    /**
     * @brief Разбирает входящее сообщение и возвращает строку-ответ клиенту.
     * @param message Строка команды (формат: @c "команда||param1||param2").
     * @return Строка ответа для отправки клиенту.
     */
    static QString processMessage(const QString &message);

private:
    /// Ожидающие коды 2FA при авторизации: логин → код.
    static QMap<QString, QString> pendingCodes;

    /// Ожидающие данные регистрации: логин → TempRegData.
    static QMap<QString, TempRegData> pendingRegistrations;

    /// Ожидающие данные сброса пароля: email → TempResetData.
    static QMap<QString, TempResetData> pendingResets;

    /**
     * @brief Генерирует случайный шестизначный код.
     * @return Строка вида "042731" (6 цифр, с ведущими нулями).
     */
    static QString generateCode();

    /**
     * @brief Возвращает SHA-256 хэш строки в hex-формате.
     * @param code Исходная строка.
     * @return Hex-строка SHA-256.
     */
    static QString hashCode(const QString &code);

    static QString handleCheckLogin(const QStringList &parts);
    static QString handleRegistration(const QStringList &parts);
    static QString handleRegistrationConfirm(const QStringList &parts);
    static QString handleAuth(const QStringList &parts);
    static QString handleGetGraph(const QStringList &parts);
    static QString handleGetTask();
    static QString handleResetPassword(const QStringList &parts);
    static QString handleSetNewPassword(const QStringList &parts);
};

#endif // FUNCTIONSFORSERVER_H
