#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDataStream>
#include <QByteArray>
#include <QDebug>
#include <QString>
#include <QMap>
#include "functionsforserver.h"

/**
 * @file mytcpserver.h
 * @brief TCP-сервер на базе Qt с поддержкой множества одновременных клиентов.
 */

/**
 * @class MyTcpServer
 * @brief Многопользовательский TCP-сервер.
 *
 * Наследуется от QObject и использует механизм сигналов/слотов Qt.
 * При запуске прослушивает порт 33333 и принимает входящие подключения.
 * Каждый клиент идентифицируется уникальным дескриптором сокета.
 * Входящие сообщения передаются в FunctionsForServer::processMessage().
 */
class MyTcpServer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор: инициализирует сервер и начинает прослушивание порта.
     * @param parent Родительский объект Qt (по умолчанию nullptr).
     */
    explicit MyTcpServer(QObject *parent = nullptr);

    /**
     * @brief Деструктор: закрывает все сокеты и останавливает сервер.
     */
    ~MyTcpServer();

public slots:
    /**
     * @brief Слот: вызывается при появлении нового входящего подключения.
     *
     * Принимает подключение, создаёт сокет, добавляет его в список клиентов
     * и подключает сигналы readyRead / disconnected.
     */
    void slotNewConnection();

    /**
     * @brief Слот: вызывается когда от клиента пришли данные.
     *
     * Читает все доступные байты, передаёт строку в processMessage(),
     * отправляет ответ обратно клиенту через тот же сокет.
     */
    void slotServerRead();

    /**
     * @brief Слот: вызывается при отключении клиента.
     *
     * Удаляет сокет из списка активных клиентов и освобождает память.
     */
    void slotClientDisconnected();

private:
    QTcpServer               *m_pServer;  ///< Объект Qt TCP-сервера.
    QMap<int, QTcpSocket*>    m_clients;  ///< Словарь активных клиентов: дескриптор → сокет.
    bool                      m_serverStatus; ///< Флаг: сервер запущен (true) или нет.
};

#endif // MYTCPSERVER_H
