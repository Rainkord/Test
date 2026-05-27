#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QSqlDatabase>

/**
 * @brief Класс для работы с базой данных пользователей (Singleton)
 *
 * Реализует паттерн Singleton для обеспечения единственного экземпляра
 * подключения к SQLite базе данных users.db. Предоставляет методы
 * для добавления пользователей, проверки учётных данных, управления
 * электронной почтой и сбросом пароля.
 *
 * @note Конструктор, деструктор и оператор присваивания приватные —
 * создание копий объекта запрещено.
 */
class Database
{
public:

    /**
     * @brief Получает единственный экземпляр класса Database
     *
     * Реализация паттерна Singleton. При первом вызове создаётся
     * экземпляр и инициализируется подключение к БД.
     *
     * @return ссылка на единственный экземпляр Database
     */
    static Database& instance();

    /**
     * @brief Проверяет существование пользователя по логину
     *
     * @param login логин пользователя для проверки
     * @return true, если пользователь с таким логином существует; false — иначе
     */
    bool userExists(const QString &login);

    /**
     * @brief Проверяет существование пользователя по email
     *
     * @param email электронная почта для проверки
     * @return true, если пользователь с таким email существует; false — иначе
     */
    bool emailExists(const QString &email);

    /**
     * @brief Добавляет нового пользователя в базу данных
     *
     * @param login логин нового пользователя (должен быть уникальным)
     * @param passwordHash хеш пароля пользователя
     * @param email электронная почта пользователя
     * @return true, если пользователь успешно добавлен; false — при ошибке
     */
    bool addUser(const QString &login, const QString &passwordHash, const QString &email);

    /**
     * @brief Проверяет корректность учётных данных пользователя
     *
     * @param login логин пользователя
     * @param passwordHash хеш пароля для проверки
     * @return true, если логин и хеш совпадают с записью в БД; false — иначе
     */
    bool checkUser(const QString &login, const QString &passwordHash);

    /**
     * @brief Получает email пользователя по логину
     *
     * @param login логин пользователя
     * @return email пользователя; пустая строка, если пользователь не найден
     */
    QString getUserEmail(const QString &login);

    /**
     * @brief Получает логин пользователя по email
     *
     * @param email электронная почта пользователя
     * @return логин пользователя; пустая строка, если пользователь не найден
     */
    QString getLoginByEmail(const QString &email);

    /**
     * @brief Обновляет пароль пользователя по email
     *
     * @param email электронная почта пользователя
     * @param newPasswordHash новый хеш пароля
     * @return true, если пароль успешно обновлён; false — при ошибке
     *         или если email не найден
     */
    bool updatePasswordByEmail(const QString &email, const QString &newPasswordHash);

private:

    /**
     * @brief Приватный конструктор (Singleton)
     *
     * Открывает базу данных users.db, создаёт таблицу users,
     * если она ещё не существует.
     */
    Database();

    /**
     * @brief Приватный деструктор
     *
     * Закрывает соединение с базой данных при уничтожении объекта.
     */
    ~Database();

    /// Запрет копирования объекта
    Database(const Database&) = delete;
    /// Запрет присваивания объекта
    Database& operator=(const Database&) = delete;

    /// Объект подключения к SQLite базе данных
    QSqlDatabase m_db;
};

#endif // DATABASE_H
