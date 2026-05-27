#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QString>

/**
 * @brief Класс калькулятора кусочно-определённой функции
 *
 * Предоставляет статические методы для вычисления значений
 * кусочно-определённой математической функции и генерации
 * данных для построения графика.
 */
class Calculator
{
public:

    /**
     * @brief Вычисляет значение кусочно-определённой функции
     *
     * Функция определена тремя отрезками:
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
    static double calculate(double x, double a, double b, double c);

    /**
     * @brief Генерирует строку данных для построения графика функции
     *
     * Вычисляет значения функции на заданном диапазоне [xMin, xMax]
     * с указанным шагом. Возвращает строку в формате
     * "graph||x1;y1||x2;y2||...".
     *
     * @param xMin минимальное значение x
     * @param xMax максимальное значение x
     * @param step шаг изменения x (если ≤ 0, используется 0.1)
     * @param a параметр a функции
     * @param b параметр b функции
     * @param c параметр c функции
     * @return строка с данными графика в формате "graph||x;y||..."
     */
    static QString generateGraphData(double xMin, double xMax, double step,
                                     double a, double b, double c);
};

#endif // CALCULATOR_H
