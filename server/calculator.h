#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QString>

/**
 * @file calculator.h
 * @brief Вычисление ветвящейся функции и генерация данных для графика.
 */

/**
 * @class Calculator
 * @brief Класс для вычисления значений ветвящейся функции и формирования
 *        строки данных графика.
 *
 * Функция задана по трём ветвям:
 * - f(x) = |x·a| − 2,          при x < −2
 * - f(x) = b·x² + x + 1,       при −2 ≤ x < 2
 * - f(x) = |x − 2| + c,        при x ≥ 2
 */
class Calculator
{
public:
    /**
     * @brief Вычисляет значение ветвящейся функции f(x).
     *
     * @param x Аргумент функции.
     * @param a Коэффициент для ветви x < −2.
     * @param b Коэффициент для ветви −2 ≤ x < 2.
     * @param c Коэффициент для ветви x ≥ 2.
     * @return Значение функции f(x) при заданных параметрах.
     */
    static double calculate(double x, double a, double b, double c);

    /**
     * @brief Генерирует строку с точками графика для диапазона [xMin, xMax].
     *
     * Формат возвращаемой строки: @c "graph||x1;y1||x2;y2||..."
     *
     * @param xMin  Левая граница диапазона.
     * @param xMax  Правая граница диапазона.
     * @param step  Шаг по оси X (должен быть > 0; при step ≤ 0 подставляется 0.1).
     * @param a     Коэффициент a.
     * @param b     Коэффициент b.
     * @param c     Коэффициент c.
     * @return Строка данных графика в формате протокола.
     */
    static QString generateGraphData(double xMin, double xMax, double step,
                                     double a, double b, double c);
};

#endif // CALCULATOR_H
