#include "functionsforserver.h"
#include "database.h"
#include "smtpclient.h"
#include "logger.h"

#include <QRandomGenerator>
#include <QCryptographicHash>
#include <QMutex>
#include <QtConcurrent/QtConcurrent>

// ── Static member definitions ────────────────────────────────────────────────────
QMap<QString, QString>       FunctionsForServer::pendingCodes;
QMap<QString, TempRegData>   FunctionsForServer::pendingRegistrations;
QMap<QString, TempResetData> FunctionsForServer::pendingResets;

static QMutex g_mutex;

// ── Helpers ───────────────────────────────────────────────────────────────────
QString FunctionsForServer::generateCode()
{
    quint32 n = QRandomGenerator::global()->bounded(100000u, 1000000u);
    return QString::number(n);
}

QString FunctionsForServer::hashCode(const QString &s)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(s.toUtf8(), QCryptographicHash::Sha256).toHex());
}

// ── processMessage ───────────────────────────────────────────────────────────
QString FunctionsForServer::processMessage(const QString &message)
{
    QStringList parts = message.split("||");
    if (parts.isEmpty())
        return "error";

    const QString &cmd = parts[0];

    if (cmd == "check_login")          return handleCheckLogin(parts);
    if (cmd == "registration")         return handleRegistration(parts);
    if (cmd == "registration_confirm") return handleRegistrationConfirm(parts);
    if (cmd == "auth")                 return handleAuth(parts);
    if (cmd == "get_graph")            return handleGetGraph(parts);
    if (cmd == "get_task")             return handleGetTask();
    if (cmd == "reset_password")       return handleResetPassword(parts);
    if (cmd == "set_new_password")     return handleSetNewPassword(parts);

    return "unknown_command";
}

// ── handleAuth ────────────────────────────────────────────────────────────────
QString FunctionsForServer::handleAuth(const QStringList &parts)
{
    if (parts.size() < 3) return "auth-";

    const QString &login    = parts[1];
    const QString &passHash = parts[2];

    if (!Database::instance().checkUser(login, passHash)) {
        Logger::authFail(login);
        return "auth-";
    }

    Logger::authOk(login);

    QString code     = generateCode();
    QString codeHash = hashCode(code);

    {
        QMutexLocker locker(&g_mutex);
        pendingCodes[login] = codeHash;
    }

    QString email = Database::instance().getUserEmail(login);
    Logger::codeSent(login, email, code);

    QtConcurrent::run([email, code]() {
        bool ok = SmtpClient::sendVerificationCode(email, code);
        if (ok)
            Logger::emailSent(email);
        else
            Logger::emailFailed(email);
    });

    return QString("auth_code_sent||%1").arg(codeHash);
}

// ── handleCheckLogin ──────────────────────────────────────────────────────────
QString FunctionsForServer::handleCheckLogin(const QStringList &parts)
{
    if (parts.size() < 2) return "check_login_error";
    bool exists = Database::instance().userExists(parts[1]);
    return exists ? "login_taken" : "login_free";
}

// ── handleRegistration ────────────────────────────────────────────────────────
QString FunctionsForServer::handleRegistration(const QStringList &parts)
{
    if (parts.size() < 4) return "reg-";

    const QString &login    = parts[1];
    const QString &passHash = parts[2];
    const QString &email    = parts[3];

    if (Database::instance().userExists(login))  return "login_taken";
    if (Database::instance().emailExists(email)) return "email_taken";

    QString code     = generateCode();
    QString codeHash = hashCode(code);

    {
        QMutexLocker locker(&g_mutex);
        TempRegData data;
        data.name         = login;
        data.passwordHash = passHash;
        data.email        = email;
        data.code         = codeHash;
        pendingRegistrations[login] = data;
    }

    Logger::regCode(login, email, code);

    QtConcurrent::run([email, code]() {
        bool ok = SmtpClient::sendVerificationCode(email, code);
        if (ok)
            Logger::emailSent(email);
        else
            Logger::emailFailed(email);
    });

    return QString("reg_code_sent||%1").arg(codeHash);
}

// ── handleRegistrationConfirm ─────────────────────────────────────────────────
QString FunctionsForServer::handleRegistrationConfirm(const QStringList &parts)
{
    if (parts.size() < 2) return "reg-";

    const QString &login = parts[1];

    TempRegData data;
    {
        QMutexLocker locker(&g_mutex);
        if (!pendingRegistrations.contains(login)) return "reg-";
        data = pendingRegistrations.take(login);
    }

    bool ok = Database::instance().addUser(data.name, data.passwordHash, data.email);

    if (ok)
        Logger::regOk(login);
    else
        Logger::regFail(login);

    return ok ? QString("reg+||%1").arg(login) : "reg-";
}

// ── handleGetGraph ─────────────────────────────────────────────────────────────
QString FunctionsForServer::handleGetGraph(const QStringList &parts)
{
    Q_UNUSED(parts)
    return "graph_error";
}

// ── handleGetTask ──────────────────────────────────────────────────────────────
QString FunctionsForServer::handleGetTask()
{
    return "task_error";
}

// ── handleResetPassword ───────────────────────────────────────────────────────
QString FunctionsForServer::handleResetPassword(const QStringList &parts)
{
    if (parts.size() < 2) return "reset-";

    const QString &email = parts[1];
    if (!Database::instance().emailExists(email)) return "reset-";

    QString login    = Database::instance().getLoginByEmail(email);
    QString code     = generateCode();
    QString codeHash = hashCode(code);

    {
        QMutexLocker locker(&g_mutex);
        TempResetData data;
        data.email = email;
        data.code  = codeHash;
        pendingResets[email] = data;
    }

    Logger::resetCode(email, code);

    QtConcurrent::run([email, login, code]() {
        bool ok = SmtpClient::sendPasswordResetCode(email, login, code);
        if (ok)
            Logger::emailSent(email);
        else
            Logger::emailFailed(email);
    });

    return QString("reset_code_sent||%1").arg(codeHash);
}

// ── handleSetNewPassword ──────────────────────────────────────────────────────
QString FunctionsForServer::handleSetNewPassword(const QStringList &parts)
{
    if (parts.size() < 3) return "set_password-";

    const QString &email        = parts[1];
    const QString &passwordHash = parts[2];

    {
        QMutexLocker locker(&g_mutex);
        pendingResets.remove(email);
    }

    bool ok = Database::instance().updatePasswordByEmail(email, passwordHash);

    if (ok)
        Logger::resetOk(email);
    else
        Logger::error(QString("Не удалось обновить пароль для: %1").arg(email));

    return ok ? "set_password+" : "set_password-";
}
