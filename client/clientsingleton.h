#ifndef CLIENTSINGLETON_H
#define CLIENTSINGLETON_H

#include <QTcpSocket>
#include <QString>

class ClientSingleton {
public:
    static ClientSingleton& instance();
    bool connectToServer(const QString &host, int port);
    QString sendRequest(const QString &request);
    void disconnectFromServer();
    bool isConnected() const;

private:
    ClientSingleton();
    ~ClientSingleton();
    ClientSingleton(const ClientSingleton&) = delete;
    ClientSingleton& operator=(const ClientSingleton&) = delete;

    QTcpSocket *socket;
};

#endif // CLIENTSINGLETON_H
