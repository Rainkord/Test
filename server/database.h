#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QSqlDatabase>

class Database
{
public:
    static Database &instance();

    bool connect(const QString &dbName);

    bool    addUser(const QString &name, const QString &passwordHash, const QString &email);
    bool    checkUser(const QString &name, const QString &passwordHash);
    bool    userExists(const QString &name);
    bool    emailExists(const QString &email);
    QString getUserEmail(const QString &name);
    QString getLoginByEmail(const QString &email);
    bool    updatePasswordByEmail(const QString &email, const QString &newHash);

private:
    Database() = default;
    bool createTable();
    QSqlDatabase m_db;
};

#endif // DATABASE_H
