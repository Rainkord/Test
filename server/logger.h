#pragma once

#include <QString>
#include <QDateTime>
#include <iostream>

// ════════════════════════════════════════════════════════════
namespace Ansi {
    constexpr auto RESET    = "\033[0m";
    constexpr auto BOLD     = "\033[1m";
    constexpr auto DIM      = "\033[2m";
    constexpr auto RED      = "\033[31m";
    constexpr auto GREEN    = "\033[32m";
    constexpr auto YELLOW   = "\033[33m";
    constexpr auto CYAN     = "\033[36m";
    constexpr auto WHITE    = "\033[37m";
    constexpr auto BRED     = "\033[91m";
    constexpr auto BGREEN   = "\033[92m";
    constexpr auto BYELLOW  = "\033[93m";
    constexpr auto BBLUE    = "\033[94m";
    constexpr auto BMAGENTA = "\033[95m";
    constexpr auto BCYAN    = "\033[96m";
    constexpr auto BWHITE   = "\033[97m";
}

// ════════════════════════════════════════════════════════════
class Logger
{
public:

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

    static void serverFailed(const QString &err)
    {
        tag("FAIL", Ansi::BRED);
        std::cout
            << Ansi::BRED << Ansi::BOLD << "Не удалось запустить сервер: "
            << Ansi::RESET << err.toStdString() << "\n";
    }

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

    static void clientDisconnected(const QString &ip)
    {
        tag("DISC", Ansi::YELLOW);
        std::cout
            << Ansi::YELLOW << "Отключение  "
            << Ansi::BWHITE << ip.toStdString()
            << Ansi::RESET  << "\n";
    }

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

    static void response(const QString &resp)
    {
        tag("SEND", Ansi::BBLUE);
        std::cout
            << Ansi::BBLUE << resp.toStdString()
            << Ansi::RESET << "\n";
    }

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

    static void authFail(const QString &login)
    {
        tag("AUTH", Ansi::BRED);
        std::cout
            << Ansi::BRED   << Ansi::BOLD << "Неверные данные"
            << Ansi::RESET  << "  пользователь: "
            << Ansi::BWHITE << login.toStdString()
            << Ansi::RESET  << "\n";
    }

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

    static void emailFailed(const QString &toEmail)
    {
        tag("MAIL", Ansi::BRED);
        std::cout
            << Ansi::BRED   << Ansi::BOLD << "Ошибка отправки"
            << Ansi::RESET  << "  → "
            << Ansi::BCYAN  << toEmail.toStdString()
            << Ansi::RESET  << "\n";
    }

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

    static void regOk(const QString &login)
    {
        tag("REG ", Ansi::BGREEN);
        std::cout
            << Ansi::BGREEN << Ansi::BOLD << "Пользователь создан"
            << Ansi::RESET  << "  логин: "
            << Ansi::BWHITE << login.toStdString()
            << Ansi::RESET  << "\n";
    }

    static void regFail(const QString &login)
    {
        tag("REG ", Ansi::BRED);
        std::cout
            << Ansi::BRED   << Ansi::BOLD << "Ошибка регистрации"
            << Ansi::RESET  << "  логин: "
            << Ansi::BWHITE << login.toStdString()
            << Ansi::RESET  << "\n";
    }

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

    static void resetOk(const QString &email)
    {
        tag("RESET", Ansi::BGREEN);
        std::cout
            << Ansi::BGREEN << Ansi::BOLD << "Пароль обновлён"
            << Ansi::RESET  << "  email: "
            << Ansi::BWHITE << email.toStdString()
            << Ansi::RESET  << "\n";
    }

    // ── Ошибка ────────────────────────────────────────────
    static void error(const QString &msg)
    {
        tag("ERR ", Ansi::BRED);
        std::cout
            << Ansi::BRED << msg.toStdString()
            << Ansi::RESET << "\n";
    }

    // ── Разделитель ──────────────────────────────────────────
    static void divider()
    {
        std::cout
            << Ansi::DIM
            << "  ──────────────────────────────────────────────\n"
            << Ansi::RESET;
    }

private:
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

    // ── Вспомогательные методы для баннера ──────────────────
    static std::string repeat(char c, int n)
    {
        return std::string(static_cast<size_t>(n), c);
    }

    static std::string spaces(int n)
    {
        return std::string(static_cast<size_t>(n), ' ');
    }

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
