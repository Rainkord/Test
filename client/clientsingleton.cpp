#include "clientsingleton.h"

ClientSingleton::ClientSingleton()
{
    socket = new QTcpSocket();
}

ClientSingleton::~ClientSingleton()
{
    if (socket) {
        socket->disconnectFromHost();
        delete socket;
        socket = nullptr;
    }
}

ClientSingleton& ClientSingleton::instance()
{
    static ClientSingleton inst;
    return inst;
}

bool ClientSingleton::connectToServer(const QString &host, int port)
{
    if (socket->state() == QAbstractSocket::ConnectedState)
        return true;

    socket->connectToHost(host, port);
    return socket->waitForConnected(3000);
}

QString ClientSingleton::sendRequest(const QString &request)
{
    if (socket->state() != QAbstractSocket::ConnectedState)
        return QString();

    QByteArray data = request.toUtf8();
    socket->write(data);
    socket->flush();
    socket->waitForBytesWritten(3000);

    if (!socket->waitForReadyRead(3000))
        return QString();

    QByteArray response = socket->readAll();
    return QString::fromUtf8(response);
}

void ClientSingleton::disconnectFromServer()
{
    socket->disconnectFromHost();
}

bool ClientSingleton::isConnected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}
