/**
 * @file clientsingleton.h
 * @brief Singleton-класс сетевого клиента.
 */

#ifndef CLIENTSINGLETON_H
#define CLIENTSINGLETON_H

#include <QObject>
#include <QTcpSocket>

/**
 * @class ClientSingleton
 * @brief Единственный экземпляр TCP-клиента для связи с сервером.
 *
 * Реализует паттерн Singleton и обеспечивает асинхронную отправку
 * текстовых запросов на сервер через QTcpSocket. При получении
 * данных от сервера испускает сигнал responseReceived.
 */
class ClientSingleton : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Возвращает единственный экземпляр класса.
     * @return Ссылка на экземпляр ClientSingleton.
     */
    static ClientSingleton &instance();

    /**
     * @brief Асинхронно отправляет запрос на сервер.
     *
     * Если соединение не установлено, предварительно подключается
     * к серверу. Запрос завершается символом '\n'.
     * @param request Строка запроса в формате протокола приложения.
     */
    void sendRequestAsync(const QString &request);

signals:
    /**
     * @brief Испускается при получении ответа от сервера.
     * @param response Строка ответа (без завершающего '\n').
     */
    void responseReceived(const QString &response);

private slots:
    /** @brief Читает данные из сокета и испускает responseReceived. */
    void onReadyRead();

private:
    /** @brief Приватный конструктор (Singleton). @param parent Родитель. */
    explicit ClientSingleton(QObject *parent = nullptr);

    QTcpSocket *m_socket; ///< TCP-сокет для связи с сервером.
};

#endif // CLIENTSINGLETON_H
