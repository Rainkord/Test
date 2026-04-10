#include "database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Database &Database::instance()
{
    static Database inst;
    return inst;
}

bool Database::connect(const QString &dbName)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbName);

    if (!m_db.open()) {
        qDebug() << "[DB] Failed to open database:" << m_db.lastError().text();
        return false;
    }

    qDebug() << "[DB] Database opened:" << dbName;
    return createTable();
}

bool Database::createTable()
{
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
        return false;
    }

    qDebug() << "[DB] Table 'users' ready";
    return true;
}

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

QString Database::getUserEmail(const QString &name)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT email FROM users WHERE name = :name");
    query.bindValue(":name", name);

    if (!query.exec()) {
        qDebug() << "[DB] getUserEmail query failed:" << query.lastError().text();
        return QString();
    }

    if (query.next()) {
        return query.value(0).toString();
    }

    return QString();
}

// ─── NEW: check if any user has this email ────────────────────────────────────

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

// ─── NEW: update password hash by email ──────────────────────────────────────

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
