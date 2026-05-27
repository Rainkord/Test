#include "mytcpserver.h"
#include "functionsforserver.h"
#include "logger.h"

#define SERVER_PORT 33333

/**
 * @brief Конструктор TCP-сервера
 *
 * Создаёт QTcpServer, подключает сигнал newConnection к слоту
 * slotNewConnection. Пытается начать прослушивание на порту SERVER_PORT.
 * В случае ошибки выводит лог-сообщение, иначе устанавливает
 * статус сервера в true.
 */
MyTcpServer::MyTcpServer(QObject *parent)
    : QObject(parent)
    , m_pServer(new QTcpServer(this))
    , m_serverStatus(false)
{
    connect(m_pServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if (!m_pServer->listen(QHostAddress::Any, SERVER_PORT)) {
        Logger::serverFailed(m_pServer->errorString());
    } else {
        m_serverStatus = true;
        Logger::serverStarted(SERVER_PORT);
    }
}

/**
 * @brief Деструктор TCP-сервера
 *
 * Проходит по всем активным клиентским соединениям, закрывает
 * каждый сокет, планирует его удаление и очищает карту клиентов.
 * Затем закрывает сам сервер.
 */
MyTcpServer::~MyTcpServer()
{
    for (QTcpSocket *socket : m_clients) {
        socket->close();
        socket->deleteLater();
    }
    m_clients.clear();
    m_pServer->close();
}

/**
 * @brief Слот обработки нового подключения клиента
 *
 * Последовательно принимает все ожидающие подключения от QTcpServer.
 * Для каждого нового сокета устанавливает обработчики сигналов
 * readyRead и disconnected, добавляет сокет в карту m_clients
 * и логирует информацию о подключении.
 */
void MyTcpServer::slotNewConnection()
{
    while (m_pServer->hasPendingConnections()) {
        QTcpSocket *socket = m_pServer->nextPendingConnection();

        connect(socket, &QTcpSocket::readyRead,
                this, &MyTcpServer::slotServerRead);
        connect(socket, &QTcpSocket::disconnected,
                this, &MyTcpServer::slotClientDisconnected);

        m_clients[socket->socketDescriptor()] = socket;

        Logger::clientConnected(
            socket->peerAddress().toString(),
            socket->peerPort(),
            socket->socketDescriptor()
        );
    }
}

/**
 * @brief Слот чтения данных от клиента
 *
 * Получает указатель на сокет-отправителя через sender().
 * Последовательно считывает строки из сокета, передаёт каждое
 * сообщение на обработку через FunctionsForServer::processMessage,
 * логирует запрос и ответ, затем отправляет ответ обратно клиенту.
 */
void MyTcpServer::slotServerRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) return;

    while (socket->canReadLine()) {
        QString message = QString::fromUtf8(socket->readLine()).trimmed();
        if (message.isEmpty()) continue;

        Logger::request(socket->peerAddress().toString(), message);

        QString response = FunctionsForServer::processMessage(message);

        Logger::response(response);
        Logger::divider();

        QByteArray data = (response + "\n").toUtf8();
        socket->write(data);
        socket->flush();
    }
}

/**
 * @brief Слот обработки отключения клиента
 *
 * Получает указатель на отключившийся сокет через sender(),
 * логирует отключение, удаляет сокет из карты m_clients
 * и планирует его удаление через deleteLater().
 */
void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) return;

    Logger::clientDisconnected(socket->peerAddress().toString());
    Logger::divider();

    m_clients.remove(socket->socketDescriptor());
    socket->deleteLater();
}
