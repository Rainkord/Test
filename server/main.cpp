#include <QCoreApplication>
#include "mytcpserver.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Logger::banner();

    MyTcpServer server;

    return a.exec();
}
