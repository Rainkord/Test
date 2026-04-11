#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QString>

/**
 * @file smtpclient.h
 * @brief Утилитный класс для отправки email через SMTP (Gmail).
 */

/**
 * @class SmtpClient
 * @brief Отправляет письма с кодами подтверждения через SMTP-сервер Gmail.
 *
 * Учётные данные (логин и пароль приложения) считываются из файла
 * @c email.txt в рабочей директории сервера.
 * Все методы статические — создание объекта не требуется.
 */
class SmtpClient
{
public:
    /**
     * @brief Отправляет письмо с кодом подтверждения регистрации.
     * @param toEmail Email получателя.
     * @param code    Шестизначный код подтверждения.
     * @return @c true если письмо отправлено успешно, иначе @c false.
     */
    static bool sendVerificationCode(const QString &toEmail, const QString &code);

    /**
     * @brief Отправляет письмо с кодом сброса пароля.
     * @param toEmail Email получателя.
     * @param login   Логин пользователя (для указания в теле письма).
     * @param code    Шестизначный код подтверждения.
     * @return @c true если письмо отправлено успешно, иначе @c false.
     */
    static bool sendPasswordResetCode(const QString &toEmail, const QString &login, const QString &code);
};

#endif // SMTPCLIENT_H
