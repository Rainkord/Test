#pragma once

#include <QString>
#include <QDateTime>
#include <QTextStream>
#include <iostream>

// ════════════════════════════════════════════════════════════
//  ANSI-цвета и стили
// ════════════════════════════════════════════════════════════
namespace Ansi {
    // Сброс
    constexpr auto RESET   = "\033[0m";

    // Жирный
    constexpr auto BOLD    = "\033[1m";
    constexpr auto DIM     = "\033[2m";

    // Цвета текста
    constexpr auto BLACK   = "\033[30m";
    constexpr auto RED     = "\033[31m";
    constexpr auto GREEN   = "\033[32m";
    constexpr auto YELLOW  = "\033[33m";
    constexpr auto BLUE    = "\033[34m";
    constexpr auto MAGENTA = "\033[35m";
    constexpr auto CYAN    = "\033[36m";
    constexpr auto WHITE   = "\033[37m";

    // Яркие цвета
    constexpr auto BRED    = "\033[91m";
    constexpr auto BGREEN  = "\033[92m";
    constexpr auto BYELLOW = "\033[93m";
    constexpr auto BBLUE   = "\033[94m";
    constexpr auto BMAGENTA= "\033[95m";
    constexpr auto BCYAN   = "\033[96m";
    constexpr auto BWHITE  = "\033[97m";

    // Фон
    constexpr auto BG_DARK = "\033[40m";
}

// ════════════════════════════════════════════════════════════
//  Logger — статический класс с красивым выводом
// ════════════════════════════════════════════════════════════
class Logger
{
public:
    // ── Баннер при старте ────────────────────────────────────
    static void banner()
    {
        std::cout
            << "\n"
            << Ansi::BBLUE << Ansi::BOLD
            << "  ╔══════════════════════════════════════════╗\n"
            << "  ║                                          ║\n"
            << "  ║   " << Ansi::BCYAN  << "  TCP SERVER  v1.0  "
            << Ansi::BBLUE << "                ║\n"
            << "  ║   " << Ansi::BWHITE << "  TIMP 251/372 · Group 5"
            << Ansi::BBLUE << "            ║\n"
            << "  ║                                          ║\n"
            << "  ╚══════════════════════════════════════════╝"
            << Ansi::RESET << "\n\n";
    }

    // ── Сервер запущен ───────────────────────────────────────
    static void serverStarted(int port)
    {
        tag("START", Ansi::BGREEN);
        std::cout
            << Ansi::BGREEN << Ansi::BOLD << "Сервер запущен"
            << Ansi::RESET  << " → порт "
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

    // ── Клиент подключился / отключился ─────────────────────
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

    // ── Запрос / ответ ───────────────────────────────────────
    static void request(const QString &ip, const QString &msg)
    {
        tag("RECV", Ansi::BMAGENTA);
        std::cout
            << Ansi::DIM    << ip.toStdString() << "  "
            << Ansi::RESET  << Ansi::BWHITE << "→ "
            << Ansi::BMAGENTA << msg.toStdString()
            << Ansi::RESET  << "\n";
    }

    static void response(const QString &resp)
    {
        tag("SEND", Ansi::BBLUE);
        std::cout
            << Ansi::BBLUE << resp.toStdString()
            << Ansi::RESET << "\n";
    }

    // ── Авторизация ──────────────────────────────────────────
    static void authOk(const QString &login)
    {
        tag("AUTH", Ansi::BGREEN);
        std::cout
            << Ansi::BGREEN << Ansi::BOLD << "Успешный вход  "
            << Ansi::BWHITE << login.toStdString()
            << Ansi::RESET  << "\n";
    }

    static void authFail(const QString &login)
    {
        tag("AUTH", Ansi::BRED);
        std::cout
            << Ansi::BRED   << Ansi::BOLD << "Неверные данные  "
            << Ansi::BWHITE << login.toStdString()
            << Ansi::RESET  << "\n";
    }

    // ── 2FA-код ──────────────────────────────────────────────
    static void codeSent(const QString &login, const QString &email, const QString &code)
    {
        tag("2FA ", Ansi::BYELLOW);
        std::cout
            << Ansi::BWHITE << "Пользователь: "
            << Ansi::BCYAN  << login.toStdString()
            << Ansi::RESET  << "  email: "
            << Ansi::DIM    << email.toStdString()
            << Ansi::RESET  << "\n";

        // Рамка с кодом
        std::cout
            << Ansi::BYELLOW << Ansi::BOLD
            << "         ┌─────────────────────┐\n"
            << "         │   КОД: "
            << Ansi::BWHITE  << code.toStdString()
            << Ansi::BYELLOW << Ansi::BOLD
            << "       │\n"
            << "         └─────────────────────┘"
            << Ansi::RESET   << "\n";
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

        std::cout
            << Ansi::BBLUE << Ansi::BOLD
            << "         ┌─────────────────────┐\n"
            << "         │   КОД: "
            << Ansi::BWHITE  << code.toStdString()
            << Ansi::BBLUE << Ansi::BOLD
            << "       │\n"
            << "         └─────────────────────┘"
            << Ansi::RESET   << "\n";
    }

    static void regOk(const QString &login)
    {
        tag("REG ", Ansi::BGREEN);
        std::cout
            << Ansi::BGREEN << Ansi::BOLD << "Пользователь создан  "
            << Ansi::BWHITE << login.toStdString()
            << Ansi::RESET  << "\n";
    }

    static void regFail(const QString &login)
    {
        tag("REG ", Ansi::BRED);
        std::cout
            << Ansi::BRED << Ansi::BOLD << "Ошибка регистрации  "
            << Ansi::BWHITE << login.toStdString()
            << Ansi::RESET  << "\n";
    }

    // ── Сброс пароля ─────────────────────────────────────────
    static void resetCode(const QString &email, const QString &code)
    {
        tag("RESET", Ansi::BMAGENTA);
        std::cout
            << Ansi::BWHITE << "Сброс пароля  email: "
            << Ansi::BCYAN  << email.toStdString()
            << Ansi::RESET  << "\n";

        std::cout
            << Ansi::BMAGENTA << Ansi::BOLD
            << "         ┌─────────────────────┐\n"
            << "         │   КОД: "
            << Ansi::BWHITE  << code.toStdString()
            << Ansi::BMAGENTA << Ansi::BOLD
            << "       │\n"
            << "         └─────────────────────┘"
            << Ansi::RESET   << "\n";
    }

    static void resetOk(const QString &email)
    {
        tag("RESET", Ansi::BGREEN);
        std::cout
            << Ansi::BGREEN << Ansi::BOLD << "Пароль обновлён  "
            << Ansi::BWHITE << email.toStdString()
            << Ansi::RESET  << "\n";
    }

    // ── Универсальная ошибка ─────────────────────────────────
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
    // ── Тег с временной меткой ───────────────────────────────
    static void tag(const char *label, const char *color)
    {
        QString ts = QDateTime::currentDateTime().toString("hh:mm:ss");
        std::cout
            << Ansi::DIM    << "[" << ts.toStdString() << "] "
            << color        << Ansi::BOLD
            << "[" << label << "] "
            << Ansi::RESET;
    }
};
