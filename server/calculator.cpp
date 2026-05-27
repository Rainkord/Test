#include "calculator.h"

#include <cmath>
#include <QStringList>

/**
 * @brief Вычисляет значение кусочно-определённой функции
 *
 * Определяет отрезок по значению x и применяет соответствующую формулу:
 * - При x < -2:  f(x) = |x·a| − 2
 * - При −2 ≤ x < 2:  f(x) = b·x² + x + 1
 * - При x ≥ 2:  f(x) = |x − 2| + c
 *
 * @param x значение аргумента
 * @param a параметр для левого отрезка (x < −2)
 * @param b параметр для среднего отрезка (−2 ≤ x < 2)
 * @param c параметр для правого отрезка (x ≥ 2)
 * @return вычисленное значение функции f(x)
 */
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

/**
 * @brief Генерирует строку данных для построения графика функции
 *
 * Последовательно вычисляет значения функции на диапазоне [xMin, xMax]
 * с заданным шагом. Результат формируется как строка "graph||x;y||...",
 * где каждая пара x;y разделена двойными вертикальными чертами.
 * Если шаг ≤ 0, используется значение по умолчанию 0.1.
 *
 * @param xMin минимальное значение x
 * @param xMax максимальное значение x
 * @param step шаг изменения x (если ≤ 0, используется 0.1)
 * @param a параметр a функции
 * @param b параметр b функции
 * @param c параметр c функции
 * @return строка с данными графика в формате "graph||x;y||..."
 */
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
