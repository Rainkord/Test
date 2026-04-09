#include <QCoreApplication>
#include <QDebug>

#include "database.h"
#include "mytcpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Connect to the SQLite database (Singleton)
    if (!Database::instance().connect("users.db")) {
        qDebug() << "Failed to connect to database. Exiting.";
        return 1;
    }

    // Create and start the TCP server
    MyTcpServer server;

    return a.exec();
}
