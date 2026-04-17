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
 * Все публичные методы и поля статические — создание объекта не требуется.
 *
 * Поддерживаемые команды:
 * | Команда           | Параметры                          | Ответ при успехе     |
 * |-------------------|------------------------------------|----------------------|
 * | check_login       | login                              | login_free / login_taken |
 * | registration      | login, hash, email                 | reg_code_sent        |
 * | verify_reg        | login, code                        | reg+||login          |
 * | auth              | login, hash                        | auth_code_sent       |
 * | verify_auth       | login, code                        | auth+||login         |
 * | get_graph         | xMin, xMax, step, a, b, c          | graph||x;y||...      |
 * | get_task          | —                                  | task||название||описание |
 * | reset_password    | email                              | reset_code_sent      |
 * | verify_reset      | email, code                        | reset_code_ok        |
 * | set_new_password  | email, code, hash                  | password_changed     |
 */
class FunctionsForServer
{
public:
    /**
     * @brief Разбирает входящее сообщение и возвращает строку-ответ клиенту.
     *
     * Является единственной точкой входа для обработки команд.
     * Внутри вызывает приватные handleXxx-методы в зависимости от команды.
     *
     * @param message Строка команды от клиента (формат: @c "команда||param1||param2").
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
     * @brief Генерирует случайный шестизначный код подтверждения.
     * @return Строка вида "042731" (всегда 6 цифр, с ведущими нулями).
     */
    static QString generateCode();

    /** @brief Обрабатывает команду @c check_login||login. */
    static QString handleCheckLogin(const QStringList &parts);

    /** @brief Обрабатывает команду @c registration||login||hash||email. */
    static QString handleRegistration(const QStringList &parts);

    /** @brief Обрабатывает команду @c verify_reg||login||code. */
    static QString handleVerifyReg(const QStringList &parts);

    /** @brief Обрабатывает команду @c auth||login||hash. */
    static QString handleAuth(const QStringList &parts);

    /** @brief Обрабатывает команду @c verify_auth||login||code. */
    static QString handleVerifyAuth(const QStringList &parts);

    /** @brief Обрабатывает команду @c get_graph||xMin||xMax||step||a||b||c. */
    static QString handleGetGraph(const QStringList &parts);

    /** @brief Обрабатывает команду @c get_task (без параметров). */
    static QString handleGetTask();

    /** @brief Обрабатывает команду @c reset_password||email. */
    static QString handleResetPassword(const QStringList &parts);

    /** @brief Обрабатывает команду @c verify_reset||email||code. */
    static QString handleVerifyReset(const QStringList &parts);

    /** @brief Обрабатывает команду @c set_new_password||email||code||hash. */
    static QString handleSetNewPassword(const QStringList &parts);
};

#endif // FUNCTIONSFORSERVER_H
