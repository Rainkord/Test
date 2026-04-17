#include <QCoreApplication>
#include "mytcpserver.h"

/**
 * @file main.cpp
 * @brief Точка входа консольного эхо-сервера.
 *
 * Создаёт экземпляр QCoreApplication и запускает MyTcpServer.
 * Сервер работает в режиме ожидания событий до принудительного завершения.
 */

/**
 * @brief Точка входа приложения.
 * @param argc Количество аргументов командной строки.
 * @param argv Массив аргументов командной строки.
 * @return Код завершения приложения (0 при нормальном завершении).
 */
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MyTcpServer server;
    return a.exec();
}
