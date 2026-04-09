#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QSqlDatabase>

class Database
{
public:
    // Returns the single instance of Database
    static Database &instance();

    bool connect(const QString &dbName);
    bool addUser(const QString &login, const QString &password, const QString &email);
    bool checkUser(const QString &login, const QString &password);
    bool userExists(const QString &login);

private:
    Database();
    ~Database();

    // Disable copy and assignment
    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    QSqlDatabase mDb;
};

#endif // DATABASE_H
