#include "mytcpserver.h"
#include "functionsforserver.h"

#include <QDebug>

MyTcpServer::MyTcpServer(QObject *parent)
    : QObject(parent),
      mTcpServer(new QTcpServer(this)),
      mServerStatus(0)
{
    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if (mTcpServer->listen(QHostAddress::Any, 33333)) {
        mServerStatus = 1;
        qDebug() << "Server started on port 33333";
    } else {
        qDebug() << "Server failed to start:" << mTcpServer->errorString();
    }
}

MyTcpServer::~MyTcpServer()
{
    // Close all client sockets
    for (QTcpSocket *socket : mClients.values()) {
        if (socket) {
            socket->close();
            delete socket;
        }
    }
    mClients.clear();

    // Close the server
    if (mTcpServer->isListening()) {
        mTcpServer->close();
    }

    qDebug() << "Server shut down.";
}

void MyTcpServer::slotNewConnection()
{
    QTcpSocket *newSocket = mTcpServer->nextPendingConnection();
    if (!newSocket) {
        return;
    }

    int descriptor = static_cast<int>(newSocket->socketDescriptor());
    mClients.insert(descriptor, newSocket);

    connect(newSocket, &QTcpSocket::readyRead,
            this, &MyTcpServer::slotServerRead);
    connect(newSocket, &QTcpSocket::disconnected,
            this, &MyTcpServer::slotClientDisconnected);

    qDebug() << "New client connected, descriptor:" << descriptor;

    // Optional greeting
    newSocket->write(QString("hello").toUtf8());
}

void MyTcpServer::slotServerRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        return;
    }

    QByteArray rawData = socket->readAll();
    QString inData = QString::fromUtf8(rawData);

    qDebug() << "Received from client"
             << socket->socketDescriptor() << ":" << inData;

    QString response = FunctionsForServer::parsing(inData,
                           static_cast<int>(socket->socketDescriptor()));

    qDebug() << "Response to client"
             << socket->socketDescriptor() << ":" << response;

    socket->write(response.toUtf8());
}

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        return;
    }

    int descriptor = static_cast<int>(socket->socketDescriptor());
    mClients.remove(descriptor);

    qDebug() << "Client disconnected, descriptor:" << descriptor;

    socket->deleteLater();
}
