#pragma once

#include <QString>
#include <QDateTime>
#include <iostream>

/**
 * @brief Пространство имён с ANSI-escape-кодами для форматирования вывода в терминале
 */
// ════════════════════════════════════════════════════════════
namespace Ansi {
    /// Сброс форматирования
    constexpr auto RESET    = "\033[0m";
    /// Жирный шрифт
    constexpr auto BOLD     = "\033[1m";
    /// Тусклый шрифт
    constexpr auto DIM      = "\033[2m";
    /// Красный цвет
    constexpr auto RED      = "\033[31m";
    /// Зелёный цвет
    constexpr auto GREEN    = "\033[32m";
    /// Жёлтый цвет
    constexpr auto YELLOW   = "\033[33m";
    /// Голубой цвет
    constexpr auto CYAN     = "\033[36m";
    /// Белый цвет
    constexpr auto WHITE    = "\033[37m";
    /// Ярко-красный цвет
    constexpr auto BRED     = "\033[91m";
    /// Ярко-зелёный цвет
    constexpr auto BGREEN   = "\033[92m";
    /// Ярко-жёлтый цвет
    constexpr auto BYELLOW  = "\033[93m";
    /// Ярко-синий цвет
    constexpr auto BBLUE    = "\033[94m";
    /// Ярко-пурпурный цвет
    constexpr auto BMAGENTA = "\033[95m";
    /// Ярко-голубой цвет
    constexpr auto BCYAN    = "\033[96m";
    /// Ярко-белый цвет
    constexpr auto BWHITE   = "\033[97m";
}

/**
 * @brief Класс логирования серверных событий в терминал
 *
 * Предоставляет статические методы для вывода форматированных
 * сообщений о событиях сервера: запуск, подключения, отключения,
 * запросы, ответы, авторизация, регистрация, отправка email,
 * ошибки. Использует ANSI-escape-коды для цветового форматирования.
 */
// ════════════════════════════════════════════════════════════
class Logger
{
public:

    /**
     * @brief Выводит баннер сервера при запуске
     *
     * Отображает декоративную рамку с названием сервера
     * и информацией о группе.
     */
    // ── Баннер при старте ──────────────────────────────────────
    static void banner()
    {
        // Ширина бокса = 44 символа (считая внешние рамки ╔╗)
        // Внутренняя ширина: 42 символа
        std::cout
            << "\n"
            << Ansi::BBLUE << Ansi::BOLD
            << "  \u2554" << repeat('\xcd', 44) << "\u2557\n"
            << "  \u2551" << spaces(44)          << "\u2551\n"
            << "  \u2551" << Ansi::BCYAN
            <<                  center("TCP SERVER", 44)
            << Ansi::BBLUE << "\u2551\n"
            << "  \u2551" << Ansi::BWHITE
            <<                  center("TIMP 251/372  \u00b7  Group 5", 44)
            << Ansi::BBLUE << "\u2551\n"
            << "  \u2551" << spaces(44)          << "\u2551\n"
            << "  \u255a" << repeat('\xcd', 44) << "\u255d"
            << Ansi::RESET << "\n\n";
    }

    /**
     * @brief Логирует успешный запуск сервера
     *
     * @param port номер порта, на котором работает сервер
     */
    // ── Сервер ───────────────────────────────────────────────
    static void serverStarted(int port)
    {
        tag("START", Ansi::BGREEN);
        std::cout
            << Ansi::BGREEN << Ansi::BOLD << "Сервер запущен"
            << Ansi::RESET  << "  → порт "
            << Ansi::BYELLOW << Ansi::BOLD << port
            << Ansi::RESET  << "\n";
        divider();
    }

    /**
     * @brief Логирует ошибку запуска сервера
     *
     * @param err описание ошибки от QTcpServer
     */
    static void serverFailed(const QString &err)
    {
        tag("FAIL", Ansi::BRED);
        std::cout
            << Ansi::BRED << Ansi::BOLD << "Не удалось запустить сервер: "
            << Ansi::RESET << err.toStdString() << "\n";
    }

    /**
     * @brief Логирует подключение нового клиента
     *
     * @param ip IP-адрес подключившегося клиента
     * @param port порт клиента
     * @param id уникальный дескриптор сокета клиента
     */
    // ── Клиенты ─────────────────────────────────────────────
    static void clientConnected(const QString &ip, quint16 port, qintptr id)
    {
        tag("CONN", Ansi::BCYAN);
        std::cout
            << Ansi::BCYAN  << Ansi::BOLD << "Подключение"
            << Ansi::RESET  << "  "
            << Ansi::BWHITE << ip.toStdString()
            << Ansi::DIM    << ":" << port
            << Ansi::RESET  << "  id=" << Ansi::BYELLOW << id
            << Ansi::RESET  << "\n";
    }

    /**
     * @brief Логирует отключение клиента
     *
     * @param ip IP-адрес отключившегося клиента
     */
    static void clientDisconnected(const QString &ip)
    {
        tag("DISC", Ansi::YELLOW);
        std::cout
            << Ansi::YELLOW << "Отключение  "
            << Ansi::BWHITE << ip.toStdString()
            << Ansi::RESET  << "\n";
    }

    /**
     * @brief Логирует входящее сообщение от клиента
     *
     * @param ip IP-адрес отправителя
     * @param msg текст сообщения
     */
    // ── Запрос / ответ ──────────────────────────────────────
    static void request(const QString &ip, const QString &msg)
    {
        tag("RECV", Ansi::BMAGENTA);
        std::cout
            << Ansi::DIM      << ip.toStdString() << "  "
            << Ansi::RESET    << Ansi::BWHITE << "→ "
            << Ansi::BMAGENTA << msg.toStdString()
            << Ansi::RESET    << "\n";
    }

    /**
     * @brief Логирует отправляемый клиенту ответ
     *
     * @param resp текст ответа
     */
    static void response(const QString &resp)
    {
        tag("SEND", Ansi::BBLUE);
        std::cout
            << Ansi::BBLUE << resp.toStdString()
            << Ansi::RESET << "\n";
    }

    /**
     * @brief Логирует успешную авторизацию
     *
     * @param login логин авторизованного пользователя
     */
    // ── Авторизация ─────────────────────────────────────────
    static void authOk(const QString &login)
    {
        tag("AUTH", Ansi::BGREEN);
        std::cout
            << Ansi::BGREEN << Ansi::BOLD << "Успешный вход"
            << Ansi::RESET  << "  пользователь: "
            << Ansi::BWHITE << login.toStdString()
            << Ansi::RESET  << "\n";
    }

    /**
     * @brief Логирует неуспешную попытку авторизации
     *
     * @param login логин пользователя с неверными данными
     */
    static void authFail(const QString &login)
    {
        tag("AUTH", Ansi::BRED);
        std::cout
            << Ansi::BRED   << Ansi::BOLD << "Неверные данные"
            << Ansi::RESET  << "  пользователь: "
            << Ansi::BWHITE << login.toStdString()
            << Ansi::RESET  << "\n";
    }

    /**
     * @brief Логирует успешную отправку email
     *
     * @param toEmail адрес получателя письма
     */
    // ── Email-отправка ───────────────────────────────────────
    static void emailSent(const QString &toEmail)
    {
        tag("MAIL", Ansi::BGREEN);
        std::cout
            << Ansi::BGREEN << Ansi::BOLD << "Письмо отправлено"
            << Ansi::RESET  << "  → "
            << Ansi::BCYAN  << toEmail.toStdString()
            << Ansi::RESET  << "\n";
    }

    /**
     * @brief Логирует ошибку отправки email
     *
     * @param toEmail адрес получателя, которому не удалось отправить письмо
     */
    static void emailFailed(const QString &toEmail)
    {
        tag("MAIL", Ansi::BRED);
        std::cout
            << Ansi::BRED   << Ansi::BOLD << "Ошибка отправки"
            << Ansi::RESET  << "  → "
            << Ansi::BCYAN  << toEmail.toStdString()
            << Ansi::RESET  << "\n";
    }

    /**
     * @brief Логирует отправку кода 2FA-подтверждения
     *
     * @param login логин пользователя
     * @param email email пользователя
     * @param code код подтверждения (отображается в рамке)
     */
    // ── 2FA-код ──────────────────────────────────────────────
    static void codeSent(const QString &login, const QString &email, const QString &code)
    {
        tag("2FA ", Ansi::BYELLOW);
        std::cout
            << Ansi::BWHITE  << "Пользователь: "
            << Ansi::BCYAN   << login.toStdString()
            << Ansi::RESET   << "  email: "
            << Ansi::DIM     << email.toStdString()
            << Ansi::RESET   << "\n";
        codeBox(code, Ansi::BYELLOW);
    }

    /**
     * @brief Логирует генерацию кода подтверждения регистрации
     *
     * @param login логин регистрируемого пользователя
     * @param email email пользователя
     * @param code код подтверждения (отображается в рамке)
     */
    // ── Регистрация ──────────────────────────────────────────
    static void regCode(const QString &login, const QString &email, const QString &code)
    {
        tag("REG ", Ansi::BBLUE);
        std::cout
            << Ansi::BWHITE << "Регистрация: "
            << Ansi::BCYAN  << login.toStdString()
            << Ansi::RESET  << "  email: "
            << Ansi::DIM    << email.toStdString()
            << Ansi::RESET  << "\n";
        codeBox(code, Ansi::BBLUE);
    }

    /**
     * @brief Логирует успешное создание пользователя
     *
     * @param login логин созданного пользователя
     */
    static void regOk(const QString &login)
    {
        tag("REG ", Ansi::BGREEN);
        std::cout
            << Ansi::BGREEN << Ansi::BOLD << "Пользователь создан"
            << Ansi::RESET  << "  логин: "
            << Ansi::BWHITE << login.toStdString()
            << Ansi::RESET  << "\n";
    }

    /**
     * @brief Логирует ошибку при регистрации
     *
     * @param login логин, при регистрации которого произошла ошибка
     */
    static void regFail(const QString &login)
    {
        tag("REG ", Ansi::BRED);
        std::cout
            << Ansi::BRED   << Ansi::BOLD << "Ошибка регистрации"
            << Ansi::RESET  << "  логин: "
            << Ansi::BWHITE << login.toStdString()
            << Ansi::RESET  << "\n";
    }

    /**
     * @brief Логирует генерацию кода сброса пароля
     *
     * @param email email пользователя
     * @param code код подтверждения (отображается в рамке)
     */
    // ── Сброс пароля ─────────────────────────────────────────
    static void resetCode(const QString &email, const QString &code)
    {
        tag("RESET", Ansi::BMAGENTA);
        std::cout
            << Ansi::BWHITE   << "Сброс пароля  email: "
            << Ansi::BCYAN    << email.toStdString()
            << Ansi::RESET    << "\n";
        codeBox(code, Ansi::BMAGENTA);
    }

    /**
     * @brief Логирует успешное обновление пароля
     *
     * @param email email пользователя, чей пароль был обновлён
     */
    static void resetOk(const QString &email)
    {
        tag("RESET", Ansi::BGREEN);
        std::cout
            << Ansi::BGREEN << Ansi::BOLD << "Пароль обновлён"
            << Ansi::RESET  << "  email: "
            << Ansi::BWHITE << email.toStdString()
            << Ansi::RESET  << "\n";
    }

    /**
     * @brief Логирует сообщение об ошибке
     *
     * @param msg текст ошибки
     */
    // ── Ошибка ────────────────────────────────────────────
    static void error(const QString &msg)
    {
        tag("ERR ", Ansi::BRED);
        std::cout
            << Ansi::BRED << msg.toStdString()
            << Ansi::RESET << "\n";
    }

    /**
     * @brief Выводит горизонтальный разделитель
     */
    // ── Разделитель ──────────────────────────────────────────
    static void divider()
    {
        std::cout
            << Ansi::DIM
            << "  ──────────────────────────────────────────────\n"
            << Ansi::RESET;
    }

private:

    /**
     * @brief Выводит временну́ю метку и тег сообщения
     *
     * @param label текстовый тег (например, "CONN", "AUTH")
     * @param color ANSI-escape-код цвета
     */
    // ── Тег ───────────────────────────────────────────────────
    static void tag(const char *label, const char *color)
    {
        QString ts = QDateTime::currentDateTime().toString("hh:mm:ss");
        std::cout
            << Ansi::DIM  << "[" << ts.toStdString() << "] "
            << color      << Ansi::BOLD
            << "[" << label << "] "
            << Ansi::RESET;
    }

    /**
     * @brief Выводит код подтверждения в декоративной рамке
     *
     * @param code код подтверждения
     * @param color ANSI-escape-код цвета рамки
     */
    // ── Рамка с кодом ───────────────────────────────────────
    static void codeBox(const QString &code, const char *color)
    {
        std::cout
            << color << Ansi::BOLD
            << "         \u250c\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2510\n"
            << "         \u2502  " << Ansi::BWHITE << Ansi::BOLD
            << "  \u041a\u041e\u0414: " << code.toStdString() << "        "
            << color << Ansi::BOLD << "\u2502\n"
            << "         \u2514\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2518"
            << Ansi::RESET << "\n";
    }

    /**
     * @brief Повторяет символ n раз
     *
     * @param c символ для повторения
     * @param n количество повторений
     * @return строка из n символов c
     */
    // ── Вспомогательные методы для баннера ──────────────────
    static std::string repeat(char c, int n)
    {
        return std::string(static_cast<size_t>(n), c);
    }

    /**
     * @brief Генерирует строку из n пробелов
     *
     * @param n количество пробелов
     * @return строка пробелов
     */
    static std::string spaces(int n)
    {
        return std::string(static_cast<size_t>(n), ' ');
    }

    /**
     * @brief Центрирует ASCII-строку в поле заданной ширины
     *
     * @param text исходная строка
     * @param width ширина поля в символах
     * @return отцентрированная строка
     */
    // Центрирование ASCII-строки в поле шириной width (символов, не байт)
    static std::string center(const char *text, int width)
    {
        std::string s(text);
        int len = static_cast<int>(s.size());
        if (len >= width) return s;
        int pad  = width - len;
        int left = pad / 2;
        int right = pad - left;
        return std::string(left, ' ') + s + std::string(right, ' ');
    }
};
