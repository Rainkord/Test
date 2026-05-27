#include "database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

/**
 * @brief Получает единственный экземпляр класса Database
 *
 * Статическая переменная inst создаётся один раз при первом вызове
 * и уничтожается при завершении программы.
 *
 * @return ссылка на единственный экземпляр Database
 */
Database &Database::instance()
{
    static Database inst;
    return inst;
}

/**
 * @brief Приватный конструктор (Singleton)
 *
 * Открывает (или создаёт) SQLite базу данных users.db.
 * Создаёт таблицу users с полями id, name, email, password_hash,
 * если она ещё не существует.
 */
Database::Database()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("users.db");

    if (!m_db.open()) {
        qDebug() << "[DB] Failed to open database:" << m_db.lastError().text();
        return;
    }

    qDebug() << "[DB] Database opened: users.db";

    QSqlQuery query(m_db);
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT UNIQUE NOT NULL,"
        "email TEXT NOT NULL,"
        "password_hash TEXT NOT NULL"
        ")"
    );

    if (!ok) {
        qDebug() << "[DB] Failed to create table:" << query.lastError().text();
    } else {
        qDebug() << "[DB] Table 'users' ready";
    }
}

/**
 * @brief Приватный деструктор
 *
 * Закрывает соединение с базой данных, если оно открыто.
 */
Database::~Database()
{
    if (m_db.isOpen())
        m_db.close();
}

/**
 * @brief Добавляет нового пользователя в базу данных
 *
 * Вставляет запись в таблицу users с указанными логином, email
 * и хешем пароля. Логин должен быть уникальным.
 *
 * @param name логин нового пользователя
 * @param passwordHash хеш пароля пользователя
 * @param email электронная почта пользователя
 * @return true, если пользователь успешно добавлен; false — при ошибке
 */
bool Database::addUser(const QString &name, const QString &passwordHash, const QString &email)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO users (name, email, password_hash) VALUES (:name, :email, :hash)");
    query.bindValue(":name",  name);
    query.bindValue(":email", email);
    query.bindValue(":hash",  passwordHash);

    if (!query.exec()) {
        qDebug() << "[DB] addUser failed:" << query.lastError().text();
        return false;
    }

    qDebug() << "[DB] User added:" << name;
    return true;
}

/**
 * @brief Проверяет корректность учётных данных пользователя
 *
 * Выполняет SELECT-запрос для поиска записи с указанными логином
 * и хешем пароля.
 *
 * @param name логин пользователя
 * @param passwordHash хеш пароля для проверки
 * @return true, если учётные данные верны; false — иначе
 */
bool Database::checkUser(const QString &name, const QString &passwordHash)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id FROM users WHERE name = :name AND password_hash = :hash");
    query.bindValue(":name", name);
    query.bindValue(":hash", passwordHash);

    if (!query.exec()) {
        qDebug() << "[DB] checkUser query failed:" << query.lastError().text();
        return false;
    }

    return query.next();
}

/**
 * @brief Проверяет существование пользователя по логину
 *
 * @param name логин пользователя для проверки
 * @return true, если пользователь найден; false — иначе
 */
bool Database::userExists(const QString &name)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id FROM users WHERE name = :name");
    query.bindValue(":name", name);

    if (!query.exec()) {
        qDebug() << "[DB] userExists query failed:" << query.lastError().text();
        return false;
    }

    return query.next();
}

/**
 * @brief Получает email пользователя по логину
 *
 * @param name логин пользователя
 * @return email пользователя; пустая строка, если не найден
 */
QString Database::getUserEmail(const QString &name)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT email FROM users WHERE name = :name");
    query.bindValue(":name", name);

    if (!query.exec()) {
        qDebug() << "[DB] getUserEmail query failed:" << query.lastError().text();
        return QString();
    }

    if (query.next())
        return query.value(0).toString();

    return QString();
}

/**
 * @brief Получает логин пользователя по email
 *
 * @param email электронная почта пользователя
 * @return логин пользователя; пустая строка, если не найден
 */
QString Database::getLoginByEmail(const QString &email)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT name FROM users WHERE email = :email");
    query.bindValue(":email", email);

    if (!query.exec()) {
        qDebug() << "[DB] getLoginByEmail query failed:" << query.lastError().text();
        return QString();
    }

    if (query.next())
        return query.value(0).toString();

    return QString();
}

/**
 * @brief Проверяет существование пользователя по email
 *
 * @param email электронная почта для проверки
 * @return true, если пользователь с таким email найден; false — иначе
 */
bool Database::emailExists(const QString &email)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id FROM users WHERE email = :email");
    query.bindValue(":email", email);

    if (!query.exec()) {
        qDebug() << "[DB] emailExists query failed:" << query.lastError().text();
        return false;
    }

    return query.next();
}

/**
 * @brief Обновляет пароль пользователя по email
 *
 * @param email электронная почта пользователя
 * @param newHash новый хеш пароля
 * @return true, если пароль обновлён; false, если email не найден
 *         или произошла ошибка
 */
bool Database::updatePasswordByEmail(const QString &email, const QString &newHash)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET password_hash = :hash WHERE email = :email");
    query.bindValue(":hash",  newHash);
    query.bindValue(":email", email);

    if (!query.exec()) {
        qDebug() << "[DB] updatePasswordByEmail failed:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "[DB] updatePasswordByEmail: no rows updated for" << email;
        return false;
    }

    qDebug() << "[DB] Password updated for email:" << email;
    return true;
}
