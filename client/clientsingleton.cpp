#include "clientsingleton.h"
#include <QDebug>

ClientSingleton::ClientSingleton(QObject *parent)
    : QObject(parent)
{
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &ClientSingleton::onReadyRead);
}

ClientSingleton::~ClientSingleton()
{
    if (socket) {
        socket->disconnectFromHost();
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
    socket->connectToHost(host, static_cast<quint16>(port));
    return socket->waitForConnected(3000);
}

void ClientSingleton::disconnectFromServer()
{
    socket->disconnectFromHost();
}

bool ClientSingleton::isConnected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

QString ClientSingleton::sendRequest(const QString &request)
{
    if (socket->state() != QAbstractSocket::ConnectedState) {
        socket->connectToHost("127.0.0.1", 33333);
        if (!socket->waitForConnected(3000))
            return QString();
    }

    socket->write((request + "\n").toUtf8());
    socket->flush();

    if (!socket->waitForBytesWritten(3000))
        return QString();

    if (socket->bytesAvailable() == 0) {
        if (!socket->waitForReadyRead(60000))
            return QString();
    }

    QByteArray data = socket->readAll();
    while (socket->waitForReadyRead(300))
        data += socket->readAll();

    return QString::fromUtf8(data).trimmed();
}

void ClientSingleton::sendRequestAsync(const QString &request)
{
    qDebug() << "[CS] sendRequestAsync, socket state:" << socket->state();

    if (socket->state() != QAbstractSocket::ConnectedState) {
        socket->connectToHost("127.0.0.1", 33333);
        if (!socket->waitForConnected(3000)) {
            qDebug() << "[CS] Connection failed:" << socket->errorString();
            emit responseReceived("");
            return;
        }
        qDebug() << "[CS] Connected OK";
    }

    QByteArray data = (request + "\n").toUtf8();
    qint64 written = socket->write(data);
    qDebug() << "[CS] Written:" << written << "of" << data.size();
    socket->flush();
}

void ClientSingleton::onReadyRead()
{
    QByteArray data = socket->readAll();
    QString response = QString::fromUtf8(data).trimmed();
    if (!response.isEmpty()) {
        emit responseReceived(response);
    }
}
