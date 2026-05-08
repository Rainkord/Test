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
 */
class ClientSingleton : public QObject
{
    Q_OBJECT

public:
    static ClientSingleton &instance();

    bool    connectToServer(const QString &host, int port);
    void    disconnectFromServer();
    bool    isConnected() const;
    QString sendRequest(const QString &request);
    void    sendRequestAsync(const QString &request);

signals:
    void responseReceived(const QString &response);

private slots:
    void onReadyRead();

private:
    explicit ClientSingleton(QObject *parent = nullptr);
    ~ClientSingleton();

    QTcpSocket *socket; ///< TCP-сокет для связи с сервером.
};

#endif // CLIENTSINGLETON_H
