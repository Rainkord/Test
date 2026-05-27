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
 * @class MyTcpServer
 * @brief Класс TCP-сервера для обработки клиентских подключений
 *
 * Управляет входящими TCP-подключениями, чтением данных от клиентов,
 * отправкой ответов и отслеживанием состояния подключённых клиентов.
 */
class MyTcpServer : public QObject
{
    Q_OBJECT

public:

    /**
     * @brief Конструктор TCP-сервера
     *
     * Создаёт QTcpServer и пытается начать прослушивание
     * на порту SERVER_PORT (33333).
     *
     * @param parent родительский QObject (по умолчанию nullptr)
     */
    explicit MyTcpServer(QObject *parent = nullptr);

    /**
     * @brief Деструктор TCP-сервера
     *
     * Закрывает все активные клиентские соединения и освобождает ресурсы.
     */
    ~MyTcpServer();

public slots:

    /**
     * @brief Слот обработки нового подключения клиента
     *
     * Принимает входящее соединение, подключает сигналы readyRead и
     * disconnected к соответствующим слотам, сохраняет сокет в map.
     */
    void slotNewConnection();

    /**
     * @brief Слот чтения данных от клиента
     *
     * Считывает данные из сокета, передаёт их на обработку
     * через FunctionsForServer и отправляет ответ обратно клиенту.
     */
    void slotServerRead();

    /**
     * @brief Слот обработки отключения клиента
     *
     * Удаляет сокет из внутренней map и планирует его удаление.
     */
    void slotClientDisconnected();

private:
    /// Указатель на TCP-сервер
    QTcpServer               *m_pServer;
    /// Map активных клиентских соединений (дескриптор сокета → QTcpSocket*)
    QMap<int, QTcpSocket*>    m_clients;
    /// Статус работы сервера (true — работает, false — остановлен)
    bool                      m_serverStatus;
};

#endif // MYTCPSERVER_H
