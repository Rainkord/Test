#ifndef FUNCTIONSFORSERVER_H
#define FUNCTIONSFORSERVER_H

#include <QString>
#include <QMap>

/**
 * @brief Структура для хранения временных данных регистрации
 *
 * Содержит данные пользователя, который ещё не завершил процесс
 * регистрации (не подтвердил код подтверждения).
 */
struct TempRegData {
    /// Логин пользователя
    QString name;
    /// Хеш пароля пользователя
    QString passwordHash;
    /// Электронная почта пользователя
    QString email;
    /// Хеш кода подтверждения
    QString code;
};

/**
 * @brief Структура для хранения временных данных сброса пароля
 *
 * Содержит данные о процессе сброса пароля до подтверждения кода.
 */
struct TempResetData {
    /// Электронная почта пользователя
    QString email;
    /// Хеш кода подтверждения
    QString code;
};

/**
 * @brief Класс обработки сообщений сервера
 *
 * Центральный класс бизнес-логики сервера. Обрабатывает входящие
 * команды от клиентов (регистрация, авторизация, сброс пароля,
 * получение графика и задания). Управляет временными данными
 * незавершённых операций и взаимодействует с базой данных
 * и SMTP-клиентом.
 */
class FunctionsForServer
{
public:

    /**
     * @brief Обрабатывает входящее сообщение от клиента
     *
     * Парсит сообщение, определяет команду по первому токену
     * и делегирует обработку соответствующему обработчику.
     *
     * @param message строка сообщения от клиента в формате "команда||параметр1||параметр2||..."
     * @return строка ответа клиенту
     */
    static QString processMessage(const QString &message);

private:

    /// Маппинг логин → хеш кода для подтверждения входа (2FA)
    static QMap<QString, QString> pendingCodes;

    /// Маппинг логин → временные данные регистрации
    static QMap<QString, TempRegData> pendingRegistrations;

    /// Маппинг email → временные данные сброса пароля
    static QMap<QString, TempResetData> pendingResets;

    /**
     * @brief Генерирует случайный 6-значный код подтверждения
     *
     * @return строка из 6 цифр (от 100000 до 999999)
     */
    static QString generateCode();

    /**
     * @brief Вычисляет SHA-256 хеш строки
     *
     * @param code исходная строка (код подтверждения)
     * @return хеш-строка в шестнадцатеричном представлении
     */
    static QString hashCode(const QString &code);

    /**
     * @brief Обработчик команды проверки занятости логина
     *
     * @param parts список частей сообщения (check_login||логин)
     * @return "login_taken" или "login_free"
     */
    static QString handleCheckLogin(const QStringList &parts);

    /**
     * @brief Обработчик команды регистрации нового пользователя
     *
     * Генерирует код подтверждения, сохраняет временные данные
     * регистрации и отправляет код на email.
     *
     * @param parts список частей сообщения (registration||логин||хеш_пароля||email)
     * @return "reg_code_sent||хеш_кода" или сообщение об ошибке
     */
    static QString handleRegistration(const QStringList &parts);

    /**
     * @brief Обработчик подтверждения регистрации
     *
     * Добавляет пользователя в базу данных после подтверждения кода.
     *
     * @param parts список частей сообщения (registration_confirm||логин)
     * @return "reg+||логин" при успехе; "reg-" при ошибке
     */
    static QString handleRegistrationConfirm(const QStringList &parts);

    /**
     * @brief Обработчик команды авторизации
     *
     * Проверяет учётные данные, генерирует код 2FA и отправляет
     * его на email пользователя.
     *
     * @param parts список частей сообщения (auth||логин||хеш_пароля)
     * @return "auth_code_sent||хеш_кода" или "auth-" при ошибке
     */
    static QString handleAuth(const QStringList &parts);

    /**
     * @brief Обработчик команды получения графика
     *
     * Парсит параметры (xMin, xMax, step, a, b, c) и вызывает
     * Calculator::generateGraphData для генерации данных графика.
     *
     * @param parts список частей: ["get_graph", xMin, xMax, step, a, b, c]
     * @return строка "graph||x;y||..." или "graph_error"
     */
    static QString handleGetGraph(const QStringList &parts);

    /**
     * @brief Обработчик команды получения задания
     *
     * Возвращает описание задания и формулу кусочной функции.
     *
     * @return строка "task||описание||формула"
     */
    static QString handleGetTask();

    /**
     * @brief Обработчик команды сброса пароля
     *
     * Генерирует код подтверждения, сохраняет временные данные
     * сброса и отправляет код на email.
     *
     * @param parts список частей сообщения (reset_password||email)
     * @return "reset_code_sent||хеш_кода" или "reset-" при ошибке
     */
    static QString handleResetPassword(const QStringList &parts);

    /**
     * @brief Обработчик установки нового пароля
     *
     * Обновляет пароль пользователя в базе данных.
     *
     * @param parts список частей сообщения (set_new_password||email||новый_хеш)
     * @return "set_password+" при успехе; "set_password-" при ошибке
     */
    static QString handleSetNewPassword(const QStringList &parts);
};

#endif // FUNCTIONSFORSERVER_H
