#include "functionsforserver.h"
#include "database.h"
#include "smtpclient.h"

#include <QRandomGenerator>
#include <QCryptographicHash>
#include <QMap>
#include <QMutex>
#include <QtConcurrent/QtConcurrent>

// ── Thread-safe pending codes storage ────────────────────────────────────────
namespace {
    QMap<QString, QString> pendingCodes;   // key -> sha256(code)
    QMutex                 pendingMutex;

    QString generateCode()
    {
        quint32 n = QRandomGenerator::global()->bounded(100000, 999999);
        return QString::number(n);
    }

    QString sha256(const QString &s)
    {
        return QString::fromLatin1(
            QCryptographicHash::hash(s.toUtf8(), QCryptographicHash::Sha256).toHex());
    }
}

// ── handleAuth ────────────────────────────────────────────────────────────────
QString FunctionsForServer::handleAuth(const QStringList &parts)
{
    // parts: ["auth", login, passwordHash]
    if (parts.size() < 3)
        return "auth-";

    const QString &login    = parts[1];
    const QString &passHash = parts[2];

    if (!Database::instance().checkUser(login, passHash))
        return "auth-";

    // Generate 6-digit code, store its hash
    QString code     = generateCode();
    QString codeHash = sha256(code);

    {
        QMutexLocker locker(&pendingMutex);
        pendingCodes[login] = codeHash;
    }

    // Send email asynchronously
    QString email = Database::instance().getUserEmail(login);
    QtConcurrent::run([email, code]() {
        SmtpClient::sendVerificationCode(email, code);
    });

    return QString("auth_code_sent||%1").arg(codeHash);
}

// ── handleCheckLogin ──────────────────────────────────────────────────────────
QString FunctionsForServer::handleCheckLogin(const QStringList &parts)
{
    if (parts.size() < 2)
        return "check_login_error";

    const QString &login = parts[1];
    bool exists = Database::instance().userExists(login);
    return exists ? "login_taken" : "login_free";
}

// ── handleRegistration ────────────────────────────────────────────────────────
QString FunctionsForServer::handleRegistration(const QStringList &parts)
{
    // parts: ["registration", login, passwordHash, email]
    if (parts.size() < 4)
        return "reg-";

    const QString &login    = parts[1];
    const QString &passHash = parts[2];
    const QString &email    = parts[3];

    if (Database::instance().userExists(login))
        return "login_taken";
    if (Database::instance().emailExists(email))
        return "email_taken";

    QString code     = generateCode();
    QString codeHash = sha256(code);

    {
        QMutexLocker locker(&pendingMutex);
        // Composite key so reg codes don’t clash with auth codes
        pendingCodes["reg:" + login]      = codeHash;
        pendingCodes["reg_data:" + login] = passHash + "||" + email;
    }

    QtConcurrent::run([email, code]() {
        SmtpClient::sendVerificationCode(email, code);
    });

    return QString("reg_code_sent||%1").arg(codeHash);
}

// ── handleRegistrationConfirm ─────────────────────────────────────────────────
QString FunctionsForServer::handleRegistrationConfirm(const QStringList &parts)
{
    // parts: ["registration_confirm", login]
    // Code already verified on client side; just create the user.
    if (parts.size() < 2)
        return "reg-";

    const QString &login = parts[1];

    QString passHash, email;
    {
        QMutexLocker locker(&pendingMutex);
        if (!pendingCodes.contains("reg_data:" + login))
            return "reg-";
        QString data = pendingCodes.take("reg_data:" + login);
        pendingCodes.remove("reg:" + login);
        QStringList d = data.split("||");
        if (d.size() < 2) return "reg-";
        passHash = d[0];
        email    = d[1];
    }

    bool ok = Database::instance().addUser(login, passHash, email);
    return ok ? "reg+" : "reg-";
}

// ── handleResetPassword ───────────────────────────────────────────────────────
QString FunctionsForServer::handleResetPassword(const QStringList &parts)
{
    // parts: ["reset_password", email]
    if (parts.size() < 2)
        return "reset-";

    const QString &email = parts[1];

    if (!Database::instance().emailExists(email))
        return "reset-";

    QString login    = Database::instance().getLoginByEmail(email);
    QString code     = generateCode();
    QString codeHash = sha256(code);

    {
        QMutexLocker locker(&pendingMutex);
        pendingCodes["reset:" + email] = codeHash;
    }

    QtConcurrent::run([email, login, code]() {
        SmtpClient::sendPasswordResetCode(email, login, code);
    });

    return QString("reset_code_sent||%1").arg(codeHash);
}

// ── handleSetNewPassword ──────────────────────────────────────────────────────
QString FunctionsForServer::handleSetNewPassword(const QStringList &parts)
{
    // parts: ["set_new_password", email, newPasswordHash]
    if (parts.size() < 3)
        return "set_password-";

    const QString &email        = parts[1];
    const QString &passwordHash = parts[2];

    bool ok = Database::instance().updatePasswordByEmail(email, passwordHash);
    return ok ? "set_password+" : "set_password-";
}
