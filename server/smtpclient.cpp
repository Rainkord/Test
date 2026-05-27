#include "smtpclient.h"

#include <QSslSocket>
#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>

/// Адрес SMTP-сервера Gmail
const QString SmtpClient::smtpHost = "smtp.gmail.com";
/// Порт SMTP-сервера Gmail (SSL)
const int     SmtpClient::smtpPort = 465;

// ── Read email.txt next to the server executable ────────────────────────────

/**
 * @brief Считывает значение ключа из файла email.txt
 *
 * Ищет файл email.txt в каталоге исполняемого файла, на уровень выше
 * и в текущем рабочем каталоге. Формат файла:
 *   ключ=значение
 * Строки, начинающиеся с '#', игнорируются. Пробелы в значении удаляются.
 *
 * @param key ключ для поиска (например, "email" или "key")
 * @return значение ключа; пустая строка, если ключ не найден
 */
static QString readEmailTxtValue(const QString &key)
{
    // Look for email.txt next to the executable, and also one level up (build dir)
    QStringList searchPaths = {
        QCoreApplication::applicationDirPath() + "/email.txt",
        QCoreApplication::applicationDirPath() + "/../email.txt",
        QDir::currentPath() + "/email.txt"
    };

    for (const QString &path : searchPaths) {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        QTextStream in(&f);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith('#')) continue;      // skip comment lines
            int sep = line.indexOf('=');
            if (sep < 0) continue;
            QString k = line.left(sep).trimmed();
            QString v = line.mid(sep + 1).trimmed();
            if (k == key) {
                v.remove(' ');   // strip spaces from value
                return v;
            }
        }
    }
    qWarning() << "[SmtpClient] email.txt: key '" << key << "' not found";
    return {};
}

/**
 * @brief Получает адрес электронной почты отправителя
 *
 * @return ссылка на строку с email отправителя
 */
const QString &SmtpClient::senderEmail() {
    static QString val = readEmailTxtValue("email");
    return val;
}

/**
 * @brief Получает пароль (App Key) отправителя
 *
 * @return ссылка на строку с ключом отправителя
 */
const QString &SmtpClient::senderPassword() {
    static QString val = readEmailTxtValue("key");
    return val;
}

// ────────────────────────────────────────────────────────────────────────────

/**
 * @brief Ожидает и считывает все доступные данные из SSL-сокета
 *
 * Если данных нет, ожидает до timeoutMs миллисекунд. Затем считывает
 * все доступные данные, повторяя waitForReadyRead до исчерпания.
 *
 * @param socket SSL-сокет
 * @param timeoutMs таймаут ожидания в миллисекундах (по умолчанию 15000)
 * @return все прочитанные данные в виде QByteArray
 */
static QByteArray waitForData(QSslSocket *socket, int timeoutMs = 15000)
{
    QByteArray result;
    if (socket->bytesAvailable() == 0) {
        socket->waitForReadyRead(timeoutMs);
    }
    result = socket->readAll();
    while (socket->waitForReadyRead(500)) {
        result.append(socket->readAll());
    }
    return result;
}

/**
 * @brief Читает ответ SMTP-сервера из сокета
 *
 * Считывает данные через waitForData и возвращает последнюю непустую строку
 * (код ответа SMTP). Логирует полученный ответ.
 *
 * @param socket SSL-сокет
 * @param timeoutMs таймаут ожидания (по умолчанию 15000 мс)
 * @return последняя строка ответа SMTP-сервера
 */
static QString readResponse(QSslSocket *socket, int timeoutMs = 15000)
{
    QByteArray data = waitForData(socket, timeoutMs);
    QString all = QString::fromUtf8(data).trimmed();
    if (!all.isEmpty()) {
        QStringList lines = all.split('\n');
        QString last = lines.last().trimmed();
        qDebug() << "[SMTP] <--" << last;
        return last;
    }
    qDebug() << "[SMTP] readResponse: no data received";
    return QString();
}

/**
 * @brief Отправляет команду SMTP-серверу и считывает ответ
 *
 * @param socket SSL-сокет
 * @param cmd команда для отправки (включая \r\n)
 * @return ответ SMTP-сервера (код + описание)
 */
static QString sendCmd(QSslSocket *socket, const QByteArray &cmd)
{
    qDebug() << "[SMTP] -->" << cmd.trimmed();
    socket->write(cmd);
    socket->flush();
    socket->waitForBytesWritten(5000);
    return readResponse(socket);
}

/**
 * @brief Выполняет полную процедуру отправки email через SMTP
 *
 * Устанавливает SSL-соединение, выполняет авторизацию (AUTH LOGIN),
 * формирует письмо с заголовками (Subject, From, To, Content-Type)
 * и отправляет его. Завершает сессию командой QUIT.
 *
 * @param toEmail адрес получателя
 * @param subjectText тема письма
 * @param bodyText текст письма
 * @return true, если письмо успешно отправлено; false — при ошибке
 */
static bool doSend(const QString &toEmail,
                   const QString &subjectText,
                   const QString &bodyText)
{
    QSslSocket socket;
    socket.connectToHostEncrypted(SmtpClient::smtpHost, SmtpClient::smtpPort);
    socket.ignoreSslErrors();
    if (!socket.waitForEncrypted(20000)) {
        qDebug() << "[SMTP] SSL connection failed:" << socket.errorString();
        return false;
    }
    qDebug() << "[SMTP] SSL connected";

    QString resp = readResponse(&socket, 15000);
    if (resp.isEmpty() || !resp.startsWith("220")) return false;

    resp = sendCmd(&socket, "EHLO localhost\r\n");
    if (!resp.startsWith("250")) return false;

    resp = sendCmd(&socket, "AUTH LOGIN\r\n");
    if (!resp.startsWith("334")) return false;

    resp = sendCmd(&socket, SmtpClient::senderEmail().toUtf8().toBase64() + "\r\n");
    if (!resp.startsWith("334")) return false;

    resp = sendCmd(&socket, SmtpClient::senderPassword().toUtf8().toBase64() + "\r\n");
    if (!resp.startsWith("235")) return false;
    qDebug() << "[SMTP] Authenticated";

    resp = sendCmd(&socket, "MAIL FROM:<" + SmtpClient::senderEmail().toUtf8() + ">\r\n");
    if (!resp.startsWith("250")) return false;

    resp = sendCmd(&socket, "RCPT TO:<" + toEmail.toUtf8() + ">\r\n");
    if (!resp.startsWith("250")) return false;

    resp = sendCmd(&socket, "DATA\r\n");
    if (!resp.startsWith("354")) return false;

    QByteArray subjectB64 = subjectText.toUtf8().toBase64();

    QByteArray email;
    email += "Subject: =?UTF-8?B?" + subjectB64 + "?=\r\n";
    email += "From: " + SmtpClient::senderEmail().toUtf8() + "\r\n";
    email += "To: " + toEmail.toUtf8() + "\r\n";
    email += "Content-Type: text/plain; charset=UTF-8\r\n";
    email += "\r\n";
    email += bodyText.toUtf8() + "\r\n";
    email += ".\r\n";

    socket.write(email);
    socket.flush();
    socket.waitForBytesWritten(5000);
    resp = readResponse(&socket);
    if (!resp.startsWith("250")) {
        qDebug() << "[SMTP] Email rejected:" << resp;
        return false;
    }

    sendCmd(&socket, "QUIT\r\n");
    socket.disconnectFromHost();
    if (socket.state() != QAbstractSocket::UnconnectedState)
        socket.waitForDisconnected(5000);

    qDebug() << "[SMTP] Email sent to" << toEmail;
    return true;
}

/**
 * @brief Отправляет письмо с кодом подтверждения
 *
 * Формирует письмо с темой "Код подтверждения ТИМП" и текстом,
 * содержащим переданный код подтверждения.
 *
 * @param toEmail адрес получателя
 * @param code код подтверждения (6 цифр)
 * @return true, если письмо успешно отправлено; false — при ошибке
 */
bool SmtpClient::sendVerificationCode(const QString &toEmail, const QString &code)
{
    QString subject = QString::fromUtf8("\u041a\u043e\u0434 \u043f\u043e\u0434\u0442\u0432\u0435\u0440\u0436\u0434\u0435\u043d\u0438\u044f \u0422\u0418\u041c\u041f");
    QString body    = QString::fromUtf8("\u0412\u0430\u0448 \u043a\u043e\u0434 \u043f\u043e\u0434\u0442\u0432\u0435\u0440\u0436\u0434\u0435\u043d\u0438\u044f: ") + code;
    return doSend(toEmail, subject, body);
}

/**
 * @brief Отправляет письмо с кодом сброса пароля
 *
 * Формирует письмо с темой "Восстановление пароля ТИМП" и текстом,
 * содержащим приветствие с логином, код сброса и предупреждение
 * о необходимости игнорировать письмо при несанкционированном запросе.
 *
 * @param toEmail адрес получателя
 * @param login логин пользователя
 * @param code код сброса пароля (6 цифр)
 * @return true, если письмо успешно отправлено; false — при ошибке
 */
bool SmtpClient::sendPasswordResetCode(const QString &toEmail,
                                        const QString &login,
                                        const QString &code)
{
    QString subject = QString::fromUtf8("\u0412\u043e\u0441\u0441\u0442\u0430\u043d\u043e\u0432\u043b\u0435\u043d\u0438\u0435 \u043f\u0430\u0440\u043e\u043b\u044f \u0422\u0418\u041c\u041f");
    QString body    = QString::fromUtf8("\u0417\u0434\u0440\u0430\u0432\u0441\u0442\u0432\u0443\u0439\u0442\u0435, ") + login +
                      QString::fromUtf8(".\n\n\u0412\u0430\u0448 \u043a\u043e\u0434 \u0434\u043b\u044f \u0441\u0431\u0440\u043e\u0441\u0430 \u043f\u0430\u0440\u043e\u043b\u044f: ") + code +
                      QString::fromUtf8("\n\n\u0415\u0441\u043b\u0438 \u0432\u044b \u043d\u0435 \u0437\u0430\u043f\u0440\u0430\u0448\u0438\u0432\u0430\u043b\u0438 \u0441\u0431\u0440\u043e\u0441 \u043f\u0430\u0440\u043e\u043b\u044f \u2014 \u0438\u0433\u043d\u043e\u0440\u0438\u0440\u0443\u0439\u0442\u0435 \u044d\u0442\u043e \u043f\u0438\u0441\u044c\u043c\u043e.");
    return doSend(toEmail, subject, body);
}
