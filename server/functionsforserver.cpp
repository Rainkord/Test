#include "functionsforserver.h"
#include "database.h"
#include "calculator.h"
#include "smtpclient.h"

#include <QDebug>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QtConcurrent/QtConcurrent>

// Static member definitions
QMap<QString, QString>       FunctionsForServer::pendingCodes;
QMap<QString, TempRegData>   FunctionsForServer::pendingRegistrations;
QMap<QString, TempResetData> FunctionsForServer::pendingResets;

// ─── Helpers ────────────────────────────────────────────────────────────────

QString FunctionsForServer::generateCode()
{
    quint32 number = QRandomGenerator::global()->bounded(1000000u);
    return QString("%1").arg(number, 6, 10, QChar('0'));
}

QString FunctionsForServer::hashCode(const QString &code)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(code.toUtf8(), QCryptographicHash::Sha256).toHex());
}

// ─── Main dispatcher ────────────────────────────────────────────────────────

QString FunctionsForServer::processMessage(const QString &message)
{
    QStringList parts = message.split("||");
    if (parts.isEmpty()) return "error||empty_message";
    QString command = parts[0].trimmed();
    qDebug() << "[Server] Command:" << command;

    if (command == "check_login")          return handleCheckLogin(parts);
    if (command == "registration")         return handleRegistration(parts);
    if (command == "registration_confirm") return handleRegistrationConfirm(parts);
    if (command == "auth")                 return handleAuth(parts);
    if (command == "get_graph")            return handleGetGraph(parts);
    if (command == "get_task")             return handleGetTask();
    if (command == "reset_password")       return handleResetPassword(parts);
    if (command == "set_new_password")     return handleSetNewPassword(parts);

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

// ─── registration||login||passwordHash||email ────────────────────────────────
//
// Сервер генерирует код и НЕМЕДЛЕННО возвращает его SHA-256 хэш клиенту.
// Письмо уходит в фоне. Клиент сравнивает хэши локально.
// После успеха клиент отправляет registration_confirm.

QString FunctionsForServer::handleRegistration(const QStringList &parts)
{
    if (parts.size() < 4) return "error||invalid_params";
    QString login        = parts[1].trimmed();
    QString passwordHash = parts[2].trimmed();
    QString email        = parts[3].trimmed();
    if (login.isEmpty() || passwordHash.isEmpty() || email.isEmpty())
        return "error||invalid_params";

    if (Database::instance().userExists(login))  return "reg-||user_exists";
    if (Database::instance().emailExists(email)) return "reg-||email_exists";

    QString code     = generateCode();
    QString codeHash = hashCode(code);

    TempRegData data;
    data.name         = login;
    data.passwordHash = passwordHash;
    data.email        = email;
    data.code         = code;
    pendingRegistrations[login] = data;
    qDebug() << "[Server] Reg code for" << login << ":" << code;

    QtConcurrent::run([email, code]() {
        SmtpClient::sendVerificationCode(email, code);
    });

    return QString("reg_code_sent||%1").arg(codeHash);
}

// ─── registration_confirm||login||passwordHash||email ────────────────────────
//
// Вызывается клиентом только после успешного локального сравнения хэшей.

QString FunctionsForServer::handleRegistrationConfirm(const QStringList &parts)
{
    if (parts.size() < 4) return "error||invalid_params";
    QString login        = parts[1].trimmed();
    QString passwordHash = parts[2].trimmed();
    QString email        = parts[3].trimmed();

    if (!pendingRegistrations.contains(login))
        return "reg-||session_expired";

    if (Database::instance().userExists(login)) {
        pendingRegistrations.remove(login);
        return "reg-||user_exists";
    }

    bool ok = Database::instance().addUser(login, passwordHash, email);
    pendingRegistrations.remove(login);
    if (!ok) return "reg-||db_error";

    qDebug() << "[Server] User registered:" << login;
    return "reg+||" + login;
}

// ─── auth||login||passwordHash ───────────────────────────────────────────────
//
// Сервер генерирует код и НЕМЕДЛЕННО возвращает его SHA-256 хэш клиенту.
// Письмо уходит в фоне. Клиент сравнивает хэши локально.

QString FunctionsForServer::handleAuth(const QStringList &parts)
{
    if (parts.size() < 3) return "error||invalid_params";
    QString login        = parts[1].trimmed();
    QString passwordHash = parts[2].trimmed();

    if (!Database::instance().checkUser(login, passwordHash)) {
        qDebug() << "[Server] Auth failed for:" << login;
        return "auth-";
    }

    QString code     = generateCode();
    QString codeHash = hashCode(code);
    pendingCodes[login] = code;
    qDebug() << "[Server] Auth code for" << login << ":" << code;

    QString email = Database::instance().getUserEmail(login);
    if (!email.isEmpty()) {
        QtConcurrent::run([email, code]() {
            SmtpClient::sendVerificationCode(email, code);
        });
    } else {
        qDebug() << "[Server] Could not find email for:" << login;
    }

    return QString("auth_code_sent||%1").arg(codeHash);
}

// ─── get_graph||xMin||xMax||step||a||b||c ───────────────────────────────────

QString FunctionsForServer::handleGetGraph(const QStringList &parts)
{
    if (parts.size() < 7) return "error||invalid_params";

    bool okXMin, okXMax, okStep, okA, okB, okC;
    double xMin = parts[1].toDouble(&okXMin);
    double xMax = parts[2].toDouble(&okXMax);
    double step = parts[3].toDouble(&okStep);
    double a    = parts[4].toDouble(&okA);
    double b    = parts[5].toDouble(&okB);
    double c    = parts[6].toDouble(&okC);

    if (!okXMin || !okXMax || !okStep || !okA || !okB || !okC)
        return "error||invalid_number_format";
    if (xMin >= xMax)  return "error||xMin_must_be_less_than_xMax";
    if (step <= 0.0)   return "error||step_must_be_positive";

    return Calculator::generateGraphData(xMin, xMax, step, a, b, c);
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
//
// Сервер генерирует код и НЕМЕДЛЕННО возвращает его SHA-256 хэш клиенту.

QString FunctionsForServer::handleResetPassword(const QStringList &parts)
{
    if (parts.size() < 2) return "error||invalid_params";
    QString email = parts[1].trimmed();
    if (email.isEmpty()) return "error||invalid_params";

    if (!Database::instance().emailExists(email)) {
        qDebug() << "[Server] reset_password: email not found:" << email;
        return "reset_error";
    }

    QString code     = generateCode();
    QString codeHash = hashCode(code);

    TempResetData data;
    data.email = email;
    data.code  = code;
    pendingResets[email] = data;
    qDebug() << "[Server] Reset code for" << email << ":" << code;

    QString login = Database::instance().getLoginByEmail(email);
    QtConcurrent::run([email, login, code]() {
        SmtpClient::sendPasswordResetCode(email, login, code);
    });

    return QString("reset_code_sent||%1").arg(codeHash);
}

// ─── set_new_password||email||newPasswordHash ────────────────────────────────
//
// Код уже проверен на клиенте локально — сервер только обновляет пароль.

QString FunctionsForServer::handleSetNewPassword(const QStringList &parts)
{
    if (parts.size() < 3) return "error||invalid_params";
    QString email        = parts[1].trimmed();
    QString passwordHash = parts[2].trimmed();
    if (email.isEmpty() || passwordHash.isEmpty()) return "error||invalid_params";

    if (!pendingResets.contains(email))
        return "reset_error||session_expired";

    bool ok = Database::instance().updatePassword(email, passwordHash);
    pendingResets.remove(email);
    if (!ok) return "reset_error||db_error";

    qDebug() << "[Server] Password updated for" << email;
    return "password_changed";
}
