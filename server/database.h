#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QSqlDatabase>

/**
 * @file database.h
 * @brief Singleton-класс для работы с базой данных пользователей.
 */

/**
 * @class Database
 * @brief Предоставляет интерфейс к SQLite-базе данных через паттерн Singleton.
 *
 * Все методы класса выполняют SQL-запросы к таблице пользователей.
 * Прямое создание объектов класса запрещено — доступ только через instance().
 */
class Database
{
public:
    /**
     * @brief Возвращает единственный экземпляр класса Database.
     * @return Ссылка на экземпляр Database.
     */
    static Database& instance();

    /**
     * @brief Проверяет, существует ли пользователь с данным логином.
     * @param login Логин пользователя.
     * @return @c true если пользователь найден, иначе @c false.
     */
    bool userExists(const QString &login);

    /**
     * @brief Проверяет, зарегистрирован ли указанный email.
     * @param email Email-адрес.
     * @return @c true если email уже занят, иначе @c false.
     */
    bool emailExists(const QString &email);

    /**
     * @brief Добавляет нового пользователя в базу данных.
     * @param login        Логин пользователя.
     * @param passwordHash Хэш пароля (SHA-256).
     * @param email        Email-адрес пользователя.
     * @return @c true при успешном добавлении, @c false при ошибке.
     */
    bool addUser(const QString &login, const QString &passwordHash, const QString &email);

    /**
     * @brief Проверяет пару логин/хэш пароля.
     * @param login        Логин пользователя.
     * @param passwordHash Хэш пароля.
     * @return @c true если пара совпадает, иначе @c false.
     */
    bool checkUser(const QString &login, const QString &passwordHash);

    /**
     * @brief Возвращает email пользователя по логину.
     * @param login Логин пользователя.
     * @return Email-адрес или пустую строку если пользователь не найден.
     */
    QString getUserEmail(const QString &login);

    /**
     * @brief Возвращает логин пользователя по email.
     * @param email Email-адрес.
     * @return Логин или пустую строку если пользователь не найден.
     */
    QString getLoginByEmail(const QString &email);

    /**
     * @brief Обновляет хэш пароля пользователя, найденного по email.
     * @param email           Email-адрес пользователя.
     * @param newPasswordHash Новый хэш пароля.
     * @return @c true при успешном обновлении, @c false при ошибке.
     */
    bool updatePasswordByEmail(const QString &email, const QString &newPasswordHash);

private:
    /** @brief Закрытый конструктор (Singleton). */
    Database();

    /** @brief Закрытый деструктор (Singleton). */
    ~Database();

    /** @cond INTERNAL */
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    /** @endcond */

    QSqlDatabase m_db; ///< Объект подключения к SQLite.
};

#endif // DATABASE_H
