#include "functionsforserver.h"
#include "database.h"
#include "calculator.h"
#include "smtpclient.h"

#include <QDebug>
#include <QRandomGenerator>

// Static member definitions
QMap<QString, QString>     FunctionsForServer::pendingCodes;
QMap<QString, TempRegData> FunctionsForServer::pendingRegistrations;
QMap<QString, TempResetData> FunctionsForServer::pendingResets;

// ─── Helpers ────────────────────────────────────────────────────────────────

QString FunctionsForServer::generateCode()
{
    quint32 number = QRandomGenerator::global()->bounded(1000000u);
    return QString("%1").arg(number, 6, 10, QChar('0'));
}

// ─── Main dispatcher ────────────────────────────────────────────────────────

QString FunctionsForServer::processMessage(const QString &message)
{
    QStringList parts = message.split("||");
    if (parts.isEmpty()) {
        return "error||empty_message";
    }

    QString command = parts[0].trimmed();
    qDebug() << "[Server] Command received:" << command;

    if (command == "check_login") {
        return handleCheckLogin(parts);
    } else if (command == "registration") {
        return handleRegistration(parts);
    } else if (command == "verify_reg") {
        return handleVerifyReg(parts);
    } else if (command == "auth") {
        return handleAuth(parts);
    } else if (command == "verify_auth") {
        return handleVerifyAuth(parts);
    } else if (command == "get_graph") {
        return handleGetGraph(parts);
    } else if (command == "get_task") {
        return handleGetTask();
    } else if (command == "reset_password") {
        return handleResetPassword(parts);
    } else if (command == "verify_reset") {
        return handleVerifyReset(parts);
    } else if (command == "set_new_password") {
        return handleSetNewPassword(parts);
    }

    return "error||unknown_command";
}

// ─── check_login||login ──────────────────────────────────────────────────────

QString FunctionsForServer::handleCheckLogin(const QStringList &parts)
{
    if (parts.size() < 2) return "error||invalid_params";
    QString login = parts[1].trimmed();
    if (login.isEmpty()) return "error||invalid_params";

    if (Database::instance().userExists(login)) {
        qDebug() << "[Server] check_login: taken:" << login;
        return "login_taken";
    }

    qDebug() << "[Server] check_login: free:" << login;
    return "login_free";
}

// ─── registration||login||password_hash||email ──────────────────────────────

QString FunctionsForServer::handleRegistration(const QStringList &parts)
{
    if (parts.size() < 4) {
        return "error||invalid_params";
    }

    QString login        = parts[1].trimmed();
    QString passwordHash = parts[2].trimmed();
    QString email        = parts[3].trimmed();

    if (login.isEmpty() || passwordHash.isEmpty() || email.isEmpty()) {
        return "error||invalid_params";
    }

    if (Database::instance().userExists(login)) {
        return "reg-||user_exists";
    }

    if (Database::instance().emailExists(email)) {
        return "reg-||email_exists";
    }

    QString code = generateCode();

    TempRegData data;
    data.name         = login;
    data.passwordHash = passwordHash;
    data.email        = email;
    data.code         = code;
    pendingRegistrations[login] = data;

    qDebug() << "[Server] Registration code for" << login << ":" << code;

    bool sent = SmtpClient::sendVerificationCode(email, code);
    if (!sent) {
        qDebug() << "[Server] Failed to send email to" << email;
    }

    return "reg_code_sent";
}

// ─── verify_reg||login||code ─────────────────────────────────────────────────

QString FunctionsForServer::handleVerifyReg(const QStringList &parts)
{
    if (parts.size() < 3) {
        return "error||invalid_params";
    }

    QString login = parts[1].trimmed();
    QString code  = parts[2].trimmed();

    if (!pendingRegistrations.contains(login)) {
        return "reg-||no_pending_registration";
    }

    TempRegData data = pendingRegistrations[login];

    if (data.code != code) {
        return "reg-||wrong_code";
    }

    bool ok = Database::instance().addUser(data.name, data.passwordHash, data.email);
    if (!ok) {
        pendingRegistrations.remove(login);
        return "reg-||db_error";
    }

    pendingRegistrations.remove(login);
    qDebug() << "[Server] User registered successfully:" << login;
    return "reg+||" + login;
}

// ─── auth||login||password_hash ──────────────────────────────────────────────

QString FunctionsForServer::handleAuth(const QStringList &parts)
{
    if (parts.size() < 3) {
        return "error||invalid_params";
    }

    QString login        = parts[1].trimmed();
    QString passwordHash = parts[2].trimmed();

    if (!Database::instance().checkUser(login, passwordHash)) {
        qDebug() << "[Server] Auth failed for:" << login;
        return "auth-";
    }

    QString code = generateCode();
    pendingCodes[login] = code;

    qDebug() << "[Server] Auth code for" << login << ":" << code;

    QString email = Database::instance().getUserEmail(login);
    if (!email.isEmpty()) {
        SmtpClient::sendVerificationCode(email, code);
    } else {
        qDebug() << "[Server] Could not find email for user:" << login;
    }

    return "auth_code_sent";
}

// ─── verify_auth||login||code ────────────────────────────────────────────────

QString FunctionsForServer::handleVerifyAuth(const QStringList &parts)
{
    if (parts.size() < 3) {
        return "error||invalid_params";
    }

    QString login = parts[1].trimmed();
    QString code  = parts[2].trimmed();

    if (!pendingCodes.contains(login)) {
        return "auth-||no_pending_auth";
    }

    if (pendingCodes[login] != code) {
        return "auth-||wrong_code";
    }

    pendingCodes.remove(login);
    qDebug() << "[Server] User authenticated successfully:" << login;
    return "auth+||" + login;
}

// ─── get_graph||xMin||xMax||step||a||b||c ────────────────────────────────────

QString FunctionsForServer::handleGetGraph(const QStringList &parts)
{
    if (parts.size() < 7) {
        return "error||invalid_params";
    }

    bool okXMin, okXMax, okStep, okA, okB, okC;
    double xMin = parts[1].toDouble(&okXMin);
    double xMax = parts[2].toDouble(&okXMax);
    double step = parts[3].toDouble(&okStep);
    double a    = parts[4].toDouble(&okA);
    double b    = parts[5].toDouble(&okB);
    double c    = parts[6].toDouble(&okC);

    if (!okXMin || !okXMax || !okStep || !okA || !okB || !okC) {
        return "error||invalid_number_format";
    }

    if (xMin >= xMax) {
        return "error||xMin_must_be_less_than_xMax";
    }

    if (step <= 0.0) {
        return "error||step_must_be_positive";
    }

    QString graphData = Calculator::generateGraphData(xMin, xMax, step, a, b, c);
    return graphData;
}

// ─── get_task ────────────────────────────────────────────────────────────────

QString FunctionsForServer::handleGetTask()
{
    return QString::fromUtf8(
        "task||"
        "Графическое отображение ветвящейся функции в рамках клиент-серверного проекта||"
        "Функция №9: f(x) = |x*a|-2 при x<-2; b*(x^2)+x+1 при -2<=x<2; |x-2|+1*c при x>=2"
    );
}

// ─── reset_password||email ───────────────────────────────────────────────────

QString FunctionsForServer::handleResetPassword(const QStringList &parts)
{
    if (parts.size() < 2) {
        return "error||invalid_params";
    }

    QString email = parts[1].trimmed();
    if (email.isEmpty()) {
        return "error||invalid_params";
    }

    if (!Database::instance().emailExists(email)) {
        qDebug() << "[Server] reset_password: email not found:" << email;
        return "reset_error";
    }

    QString login = Database::instance().getLoginByEmail(email);

    QString code = generateCode();

    TempResetData data;
    data.email = email;
    data.code  = code;
    pendingResets[email] = data;

    qDebug() << "[Server] Password reset code for" << email << ":" << code;

    bool sent = SmtpClient::sendPasswordResetCode(email, login, code);
    if (!sent) {
        qDebug() << "[Server] Failed to send reset email to" << email;
    }

    return "reset_code_sent";
}

// ─── verify_reset||email||code ───────────────────────────────────────────────

QString FunctionsForServer::handleVerifyReset(const QStringList &parts)
{
    if (parts.size() < 3) {
        return "error||invalid_params";
    }

    QString email = parts[1].trimmed();
    QString code  = parts[2].trimmed();

    if (!pendingResets.contains(email)) {
        return "reset_code_fail";
    }

    if (pendingResets[email].code != code) {
        qDebug() << "[Server] verify_reset: wrong code for" << email;
        return "reset_code_fail";
    }

    qDebug() << "[Server] verify_reset: code correct for" << email;
    return "reset_code_ok";
}

// ─── set_new_password||email||code||hash ─────────────────────────────────────

QString FunctionsForServer::handleSetNewPassword(const QStringList &parts)
{
    if (parts.size() < 4) {
        return "error||invalid_params";
    }

    QString email        = parts[1].trimmed();
    QString code         = parts[2].trimmed();
    QString passwordHash = parts[3].trimmed();

    if (!pendingResets.contains(email) || pendingResets[email].code != code) {
        qDebug() << "[Server] set_new_password: invalid or expired code for" << email;
        return "reset_error";
    }

    bool ok = Database::instance().updatePasswordByEmail(email, passwordHash);
    if (!ok) {
        qDebug() << "[Server] set_new_password: DB update failed for" << email;
        return "reset_error";
    }

    pendingResets.remove(email);
    qDebug() << "[Server] Password changed successfully for" << email;
    return "password_changed";
}
