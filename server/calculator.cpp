#include "calculator.h"

#include <cmath>
#include <QStringList>
#include <QDebug>

double Calculator::calculate(double x)
{
    if (x < -2.0) {
        return std::fabs(x) - 2.0;
    } else if (x >= -2.0 && x < 2.0) {
        return x * x + x + 1.0;
    } else {
        // x >= 2
        return std::fabs(x - 2.0) + 1.0;
    }
}

QString Calculator::generateGraphData(double xMin, double xMax, double step)
{
    if (step <= 0.0) {
        qDebug() << "Calculator::generateGraphData() invalid step:" << step;
        return "graph||";
    }

    QStringList points;

    for (double x = xMin; x <= xMax + 1e-9; x += step) {
        double y = calculate(x);
        // Format each point as "x;y" with 4 decimal places
        QString point = QString("%1;%2")
                            .arg(x, 0, 'f', 4)
                            .arg(y, 0, 'f', 4);
        points.append(point);
    }

    // Prepend "graph" and join everything with "||"
    QString result = "graph||" + points.join("||");
    return result;
}
