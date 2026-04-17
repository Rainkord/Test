#include "calculator.h"

#include <cmath>
#include <QStringList>

double Calculator::calculate(double x, double a, double b, double c)
{
    if (x < -2.0) {
        // |x*a| - 2
        return std::fabs(x * a) - 2.0;
    } else if (x >= -2.0 && x < 2.0) {
        // b*(x^2) + x + 1
        return b * (x * x) + x + 1.0;
    } else {
        // |x - 2| + 1*c
        return std::fabs(x - 2.0) + 1.0 * c;
    }
}

QString Calculator::generateGraphData(double xMin, double xMax, double step,
                                      double a, double b, double c)
{
    if (step <= 0.0) {
        step = 0.1;
    }

    QString result = "graph";

    double x = xMin;
    while (x <= xMax + 1e-9) {
        double y = calculate(x, a, b, c);

        // Format x and y with enough precision, removing trailing zeros
        QString xStr = QString::number(x, 'g', 10);
        QString yStr = QString::number(y, 'g', 10);

        result += "||" + xStr + ";" + yStr;

        x += step;
        // Guard against floating point drift causing infinite loops
        if (x > xMax + std::fabs(step) * 2.0) break;
    }

    return result;
}
