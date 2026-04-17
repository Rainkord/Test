#include "mytcpserver.h"
#include "functionsforserver.h"

#include <QDebug>

#define SERVER_PORT 33333

MyTcpServer::MyTcpServer(QObject *parent)
    : QObject(parent)
    , m_pServer(new QTcpServer(this))
    , m_serverStatus(false)
{
    connect(m_pServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if (!m_pServer->listen(QHostAddress::Any, SERVER_PORT)) {
        qDebug() << "[Server] Failed to start:" << m_pServer->errorString();
    } else {
        m_serverStatus = true;
        qDebug() << "[Server] Listening on port" << SERVER_PORT;
    }
}

MyTcpServer::~MyTcpServer()
{
    for (QTcpSocket *socket : m_clients) {
        socket->close();
        socket->deleteLater();
    }
    m_clients.clear();
    m_pServer->close();
}

void MyTcpServer::slotNewConnection()
{
    while (m_pServer->hasPendingConnections()) {
        QTcpSocket *socket = m_pServer->nextPendingConnection();

        connect(socket, &QTcpSocket::readyRead,
                this, &MyTcpServer::slotServerRead);
        connect(socket, &QTcpSocket::disconnected,
                this, &MyTcpServer::slotClientDisconnected);

        m_clients[socket->socketDescriptor()] = socket;

        qDebug() << "[Server] New connection from"
                 << socket->peerAddress().toString()
                 << "port" << socket->peerPort()
                 << "descriptor" << socket->socketDescriptor();
    }
}

void MyTcpServer::slotServerRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) return;

    while (socket->canReadLine()) {
        QString message = QString::fromUtf8(socket->readLine()).trimmed();
        if (message.isEmpty()) continue;

        qDebug() << "[Server] Received from"
                 << socket->peerAddress().toString()
                 << ":" << message;

        QString response = FunctionsForServer::processMessage(message);
        qDebug() << "[Server] Sending response:" << response;

        QByteArray data = (response + "\n").toUtf8();
        socket->write(data);
        socket->flush();
    }
}

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) return;

    qDebug() << "[Server] Client disconnected:"
             << socket->peerAddress().toString();

    m_clients.remove(socket->socketDescriptor());
    socket->deleteLater();
}
