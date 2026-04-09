#include "database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Database &Database::instance()
{
    static Database db;
    return db;
}

Database::Database()
{
    // Constructor is private; connection is established via connect()
}

Database::~Database()
{
    if (mDb.isOpen()) {
        mDb.close();
    }
}

bool Database::connect(const QString &dbName)
{
    mDb = QSqlDatabase::addDatabase("QSQLITE");
    mDb.setDatabaseName(dbName);

    if (!mDb.open()) {
        qDebug() << "Database::connect() error:" << mDb.lastError().text();
        return false;
    }

    // Create the users table if it does not exist
    QSqlQuery query(mDb);
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS users ("
        "  id       INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  login    TEXT    UNIQUE NOT NULL,"
        "  password TEXT    NOT NULL,"
        "  email    TEXT    NOT NULL"
        ");"
    );

    if (!ok) {
        qDebug() << "Database::connect() failed to create table:"
                 << query.lastError().text();
        return false;
    }

    qDebug() << "Database connected to" << dbName;
    return true;
}

bool Database::addUser(const QString &login, const QString &password,
                       const QString &email)
{
    QSqlQuery query(mDb);
    query.prepare("INSERT INTO users (login, password, email) "
                  "VALUES (:login, :password, :email);");
    query.bindValue(":login",    login);
    query.bindValue(":password", password);
    query.bindValue(":email",    email);

    if (!query.exec()) {
        qDebug() << "Database::addUser() error:" << query.lastError().text();
        return false;
    }

    qDebug() << "Database::addUser() added user:" << login;
    return true;
}

bool Database::checkUser(const QString &login, const QString &password)
{
    QSqlQuery query(mDb);
    query.prepare("SELECT id FROM users "
                  "WHERE login = :login AND password = :password;");
    query.bindValue(":login",    login);
    query.bindValue(":password", password);

    if (!query.exec()) {
        qDebug() << "Database::checkUser() error:" << query.lastError().text();
        return false;
    }

    return query.next(); // true if at least one matching record
}

bool Database::userExists(const QString &login)
{
    QSqlQuery query(mDb);
    query.prepare("SELECT id FROM users WHERE login = :login;");
    query.bindValue(":login", login);

    if (!query.exec()) {
        qDebug() << "Database::userExists() error:" << query.lastError().text();
        return false;
    }

    return query.next();
}
