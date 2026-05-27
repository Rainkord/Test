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

/**
 * @brief Генерирует случайный 6-значный код подтверждения
 *
 * Использует QRandomGenerator для генерации числа в диапазоне
 * [100000, 999999] и возвращает его в виде строки.
 *
 * @return строка из 6 цифр
 */
QString FunctionsForServer::generateCode()
{
    quint32 n = QRandomGenerator::global()->bounded(100000u, 1000000u);
    return QString::number(n);
}

/**
 * @brief Вычисляет SHA-256 хеш строки
 *
 * Конвертирует входную строку в UTF-8 и вычисляет её SHA-256 хеш,
 * возвращая результат в шестнадцатеричном представлении.
 *
 * @param s входная строка
 * @return hex-строка SHA-256 хеша
 */
QString FunctionsForServer::hashCode(const QString &s)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(s.toUtf8(), QCryptographicHash::Sha256).toHex());
}

// ── processMessage ───────────────────────────────────────────────────────────

/**
 * @brief Обрабатывает входящее сообщение от клиента
 *
 * Разделяет сообщение по разделителю "||", определяет команду
 * по первому токену и делегирует обработку соответствующему
 * обработчику. Поддерживаемые команды: check_login, registration,
 * registration_confirm, auth, get_graph, get_task, reset_password,
 * set_new_password.
 *
 * @param message строка сообщения от клиента
 * @return строка ответа клиенту; "error" при пустом сообщении;
 *         "unknown_command" при неизвестной команде
 */
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

/**
 * @brief Обработчик команды авторизации
 *
 * Проверяет учётные данные пользователя через Database::checkUser.
 * При успешной проверке генерирует код 2FA, сохраняет его хеш
 * в pendingCodes и асинхронно отправляет код на email через SMTP.
 *
 * @param parts список частей сообщения: ["auth", "логин", "хеш_пароля"]
 * @return "auth_code_sent||хеш_кода" при успехе; "auth-" при ошибке
 */
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

/**
 * @brief Обработчик команды проверки занятости логина
 *
 * Проверяет是否存在 пользователя с указанным логином в базе данных.
 *
 * @param parts список частей сообщения: ["check_login", "логин"]
 * @return "login_taken" если логин занят; "login_free" если свободен;
 *         "check_login_error" при некорректном формате
 */
QString FunctionsForServer::handleCheckLogin(const QStringList &parts)
{
    if (parts.size() < 2) return "check_login_error";
    bool exists = Database::instance().userExists(parts[1]);
    return exists ? "login_taken" : "login_free";
}

// ── handleRegistration ────────────────────────────────────────────────────────

/**
 * @brief Обработчик команды регистрации нового пользователя
 *
 * Проверяет уникальность логина и email, генерирует код подтверждения,
 * сохраняет временные данные регистрации в pendingRegistrations и
 * асинхронно отправляет код на email через SMTP.
 *
 * @param parts список частей сообщения: ["registration", "логин", "хеш_пароля", "email"]
 * @return "reg_code_sent||хеш_кода" при успехе; "login_taken" если логин занят;
 *         "email_taken" если email занят; "reg-" при ошибке
 */
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

/**
 * @brief Обработчик подтверждения регистрации
 *
 * Извлекает временные данные регистрации из pendingRegistrations
 * и добавляет пользователя в базу данных через Database::addUser.
 *
 * @param parts список частей сообщения: ["registration_confirm", "логин"]
 * @return "reg+||логин" при успешной регистрации; "reg-" при ошибке
 */
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

/**
 * @brief Обработчик команды получения графика
 *
 * Заглушка — возвращает ошибку.
 *
 * @param parts список частей сообщения
 * @return "graph_error"
 */
QString FunctionsForServer::handleGetGraph(const QStringList &parts)
{
    Q_UNUSED(parts)
    return "graph_error";
}

// ── handleGetTask ──────────────────────────────────────────────────────────────

/**
 * @brief Обработчик команды получения задания
 *
 * Заглушка — возвращает ошибку.
 *
 * @return "task_error"
 */
QString FunctionsForServer::handleGetTask()
{
    return "task_error";
}

// ── handleResetPassword ───────────────────────────────────────────────────────

/**
 * @brief Обработчик команды сброса пароля
 *
 * Проверяет существование email в базе данных, генерирует код
 * подтверждения, сохраняет временные данные сброса в pendingResets
 * и асинхронно отправляет код на email через SMTP.
 *
 * @param parts список частей сообщения: ["reset_password", "email"]
 * @return "reset_code_sent||хеш_кода" при успехе; "reset-" при ошибке
 */
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

/**
 * @brief Обработчик установки нового пароля
 *
 * Удаляет временные данные сброса из pendingResets и обновляет
 * хеш пароля в базе данных через Database::updatePasswordByEmail.
 *
 * @param parts список частей сообщения: ["set_new_password", "email", "новый_хеш"]
 * @return "set_password+" при успехе; "set_password-" при ошибке
 */
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
