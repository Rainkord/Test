#ifndef CLIENTSINGLETON_H
#define CLIENTSINGLETON_H

#include <QObject>
#include <QTcpSocket>
#include <QString>

class ClientSingleton : public QObject
{
    Q_OBJECT

public:
    static ClientSingleton& instance();

    bool connectToServer(const QString &host, int port);
    void disconnectFromServer();
    bool isConnected() const;

    // Синхронный вызов (для коротких запросов — verify_reg и т.д.)
    QString sendRequest(const QString &request);

    // Асинхронный вызов — не блокирует UI
    void sendRequestAsync(const QString &request);

signals:
    void responseReceived(const QString &response);

private:
    explicit ClientSingleton(QObject *parent = nullptr);
    ~ClientSingleton();
    ClientSingleton(const ClientSingleton&) = delete;
    ClientSingleton& operator=(const ClientSingleton&) = delete;

    QTcpSocket *socket;
    QString     m_pendingResponse;

private slots:
    void onReadyRead();
};

#endif // CLIENTSINGLETON_H
