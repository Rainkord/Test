#ifndef FUNCTIONSFORSERVER_H
#define FUNCTIONSFORSERVER_H

#include <QString>

class FunctionsForServer
{
public:
    // Parses the raw incoming message and dispatches to the correct handler.
    static QString parsing(const QString &input, int socketDescriptor);

    // auth||login||password -> "auth+||login" or "auth-"
    static QString authorization(const QString &login, const QString &password);

    // registration||login||password||confirmPassword||email
    //   -> "reg+||login" or "reg-||reason"
    static QString registration(const QString &login, const QString &password,
                                const QString &confirmPassword,
                                const QString &email);

    // get_task -> task description string
    static QString getTaskParams();

    // get_graph||xMin||xMax||step -> graph data string
    static QString getGraphData(double xMin, double xMax, double step);

private:
    FunctionsForServer() = delete;
};

#endif // FUNCTIONSFORSERVER_H
