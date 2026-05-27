#include "clientsingleton.h"
#include <QDebug>

/**
 * @brief Конструктор класса ClientSingleton
 * 
 * Создаёт TCP-сокет и подключает его сигнал readyRead
 * к слоту onReadyRead для обработки входящих данных.
 * 
 * @param parent Родительский объект Qt
 */
ClientSingleton::ClientSingleton(QObject *parent)
    : QObject(parent)
{
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &ClientSingleton::onReadyRead);
}

/**
 * @brief Деструктор класса ClientSingleton
 * 
 * Отключает сокет от хоста, если он был создан.
 */
ClientSingleton::~ClientSingleton()
{
    if (socket) {
        socket->disconnectFromHost();
    }
}

/**
 * @brief Получить единственный экземпляр класса (паттерн Singleton)
 * 
 * Использует локальную статическую переменную для保证
 * единственности экземпляра на протяжении всего времени работы программы.
 * 
 * @return Ссылка на единственный экземпляр ClientSingleton
 */
ClientSingleton& ClientSingleton::instance()
{
    static ClientSingleton inst;
    return inst;
}

/**
 * @brief Подключиться к серверу
 * 
 * Если клиент уже подключён — возвращает true без повторного подключения.
 * Иначе пытается установить TCP-соединение с указанным хостом и портом
 * с таймаутом 3 секунды.
 * 
 * @param host Адрес сервера (IP или домен)
 * @param port Номер порта сервера
 * @return true если подключение установлено, false в случае ошибки
 */
bool ClientSingleton::connectToServer(const QString &host, int port)
{
    if (socket->state() == QAbstractSocket::ConnectedState)
        return true;
    socket->connectToHost(host, static_cast<quint16>(port));
    return socket->waitForConnected(3000);
}

/**
 * @brief Отключиться от сервера
 * 
 * Инициирует разрыв TCP-соединения через метод disconnectFromHost сокета.
 */
void ClientSingleton::disconnectFromServer()
{
    socket->disconnectFromHost();
}

/**
 * @brief Проверить состояние подключения
 * 
 * @return true если сокет находится в состоянии ConnectedState,
 *         false в противном случае
 */
bool ClientSingleton::isConnected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

/**
 * @brief Отправить синхронный запрос и получить ответ
 * 
 * Отправляет строку запроса на сервер с символом переноса строки в конце.
 * Если клиент не подключён — пытается подключиться к 127.0.0.1:33333.
 * Ожидает ответа сервера с таймаутом 60 секунд, после чего считывает
 * все доступные данные.
 * 
 * @param request Строка запроса для отправки на сервер
 * @return Ответ сервера в виде строки (без пробелов по краям) или
 *         пустая строка при ошибке подключения/таймауте
 */
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

/**
 * @brief Отправить асинхронный запрос
 * 
 * Отправляет строку запроса на сервер без блокировки основного потока.
 * Если клиент не подключён — пытается подключиться к 127.0.0.1:33333.
 * При ошибке подключения испускает сигнал responseReceived с пустой строкой.
 * Ответ от сервера будет обработан позже через слот onReadyRead.
 * 
 * @param request Строка запроса для отправки на сервер
 */
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

/**
 * @brief Слот-обработчик получения данных от сервера
 * 
 * Считывает все доступные данные из сокета, преобразует их в строку
 * и испускает сигнал responseReceived, если строка не пуста.
 */
void ClientSingleton::onReadyRead()
{
    QByteArray data = socket->readAll();
    QString response = QString::fromUtf8(data).trimmed();
    if (!response.isEmpty()) {
        emit responseReceived(response);
    }
}
