#ifndef CLIENTSINGLETON_H
#define CLIENTSINGLETON_H

#include <QObject>
#include <QTcpSocket>

/**
 * @brief Класс-одиночка для управления TCP-соединением с сервером
 * 
 * Обеспечивает единственный экземпляр клиента для подключения
 * к серверу по протоколу TCP. Поддерживает синхронные и асинхронные
 * запросы, а также уведомления о получении ответа через сигнал.
 */
class ClientSingleton : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Получить единственный экземпляр класса (паттерн Singleton)
     * 
     * @return Ссылка на единственный экземпляр ClientSingleton
     */
    static ClientSingleton &instance();

    /**
     * @brief Подключиться к серверу
     * 
     * Устанавливает TCP-соединение с указанным хостом и портом.
     * Если уже подключён — возвращает true без повторного подключения.
     * 
     * @param host Адрес сервера (IP или домен)
     * @param port Номер порта сервера
     * @return true если подключение установлено, false в случае ошибки
     */
    bool    connectToServer(const QString &host, int port);

    /**
     * @brief Отключиться от сервера
     * 
     * Разрывает текущее TCP-соединение.
     */
    void    disconnectFromServer();

    /**
     * @brief Проверить состояние подключения
     * 
     * @return true если клиент подключён к серверу, false в противном случае
     */
    bool    isConnected() const;

    /**
     * @brief Отправить синхронный запрос и получить ответ
     * 
     * Отправляет строку запроса на сервер и блокирует выполнение,
     * ожидая ответ. Если не подключён — пытается подключиться к 127.0.0.1:33333.
     * 
     * @param request Строка запроса для отправки на сервер
     * @return Ответ сервера в виде строки или пустая строка при ошибке
     */
    QString sendRequest(const QString &request);

    /**
     * @brief Отправить асинхронный запрос
     * 
     * Отправляет строку запроса на сервер без блокировки.
     * Ответ будет доставлен через сигнал responseReceived().
     * Если не подключён — пытается подключиться к 127.0.0.1:33333.
     * 
     * @param request Строка запроса для отправки на сервер
     */
    void    sendRequestAsync(const QString &request);

signals:
    /// @brief Сигнал, испускаемый при получении ответа от сервера
    /// @param response Ответ сервера в виде строки
    void responseReceived(const QString &response);

private slots:
    /// @brief Слот-обработчик события "данные доступны для чтения" из сокета
    void onReadyRead();

private:
    /**
     * @brief Конструктор класса (приватный — паттерн Singleton)
     * 
     * @param parent Родительский объект Qt
     */
    explicit ClientSingleton(QObject *parent = nullptr);

    /**
     * @brief Деструктор класса
     * 
     * Отключает сокет от хоста при уничтожении объекта.
     */
    ~ClientSingleton();

    /// TCP-сокет для соединения с сервером
    QTcpSocket *socket;
};

#endif // CLIENTSINGLETON_H
