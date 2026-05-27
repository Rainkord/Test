#include <QCoreApplication>
#include "mytcpserver.h"
#include "logger.h"

/**
 * @brief Точка входа в программу (TCP-сервер)
 *
 * Создаёт объект QApplication, выводит баннер сервера,
 * создаёт экземпляр MyTcpServer и запускает основной цикл
 * обработки событий Qt.
 *
 * @param argc количество аргументов командной строки
 * @param argv массив аргументов командной строки
 * @return код возврата приложения
 */
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Logger::banner();

    MyTcpServer server;

    return a.exec();
}
