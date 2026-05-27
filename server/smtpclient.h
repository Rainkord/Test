#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QString>

/**
 * @brief Класс SMTP-клиента для отправки email-писем
 *
 * Предоставляет статические методы для отправки писем через
 * SMTP-сервер Gmail (smtp.gmail.com:465) с использованием
 * SSL-шифрования. Учётные данныечитываются из файла email.txt.
 */
class SmtpClient
{
public:

    /// Адрес SMTP-сервера (smtp.gmail.com)
    static const QString smtpHost;

    /// Порт SMTP-сервера (465 — SSL)
    static const int smtpPort;

    /**
     * @brief Получает адрес электронной почты отправителя
     *
     * Считывает значение ключа "email" из файла email.txt.
     *
     * @return ссылка на строку с адресом отправителя
     */
    static const QString &senderEmail();

    /**
     * @brief Получает пароль (App Key) отправителя
     *
     * Считывает значение ключа "key" из файла email.txt.
     *
     * @return ссылка на строку с паролем/ключом отправителя
     */
    static const QString &senderPassword();

    /**
     * @brief Отправляет письмо с кодом подтверждения
     *
     * Формирует и отправляет email с темой "Код подтверждения ТИМП"
     * и текстом, содержащим код подтверждения.
     *
     * @param toEmail адрес получателя
     * @param code код подтверждения (6 цифр)
     * @return true, если письмо успешно отправлено; false — при ошибке
     */
    static bool sendVerificationCode(const QString &toEmail, const QString &code);

    /**
     * @brief Отправляет письмо с кодом сброса пароля
     *
     * Формирует и отправляет email с темой "Восстановление пароля ТИМП"
     * и текстом, содержащим приветствие, код сброса и предупреждение.
     *
     * @param toEmail адрес получателя
     * @param login логин пользователя
     * @param code код сброса пароля (6 цифр)
     * @return true, если письмо успешно отправлено; false — при ошибке
     */
    static bool sendPasswordResetCode(const QString &toEmail, const QString &login, const QString &code);
};

#endif // SMTPCLIENT_H
