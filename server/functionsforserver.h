#ifndef FUNCTIONSFORSERVER_H
#define FUNCTIONSFORSERVER_H

#include <QString>
#include <QMap>

// Holds pending registration data until email code is verified
struct TempRegData {
    QString name;
    QString passwordHash;
    QString email;
    QString code;
};

// Holds pending password-reset data
struct TempResetData {
    QString email;
    QString code;
};

class FunctionsForServer
{
public:
    // Main entry point: parse incoming message and return response string
    static QString processMessage(const QString &message);

private:
    // Pending auth codes: login -> code
    static QMap<QString, QString> pendingCodes;

    // Pending registrations: login -> TempRegData
    static QMap<QString, TempRegData> pendingRegistrations;

    // Pending password resets: email -> TempResetData
    static QMap<QString, TempResetData> pendingResets;

    // Generates a 6-digit random code as a zero-padded string
    static QString generateCode();

    // Individual command handlers
    static QString handleRegistration(const QStringList &parts);
    static QString handleVerifyReg(const QStringList &parts);
    static QString handleAuth(const QStringList &parts);
    static QString handleVerifyAuth(const QStringList &parts);
    static QString handleGetGraph(const QStringList &parts);
    static QString handleGetTask();

    // Password reset handlers
    static QString handleResetPassword(const QStringList &parts);   // reset_password||email
    static QString handleVerifyReset(const QStringList &parts);     // verify_reset||email||code
    static QString handleSetNewPassword(const QStringList &parts);  // set_new_password||email||code||hash
};

#endif // FUNCTIONSFORSERVER_H
