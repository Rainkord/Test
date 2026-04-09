#include "functionsforserver.h"
#include "database.h"
#include "calculator.h"

#include <QStringList>
#include <QDebug>

QString FunctionsForServer::parsing(const QString &input, int socketDescriptor)
{
    QStringList parts = input.split("||");

    if (parts.isEmpty()) {
        qDebug() << "parsing() empty input from descriptor" << socketDescriptor;
        return "error||empty_request";
    }

    QString command = parts.at(0).trimmed();
    qDebug() << "parsing() command:" << command
             << "from descriptor:" << socketDescriptor;

    if (command == "auth") {
        // Expected: auth||login||password  (3 parts total)
        if (parts.size() < 3) {
            return "error||invalid_auth_params";
        }
        QString login    = parts.at(1);
        QString password = parts.at(2);
        return authorization(login, password);

    } else if (command == "registration") {
        // Expected: registration||login||password||confirmPassword||email
        //           (5 parts total)
        if (parts.size() < 5) {
            return "error||invalid_registration_params";
        }
        QString login           = parts.at(1);
        QString password        = parts.at(2);
        QString confirmPassword = parts.at(3);
        QString email           = parts.at(4);
        return registration(login, password, confirmPassword, email);

    } else if (command == "get_task") {
        return getTaskParams();

    } else if (command == "get_graph") {
        // Expected: get_graph||xMin||xMax||step  (4 parts total)
        if (parts.size() < 4) {
            return "error||invalid_graph_params";
        }
        bool okMin, okMax, okStep;
        double xMin = parts.at(1).toDouble(&okMin);
        double xMax = parts.at(2).toDouble(&okMax);
        double step = parts.at(3).toDouble(&okStep);

        if (!okMin || !okMax || !okStep) {
            return "error||invalid_graph_values";
        }
        return getGraphData(xMin, xMax, step);

    } else {
        qDebug() << "parsing() unknown command:" << command;
        return "error||unknown_command";
    }
}

QString FunctionsForServer::authorization(const QString &login,
                                           const QString &password)
{
    bool ok = Database::instance().checkUser(login, password);
    if (ok) {
        qDebug() << "authorization() success for login:" << login;
        return "auth+||" + login;
    } else {
        qDebug() << "authorization() failed for login:" << login;
        return "auth-";
    }
}

QString FunctionsForServer::registration(const QString &login,
                                          const QString &password,
                                          const QString &confirmPassword,
                                          const QString &email)
{
    if (password != confirmPassword) {
        qDebug() << "registration() passwords do not match for login:" << login;
        return "reg-||passwords_do_not_match";
    }

    if (Database::instance().userExists(login)) {
        qDebug() << "registration() user already exists:" << login;
        return "reg-||user_already_exists";
    }

    bool ok = Database::instance().addUser(login, password, email);
    if (ok) {
        qDebug() << "registration() success for login:" << login;
        return "reg+||" + login;
    } else {
        qDebug() << "registration() database error for login:" << login;
        return "reg-||database_error";
    }
}

QString FunctionsForServer::getTaskParams()
{
    return "task||Function 9: f(x) = |x|-2 for x<-2, x^2+x+1 for -2<=x<2, |x-2|+1 for x>=2";
}

QString FunctionsForServer::getGraphData(double xMin, double xMax, double step)
{
    return Calculator::generateGraphData(xMin, xMax, step);
}
